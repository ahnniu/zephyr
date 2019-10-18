#include <minode_sound_sensor.h>

static void sound_on_new_sampling(struct minode_analog_device *analog_dev,
			u16_t prev_adc_val, u16_t new_adc_val)
{
	struct minode_sound_driver_data *driver_data = CONTAINER_OF(analog_dev, struct minode_sound_driver_data, analog_dev);
	struct minode_sound_device *dev = CONTAINER_OF(driver_data, struct minode_sound_device, driver_data);

	if (new_adc_val > MINODE_SOUND_NOISE_LOWEST_AMPLITUDE_ADC_VAL) {
		if (dev->on_noise_detect) {
			dev->on_noise_detect(dev);
		}
	}
}

int minode_sound_init(struct minode_sound_device *dev)
{
	struct minode_analog_device *analog_dev = &dev->driver_data.analog_dev;

	if (!dev) {
		return -EINVAL;
	}

	if (!dev->connector) {
		return -EINVAL;
	}

	analog_dev->connector = dev->connector;
	analog_dev->user_data = dev->user_data;
	analog_dev->on_new_sampling = sound_on_new_sampling;
	return minode_analog_init(analog_dev,
					dev->driver_data.buffer,
					MINODE_SOUND_SAMPLINGS_COUNT,
					MINODE_SOUND_NOISE_HIGHEST_FREQ_HZ,
					MINODE_SOUND_REFERENCE_ADC_VAL);
}

void minode_sound_start_listening(struct minode_sound_device *dev)
{
	struct minode_analog_device *analog_dev;

	if (!dev) {
		return;
	}

	analog_dev = &dev->driver_data.analog_dev;
	minode_analog_start_sampling(analog_dev, MINODE_SOUND_SAMPLINGS_PEROID_MS);
}

void minode_sound_stop_listening(struct minode_sound_device *dev)
{
	struct minode_analog_device *analog_dev;

	if (!dev) {
		return;
	}

	analog_dev = &dev->driver_data.analog_dev;
	minode_analog_stop_sampling(analog_dev);
}

int minode_sound_is_in_noise(struct minode_sound_device *dev)
{
	u16_t adc_val;
	struct minode_analog_device *analog_dev;

	if (!dev) {
		return -EINVAL;
	}

	analog_dev = &dev->driver_data.analog_dev;
	adc_val = analog_dev->driver_data.avg_adc_val;

	return (adc_val > MINODE_SOUND_NOISE_LOWEST_AMPLITUDE_ADC_VAL) ? 1 : 0;
}