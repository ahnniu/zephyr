#ifndef ZEPHYR_INCLUDE_LIB_MINODE_ROTARY_H_
#define ZEPHYR_INCLUDE_LIB_MINODE_ROTARY_H_

#include <minode_analog.h>

#define MINODE_ROTARY_SAMPLINGS_PEROID_MS CONFIG_ELEMENT14_MINODE_LIB_ROTARY_SAMPLINGS_PERIOD_MS

enum minode_rotary_level {
	MINODE_ROTARY_LEVEL_0 = 0,
	MINODE_ROTARY_LEVEL_1 = 1,
	MINODE_ROTARY_LEVEL_2 = 2,
	MINODE_ROTARY_LEVEL_3 = 3,
	MINODE_ROTARY_LEVEL_4 = 4,
	MINODE_ROTARY_LEVEL_5 = 5,
	MINODE_ROTARY_LEVEL_6 = 6,
	MINODE_ROTARY_LEVEL_7 = 7,
	MINODE_ROTARY_LEVEL_8 = 8,
	MINODE_ROTARY_LEVEL_9 = 9,
	MINODE_ROTARY_LEVEL_INVAL = -22
};

struct minode_rotary_driver_data {
	struct minode_analog_device analog_dev;
	u8_t buffer[2];
};

struct minode_rotary_device;

typedef void (*minode_rotary_callback_handler_t)(struct minode_rotary_device *dev,
					enum minode_rotary_level prev_level, enum minode_rotary_level new_level);

struct minode_rotary_device {
	const char *connector;
	void *user_data;
	minode_rotary_callback_handler_t on_level_change;
	struct minode_rotary_driver_data driver_data;
};

#define MINODE_ROTARY_DEVICE_DEFINE(_device_name, _conn_name, _user_data,       \
					_handler_level_change)                                                \
	static __used struct minode_rotary_device _device_name = {                    \
		.connector = STRINGIFY(_conn_name),                                         \
		.user_data = _user_data,                                                    \
		.on_level_change = _handler_level_change                                    \
	}

int minode_rotary_init(struct minode_rotary_device *dev);
enum minode_rotary_level minode_rotary_retrieve(struct minode_rotary_device *dev);
enum minode_rotary_level minode_rotary_get(struct minode_rotary_device *dev);
void minode_rotary_start_listening(struct minode_rotary_device *dev);
void minode_rotary_stop_listening(struct minode_rotary_device *dev);

#endif