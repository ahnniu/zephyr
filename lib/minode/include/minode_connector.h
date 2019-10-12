#ifndef ZEPHYR_INCLUDE_LIB_MINODE_CONNECTOR_H_
#define ZEPHYR_INCLUDE_LIB_MINODE_CONNECTOR_H_

struct minode_connector_gpio {
	struct device *dev;
	u32_t pin;
	struct gpio_callback callback;
};

struct device *minode_connector_get_gpio_device(const char* conn_name, int pin_id);
u32_t minode_connector_get_gpio_pin(const char* conn_name, int pin_id);

#endif