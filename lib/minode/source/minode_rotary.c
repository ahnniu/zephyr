#include <minode_rotary.h>

static enum minode_rotary_level rotary_adc_to_level(u16_t adc_val)
{
	return (adc_val * 10) / 1024;
}

static void rotary_on_new_sampling(struct minode_analog_device *analog_dev,
			u16_t prev_adc_val, u16_t new_adc_val)
{
	enum minode_rotary_level prev_level, new_level;
	struct minode_rotary_driver_data *driver_data = CONTAINER_OF(analog_dev, struct minode_rotary_driver_data, analog_dev);
	struct minode_rotary_device *dev = CONTAINER_OF(driver_data, struct minode_rotary_device, driver_data);

	prev_level = rotary_adc_to_level(prev_adc_val);
	new_level = rotary_adc_to_level(new_adc_val);

	if (prev_level != new_level) {
		if (dev->on_level_change) {
			dev->on_level_change(dev, prev_level, new_level);
		}
	}
}

int minode_rotary_init(struct minode_rotary_device *dev)
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
	analog_dev->on_new_sampling = rotary_on_new_sampling;
	return minode_analog_init(analog_dev, dev->driver_data.buffer, 1, 0, 0);
}

enum minode_rotary_level minode_rotary_retrieve(struct minode_rotary_device *dev)
{
	u16_t adc_val;
	struct minode_analog_device *analog_dev;

	if (!dev) {
		return MINODE_ROTARY_LEVEL_INVAL;
	}

	analog_dev = &dev->driver_data.analog_dev;
	adc_val = minode_analog_retrieve(analog_dev);
	return rotary_adc_to_level(adc_val);
}

enum minode_rotary_level minode_rotary_get(struct minode_rotary_device *dev)
{
	u16_t adc_val;
	struct minode_analog_device *analog_dev;

	if (!dev) {
		return MINODE_ROTARY_LEVEL_INVAL;
	}

	analog_dev = &dev->driver_data.analog_dev;
	adc_val = analog_dev->driver_data.avg_adc_val;
	return rotary_adc_to_level(adc_val);
}

void minode_rotary_start_listening(struct minode_rotary_device *dev)
{
	struct minode_analog_device *analog_dev;

	if (!dev) {
		return;
	}

	analog_dev = &dev->driver_data.analog_dev;
	minode_analog_start_sampling(analog_dev, MINODE_ROTARY_SAMPLINGS_PEROID_MS);
}

void minode_rotary_stop_listening(struct minode_rotary_device *dev)
{
	struct minode_analog_device *analog_dev;

	if (!dev) {
		return;
	}

	analog_dev = &dev->driver_data.analog_dev;
	minode_analog_stop_sampling(analog_dev);
}