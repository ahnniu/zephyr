#ifndef ZEPHYR_INCLUDE_LIB_MINODE_CONNECTOR_H_
#define ZEPHYR_INCLUDE_LIB_MINODE_CONNECTOR_H_

#include <drivers/gpio.h>

struct minode_connector_gpio {
	struct device *dev;
	u32_t pin;
	struct gpio_callback callback;
};

struct device *minode_connector_get_gpio_device(const char* conn_name, int pin_id);
u32_t minode_connector_get_gpio_pin(const char* conn_name, int pin_id);

struct device *minode_connector_get_adc_device(const char* conn_name, int pin_id);
u8_t minode_connector_get_adc_input(const char* conn_name, int pin_id);
u8_t minode_connector_allocate_adc_channel_id();

#endif