#ifndef ZEPHYR_INCLUDE_LIB_MINODE_LIGHT_SENSOR_H_
#define ZEPHYR_INCLUDE_LIB_MINODE_LIGHT_SENSOR_H_

#include <drivers/adc.h>

#define MINODE_LIGHT_SAMPLINGS_COUNT CONFIG_ELEMENT14_MINODE_LIB_LIGHT_SAMPLINGS_COUNT
#define MINODE_LIGHT_SAMPLINGS_PEROID_MS (1 * 1000)

// MINODE_LIGHT_LEVEL_0 means brightest
// MINODE_LIGHT_LEVEL_4 means darkest
enum minode_light_level {
	MINODE_LIGHT_LEVEL_0 = 0,
	MINODE_LIGHT_LEVEL_1 = 1,
	MINODE_LIGHT_LEVEL_2 = 2,
	MINODE_LIGHT_LEVEL_3 = 3,
	MINODE_LIGHT_LEVEL_4 = 4,
	MINODE_LIGHT_LEVEL_INVAL = -22
};

struct minode_light_driver_data {
	struct device *adc_dev;
	struct adc_channel_cfg channel;
	struct adc_sequence sampling;
	struct adc_sequence_options sampling_options;
	u8_t buffer[2* MINODE_LIGHT_SAMPLINGS_COUNT];	// 2 bytes for each sample (resolution = 10)
	struct k_timer timer;
	struct k_work work;
	enum minode_light_level level;
};
struct minode_light_device;

typedef void (*minode_light_callback_handler_t)(struct minode_light_device *dev,
							enum minode_light_level prev_level, enum minode_light_level new_level);

struct minode_light_device {
	const char *connector;
	void *user_data;
	minode_light_callback_handler_t on_level_change;
	struct minode_light_driver_data driver_data;
};

#define MINODE_LIGHT_DEVICE_DEFINE(_device_name, _conn_name, _user_data,        \
					_handler_level_change)                                                \
	static __used struct minode_light_device _device_name = {                     \
		.connector = STRINGIFY(_conn_name),                                         \
		.user_data = _user_data,                                                    \
		.on_level_change = _handler_level_change                                    \
	}

int minode_light_init(struct minode_light_device *dev);
void minode_light_start_listening(struct minode_light_device *dev);
void minode_light_stop_listening(struct minode_light_device *dev);
enum minode_light_level minode_light_retrieve_level(struct minode_light_device *dev);
enum minode_light_level minode_light_get_level(struct minode_light_device *dev);


#endif