#ifndef ZEPHYR_INCLUDE_LIB_MINODE_SWITCH_H_
#define ZEPHYR_INCLUDE_LIB_MINODE_SWITCH_H_

struct minode_switch_driver_data {
	struct minode_connector_gpio gpio;
	int status;
};

struct minode_switch_device;

typedef void (*minode_switch_callback_handler_t)(struct minode_switch_device *dev);

struct minode_switch_device {
	const char *connector;
	void *user_data;
	minode_switch_callback_handler_t handler_on;
	minode_switch_callback_handler_t handler_off;
	struct minode_switch_driver_data driver_data;
};

#define MINODE_SWITCH_DEVICE_DEFINE(_device_name, _conn_name, _user_data,       \
					_handler_on, _handler_off)                                            \
	static __used struct minode_switch_device _device_name = {                    \
		.connector = STRINGIFY(_conn_name),                                         \
		.user_data = _user_data,                                                    \
		.handler_on = _handler_on,                                                  \
		.handler_off = _handler_off                                                 \
	}

int minode_switch_init(struct minode_switch_device *dev);

int minode_switch_enable_callback(struct minode_switch_device *dev);
int minode_switch_disable_callback(struct minode_switch_device *dev);

int minode_switch_retrieve_status(struct minode_switch_device *dev, bool if_cache);
int minode_switch_status(struct minode_switch_device *dev);
bool minode_switch_is_on(struct minode_switch_device *dev);

#endif