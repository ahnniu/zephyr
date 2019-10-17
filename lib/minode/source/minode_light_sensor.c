#include <minode_connector.h>
#include <minode_light_sensor.h>

#define MINODE_LIGHT_LEVEL_0_ADC_VAL    0
#define MINODE_LIGHT_LEVEL_1_ADC_VAL    256
#define MINODE_LIGHT_LEVEL_2_ADC_VAL    512
#define MINODE_LIGHT_LEVEL_3_ADC_VAL    768
#define MINODE_LIGHT_LEVEL_4_ADC_VAL    900
#define MINODE_LIGHT_LEVEL_5_ADC_VAL    1024

static void timer_expiry_handler(struct k_timer *timer)
{
	struct minode_light_driver_data *driver_data = CONTAINER_OF(timer, struct minode_light_driver_data, timer);

	k_work_submit(&driver_data->work);
}

static void work_handler(struct k_work *work)
{
	enum minode_light_level prev_level;
	enum minode_light_level new_level;

	struct minode_light_driver_data *driver_data = CONTAINER_OF(work, struct minode_light_driver_data, work);
	struct minode_light_device *dev = CONTAINER_OF(driver_data, struct minode_light_device, driver_data);

	prev_level = driver_data->level;

	new_level = minode_light_retrieve_level(dev);
	driver_data->level = new_level;

	if(prev_level != new_level) {
		if (dev->on_level_change) {
			dev->on_level_change(dev, prev_level, new_level);
		}
	}
}

int minode_light_init(struct minode_light_device *dev)
{
	struct device *adc_dev;
	u8_t adc_input;
	u8_t adc_channel_id;

	if (!dev) {
		return -EINVAL;
	}

	if (!dev->connector) {
		return -EINVAL;
	}

	adc_dev = minode_connector_get_adc_device(dev->connector, 0);
	if (!adc_dev) {
		return -ENODEV;
	}
	adc_input = minode_connector_get_adc_input(dev->connector, 0);
	if(adc_input < 0) {
		return -ENODEV;
	}
	adc_channel_id = minode_connector_allocate_adc_channel_id();
	if (adc_channel_id < 0) {
		return -ENODEV;
	}

	dev->driver_data.adc_dev = adc_dev;

	dev->driver_data.channel.gain = ADC_GAIN_1_3;
	dev->driver_data.channel.reference = ADC_REF_VDD_1_3;
	dev->driver_data.channel.acquisition_time = ADC_ACQ_TIME_DEFAULT;
	dev->driver_data.channel.channel_id = adc_channel_id;
	dev->driver_data.channel.differential = 0;
	dev->driver_data.channel.input_positive = adc_input;

	dev->driver_data.sampling_options.interval_us = 0;
	dev->driver_data.sampling_options.callback = NULL;
	dev->driver_data.sampling_options.extra_samplings = MINODE_LIGHT_SAMPLINGS_COUNT - 1;

	dev->driver_data.sampling.options = &(dev->driver_data.sampling_options);
	dev->driver_data.sampling.channels = 1 << adc_channel_id;
	dev->driver_data.sampling.buffer = dev->driver_data.buffer;
	dev->driver_data.sampling.buffer_size = 2* MINODE_LIGHT_SAMPLINGS_COUNT;
	dev->driver_data.sampling.resolution = 10;
	dev->driver_data.sampling.oversampling = 0;
	dev->driver_data.sampling.calibrate = false;

	adc_channel_setup(adc_dev, &dev->driver_data.channel);

	dev->driver_data.level = minode_light_retrieve_level(dev);

	k_work_init(&dev->driver_data.work, work_handler);
	k_timer_init(&dev->driver_data.timer, timer_expiry_handler, NULL);

	return 0;
}

void minode_light_start_listening(struct minode_light_device *dev)
{
	k_timer_start(&dev->driver_data.timer, K_MSEC(MINODE_LIGHT_SAMPLINGS_PEROID_MS), K_MSEC(MINODE_LIGHT_SAMPLINGS_PEROID_MS));
}

void minode_light_stop_listening(struct minode_light_device *dev)
{
	k_timer_stop(&dev->driver_data.timer);
}

enum minode_light_level minode_light_retrieve_level(struct minode_light_device *dev)
{
	int i;
	u32_t total_adc_val;
	u16_t adc_val;
	enum minode_light_level level;
	u8_t *buffer;

	adc_read(dev->driver_data.adc_dev, &dev->driver_data.sampling);

	buffer = dev->driver_data.buffer;
	total_adc_val = 0;
	for(i = 0; i < MINODE_LIGHT_SAMPLINGS_COUNT; i++) {
		adc_val = ((buffer[2 * i + 1] & 0x03) << 8 | buffer[2 * i]);
		total_adc_val += adc_val;
	}

	adc_val = total_adc_val / MINODE_LIGHT_SAMPLINGS_COUNT;

	if (adc_val >= MINODE_LIGHT_LEVEL_0_ADC_VAL && adc_val < MINODE_LIGHT_LEVEL_1_ADC_VAL) {
		level = MINODE_LIGHT_LEVEL_0;
	} else if (adc_val >= MINODE_LIGHT_LEVEL_1_ADC_VAL && adc_val < MINODE_LIGHT_LEVEL_2_ADC_VAL) {
		level = MINODE_LIGHT_LEVEL_1;
	} else if (adc_val >= MINODE_LIGHT_LEVEL_2_ADC_VAL && adc_val < MINODE_LIGHT_LEVEL_3_ADC_VAL) {
		level = MINODE_LIGHT_LEVEL_2;
	} else if (adc_val >= MINODE_LIGHT_LEVEL_3_ADC_VAL && adc_val < MINODE_LIGHT_LEVEL_4_ADC_VAL) {
		level = MINODE_LIGHT_LEVEL_3;
	} else if (adc_val >= MINODE_LIGHT_LEVEL_4_ADC_VAL && adc_val < MINODE_LIGHT_LEVEL_5_ADC_VAL) {
		level = MINODE_LIGHT_LEVEL_4;
	} else {
		level = MINODE_LIGHT_LEVEL_INVAL;
	}

	return level;
}

enum minode_light_level minode_light_get_level(struct minode_light_device *dev)
{
	return dev->driver_data.level;
}