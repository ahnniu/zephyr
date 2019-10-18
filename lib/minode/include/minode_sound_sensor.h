#ifndef ZEPHYR_INCLUDE_LIB_MINODE_SOUND_SENSOR_H_
#define ZEPHYR_INCLUDE_LIB_MINODE_SOUND_SENSOR_H_

#include <minode_analog.h>

#define MINODE_SOUND_SAMPLINGS_COUNT CONFIG_ELEMENT14_MINODE_LIB_SOUND_SENSOR_SAMPLINGS_COUNT
#define MINODE_SOUND_SAMPLINGS_PEROID_MS CONFIG_ELEMENT14_MINODE_LIB_SOUND_SENSOR_SAMPLINGS_PERIOD_MS
#define MINODE_SOUND_NOISE_HIGHEST_FREQ_HZ CONFIG_ELEMENT14_MINODE_LIB_SOUND_SENSOR_NOISE_HIGHEST_FREQ_HZ
#define MINODE_SOUND_NOISE_LOWEST_AMPLITUDE_ADC_VAL CONFIG_ELEMENT14_MINODE_LIB_SOUND_SENSOR_NOISE_LOWEST_AMPLITUDE_ADC_VAL
// Vp = 3.3V / 2 = 1.65V;
#define MINODE_SOUND_REFERENCE_ADC_VAL 512

struct minode_sound_driver_data {
	struct minode_analog_device analog_dev;
	u8_t buffer[2 * MINODE_SOUND_SAMPLINGS_COUNT];
};

struct minode_sound_device;

typedef void (*minode_sound_callback_handler_t)(struct minode_sound_device *dev);

struct minode_sound_device {
	const char *connector;
	void *user_data;
	minode_sound_callback_handler_t on_noise_detect;
	struct minode_sound_driver_data driver_data;
};

#define MINODE_SOUND_DEVICE_DEFINE(_device_name, _conn_name, _user_data,        \
					_handler_noise_detect)                                                \
	static __used struct minode_sound_device _device_name = {                     \
		.connector = STRINGIFY(_conn_name),                                         \
		.user_data = _user_data,                                                    \
		.on_noise_detect = _handler_noise_detect                                    \
	}

int minode_sound_init(struct minode_sound_device *dev);
void minode_sound_start_listening(struct minode_sound_device *dev);
void minode_sound_stop_listening(struct minode_sound_device *dev);
int minode_sound_is_in_noise(struct minode_sound_device *dev);

#endif