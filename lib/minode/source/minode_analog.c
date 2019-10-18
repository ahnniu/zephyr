#include <minode_connector.h>
#include <minode_analog.h>

static void timer_expiry_handler(struct k_timer *timer)
{
	struct minode_analog_driver_data *driver_data = CONTAINER_OF(timer, struct minode_analog_driver_data, timer);

	k_work_submit(&driver_data->work);
}

static void work_handler(struct k_work *work)
{
	u16_t prev_adc_val;
	u16_t new_adc_val;
	struct minode_analog_driver_data *driver_data = CONTAINER_OF(work, struct minode_analog_driver_data, work);
	struct minode_analog_device *dev = CONTAINER_OF(driver_data, struct minode_analog_device, driver_data);

	prev_adc_val = driver_data->avg_adc_val;
	new_adc_val = minode_analog_retrieve(dev);
	driver_data->avg_adc_val = new_adc_val;

	if (dev->on_new_sampling) {
		dev->on_new_sampling(dev, prev_adc_val, new_adc_val);
	}
}

int minode_analog_init(struct minode_analog_device *dev,
			u8_t *sampling_buffer, int sampling_count,
			int sampling_freq_hz, u16_t sampling_reference)
{
	struct device *adc_dev;
	u8_t adc_input;
	u8_t adc_channel_id;

	if (!dev) {
		return -EINVAL;
	}

	if (!sampling_buffer) {
		return -EINVAL;
	}

	if (!dev->connector) {
		return -EINVAL;
	}

	if (!sampling_buffer) {
		return -EINVAL;
	}

	if (sampling_count < 0 || sampling_count > 31) {
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
	dev->driver_data.ref_val = sampling_reference;

	dev->driver_data.channel.gain = ADC_GAIN_1_3;
	dev->driver_data.channel.reference = ADC_REF_VDD_1_3;
	dev->driver_data.channel.acquisition_time = ADC_ACQ_TIME_DEFAULT;
	dev->driver_data.channel.channel_id = adc_channel_id;
	dev->driver_data.channel.differential = 0;
	dev->driver_data.channel.input_positive = adc_input;

	if (sampling_freq_hz) {
		dev->driver_data.sampling_options.interval_us = 1000000 / sampling_freq_hz;
	} else {
		dev->driver_data.sampling_options.interval_us = 0;
	}
	dev->driver_data.sampling_options.callback = NULL;
	dev->driver_data.sampling_options.extra_samplings = sampling_count - 1;

	dev->driver_data.sampling.options = &(dev->driver_data.sampling_options);
	dev->driver_data.sampling.channels = 1 << adc_channel_id;
	dev->driver_data.sampling.buffer = sampling_buffer;
	dev->driver_data.sampling.buffer_size = 2 * sampling_count;
	dev->driver_data.sampling.resolution = 10;
	dev->driver_data.sampling.oversampling = 0;
	dev->driver_data.sampling.calibrate = false;

	adc_channel_setup(adc_dev, &dev->driver_data.channel);

	dev->driver_data.avg_adc_val = minode_analog_retrieve(dev);

	k_work_init(&dev->driver_data.work, work_handler);
	k_timer_init(&dev->driver_data.timer, timer_expiry_handler, NULL);

	return 0;
}

void minode_analog_start_sampling(struct minode_analog_device *dev, int period_ms)
{
	if (!dev) {
		return;
	}

	k_timer_start(&dev->driver_data.timer, K_MSEC(period_ms), K_MSEC(period_ms));
}

void minode_analog_stop_sampling(struct minode_analog_device *dev)
{
	if (!dev) {
		return;
	}

	k_timer_stop(&dev->driver_data.timer);
}

u16_t minode_analog_retrieve(struct minode_analog_device *dev)
{
	int i;
	u32_t total_adc_val;
	u16_t adc_val;
	u16_t ref_val;
	u8_t *buffer;
	int sampling_count;

	if (!dev) {
		return -EINVAL;
	}

	buffer = dev->driver_data.sampling.buffer;
	sampling_count = dev->driver_data.sampling_options.extra_samplings + 1;
	ref_val = dev->driver_data.ref_val;

	adc_read(dev->driver_data.adc_dev, &dev->driver_data.sampling);

	total_adc_val = 0;
	for(i = 0; i < sampling_count; i++) {
		adc_val = ((buffer[2 * i + 1] & 0x03) << 8 | buffer[2 * i]);
		if (adc_val > ref_val) {
			adc_val -= ref_val;
		} else {
			adc_val = ref_val - adc_val;
		}
		total_adc_val += adc_val;
	}

	adc_val = total_adc_val / sampling_count;
	return adc_val;
}

u16_t minode_analog_get(struct minode_analog_device *dev)
{
	if (!dev) {
		return -EINVAL;
	}

	return dev->driver_data.avg_adc_val;
}