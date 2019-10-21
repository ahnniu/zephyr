#include <string.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include <minode_dht11.h>

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#define MINODE_DHT_START_SIGNAL_DURATION			18000
#define MINODE_DHT_SIGNAL_MAX_WAIT_DURATION		100
#define MINODE_DHT_DATA_BITS_NUM							40

static s8_t dht_measure_signal_duration(struct minode_dht_driver_data *drv_data,
					  u32_t signal_val)
{
	u32_t val;
	u32_t elapsed_cycles;
	u32_t max_wait_cycles = (u32_t)(
		(u64_t)MINODE_DHT_SIGNAL_MAX_WAIT_DURATION *
		(u64_t)sys_clock_hw_cycles_per_sec() /
		(u64_t)USEC_PER_SEC
	);
	u32_t start_cycles = k_cycle_get_32();

	do {
		gpio_pin_read(drv_data->gpio, drv_data->pin, &val);
		elapsed_cycles = k_cycle_get_32() - start_cycles;

		if (elapsed_cycles > max_wait_cycles) {
			return -1;
		}
	} while (val == signal_val);

	return (u64_t)elapsed_cycles *
	       (u64_t)USEC_PER_SEC /
	       (u64_t)sys_clock_hw_cycles_per_sec();
}

int minode_dht_sample_fetch(struct minode_dht_device *dev)
{
	struct minode_dht_driver_data *drv_data;
	int ret = 0;
	s8_t signal_duration[MINODE_DHT_DATA_BITS_NUM];
	s8_t max_duration, min_duration, avg_duration;
	u8_t buf[5];
	unsigned int i, j;

	if (!dev) {
		return -EINVAL;
	}

	drv_data = &dev->driver_data;

	/* send start signal */
	gpio_pin_write(drv_data->gpio, drv_data->pin, 0);

	k_busy_wait(MINODE_DHT_START_SIGNAL_DURATION);

	gpio_pin_write(drv_data->gpio, drv_data->pin, 1);

	/* switch to DIR_IN to read sensor signals */
	gpio_pin_configure(drv_data->gpio, drv_data->pin,
			   GPIO_DIR_IN);

	/* wait for sensor response */
	if (dht_measure_signal_duration(drv_data, 1) == -1) {
		ret = -EIO;
		goto cleanup;
	}

	/* read sensor response */
	if (dht_measure_signal_duration(drv_data, 0) == -1) {
		ret = -EIO;
		goto cleanup;
	}

	/* wait for sensor data */
	if (dht_measure_signal_duration(drv_data, 1) == -1) {
		ret = -EIO;
		goto cleanup;
	}

	/* read sensor data */
	for (i = 0U; i < MINODE_DHT_DATA_BITS_NUM; i++) {
		/* LOW signal to indicate a new bit */
		if (dht_measure_signal_duration(drv_data, 0) == -1) {
			ret = -EIO;
			goto cleanup;
		}

		/* HIGH signal duration indicates bit value */
		signal_duration[i] = dht_measure_signal_duration(drv_data, 1);
		if (signal_duration[i] == -1) {
			ret = -EIO;
			goto cleanup;
		}
	}

	/*
	 * the datasheet says 20-40us HIGH signal duration for a 0 bit and
	 * 80us for a 1 bit; however, since dht_measure_signal_duration is
	 * not very precise, compute the threshold for deciding between a
	 * 0 bit and a 1 bit as the average between the minimum and maximum
	 * if the durations stored in signal_duration
	 */
	min_duration = signal_duration[0];
	max_duration = signal_duration[0];
	for (i = 1U; i < MINODE_DHT_DATA_BITS_NUM; i++) {
		if (min_duration > signal_duration[i]) {
			min_duration = signal_duration[i];
		}
		if (max_duration < signal_duration[i]) {
			max_duration = signal_duration[i];
		}
	}
	avg_duration = ((s16_t)min_duration + (s16_t)max_duration) / 2;

	/* store bits in buf */
	j = 0U;
	(void)memset(buf, 0, sizeof(buf));
	for (i = 0U; i < MINODE_DHT_DATA_BITS_NUM; i++) {
		if (signal_duration[i] >= avg_duration) {
			buf[j] = (buf[j] << 1) | 1;
		} else {
			buf[j] = buf[j] << 1;
		}

		if (i % 8 == 7U) {
			j++;
		}
	}

	/* verify checksum */
	if (((buf[0] + buf[1] + buf[2] + buf[3]) & 0xFF) != buf[4]) {
		ret = -EIO;
	} else {
		memcpy(drv_data->sample, buf, 4);
	}

cleanup:
	/* switch to DIR_OUT and leave pin to HIGH until next fetch */
	gpio_pin_configure(drv_data->gpio, drv_data->pin,
			   GPIO_DIR_OUT);
	gpio_pin_write(drv_data->gpio, drv_data->pin, 1);

	return ret;
}

int minode_dht_channel_get(struct minode_dht_device *dev,
			   enum sensor_channel chan,
			   struct sensor_value *val)
{
	struct minode_dht_driver_data *drv_data;

	if (!dev) {
		return -EINVAL;
	}

	if (chan != SENSOR_CHAN_AMBIENT_TEMP && chan != SENSOR_CHAN_HUMIDITY) {
		return -EINVAL;
	}

	drv_data = &dev->driver_data;
	/* use only integral data byte */
	if (chan == SENSOR_CHAN_HUMIDITY) {
		val->val1 = drv_data->sample[0];
		val->val2 = 0;
	} else { /* chan == SENSOR_CHAN_AMBIENT_TEMP */
		val->val1 = drv_data->sample[2];
		val->val2 = 0;
	}

	return 0;
}

int minode_dht_humidity_get(struct minode_dht_device *dev, struct sensor_value *val)
{
	return minode_dht_channel_get(dev, SENSOR_CHAN_HUMIDITY, val);
}

int minode_dht_ambient_temp_get(struct minode_dht_device *dev, struct sensor_value *val)
{
	return minode_dht_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, val);
}

static void timer_expiry_handler(struct k_timer *timer)
{
	struct minode_dht_driver_data *driver_data = CONTAINER_OF(timer, struct minode_dht_driver_data, timer);

	k_work_submit(&driver_data->work);
}

static void work_handler(struct k_work *work)
{
	struct minode_dht_driver_data *driver_data = CONTAINER_OF(work, struct minode_dht_driver_data, work);
	struct minode_dht_device *dev = CONTAINER_OF(driver_data, struct minode_dht_device, driver_data);

	minode_dht_sample_fetch(dev);

	if (dev->on_new_sampling) {
		dev->on_new_sampling(dev);
	}
}

void minode_dht_start_sampling(struct minode_dht_device *dev, int period_ms)
{
	if (!dev) {
		return;
	}

	k_timer_start(&dev->driver_data.timer, K_MSEC(period_ms), K_MSEC(period_ms));
}

void minode_dht_stop_sampling(struct minode_dht_device *dev)
{
	if (!dev) {
		return;
	}

	k_timer_stop(&dev->driver_data.timer);
}

int minode_dht_init(struct minode_dht_device *dev)
{
	struct minode_dht_driver_data *drv_data;

	if (!dev) {
		return -EINVAL;
	}

	if (!dev->connector) {
		return -EINVAL;
	}

	drv_data = &dev->driver_data;

	drv_data->gpio = minode_connector_get_gpio_device(dev->connector, 0);
	if (drv_data->gpio == NULL) {
		return -ENODEV;
	}

	drv_data->pin = minode_connector_get_gpio_pin(dev->connector, 0);
	if (drv_data->pin < 0) {
		return -ENODEV;
	}

	gpio_pin_configure(drv_data->gpio, drv_data->pin,
			   GPIO_DIR_OUT);

	gpio_pin_write(drv_data->gpio, drv_data->pin, 1);

	k_work_init(&dev->driver_data.work, work_handler);
	k_timer_init(&dev->driver_data.timer, timer_expiry_handler, NULL);

	return 0;
}

