#include <drivers/gpio.h>

#include "minode_connector.h"
#include "minode_switch.h"

int minode_switch_retrieve_status(struct minode_switch_device *dev, bool if_cache)
{
	u32_t val;
	int status;

	gpio_pin_read(dev->driver_data->gpio.dev, dev->driver_data->gpio.pin, &val);

	status = val ? 0: 1;
	if (if_cache) {
		dev->driver_data->status = status;
	}
	return status;
}

int minode_switch_status(struct minode_switch_device *dev)
{
	return dev->driver_data->status;
}

bool minode_switch_is_on(struct minode_switch_device *dev)
{
	return dev->driver_data->status ? true : false;
}

int minode_switch_enable_callback(struct minode_switch_device *dev)
{
	return gpio_pin_enable_callback(dev->driver_data->gpio.dev, dev->driver_data->gpio.pin);
}

int minode_switch_disable_callback(struct minode_switch_device *dev)
{
	return gpio_pin_disable_callback(dev->driver_data->gpio.dev, dev->driver_data->gpio.pin);
}

static void gpio_handler_switch(struct device *gpio_device, struct gpio_callback *cb,
			   u32_t pins)
{
	int new_status;
	struct minode_connector_gpio *gpio = CONTAINER_OF(cb, struct minode_connector_gpio, callback);
	struct minode_switch_driver_data *driver_data = CONTAINER_OF(gpio, struct minode_switch_driver_data, gpio);
	struct minode_switch_device *dev = CONTAINER_OF(driver_data, struct minode_switch_device, driver_data);

	new_status = minode_switch_retrieve_status(dev, false);

	// Switch status doesn't change, ignore this interrupt
	if (driver_data->status == new_status) {
		return;
	}
	// Update onoff status cache
	driver_data->status = new_status;

	if (new_status) {
		if (dev->handler_on) {
			dev->handler_on(dev);
		}
	} else {
		if (dev->handler_off) {
			dev->handler_off(dev);
		}
	}
}

int minode_switch_init(struct minode_switch_device *dev)
{
	if(!dev) {
		return -EINVAL;
	}

	if(!dev->connector) {
		return -EINVAL;
	}

	dev->driver_data->gpio.dev = minode_connector_get_gpio_device(dev->connector, 0);
	if (!dev->driver_data->gpio.dev) {
		return -ENODEV;
	}

	dev->driver_data->gpio.pin = minode_connector_get_gpio_pin(dev->connector, 0);
	if (dev->driver_data->gpio.pin == -1) {
		return -ENODEV;
	}

	gpio_pin_configure(dev->driver_data->gpio.dev, dev->driver_data->gpio.pin,
				(GPIO_DIR_IN | GPIO_INT | GPIO_INT_EDGE | GPIO_INT_DOUBLE_EDGE));

	// Init onoff status
	minode_switch_retrieve_status(dev, true);

	gpio_init_callback(&dev->driver_data->gpio.callback, gpio_handler_switch,
				BIT(dev->driver_data->gpio.pin));
	gpio_add_callback(dev->driver_data->gpio.dev, &dev->driver_data->gpio.callback);

	minode_switch_enable_callback(dev);

	return 0;
}