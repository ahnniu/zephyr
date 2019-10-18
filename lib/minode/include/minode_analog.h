#ifndef ZEPHYR_INCLUDE_LIB_MINODE_ANALOG_H_
#define ZEPHYR_INCLUDE_LIB_MINODE_ANALOG_H_

#include <drivers/adc.h>

struct minode_analog_driver_data {
	struct device *adc_dev;
	struct adc_channel_cfg channel;
	struct adc_sequence sampling;
	struct adc_sequence_options sampling_options;
	struct k_timer timer;
	struct k_work work;
	u16_t avg_adc_val;
};

struct minode_analog_device;

typedef void (*minode_analog_callback_handler_t)(struct minode_analog_device *dev,
					u16_t prev_adc_val, u16_t new_adc_val);

struct minode_analog_device {
	const char *connector;
	void *user_data;
	minode_analog_callback_handler_t on_new_sampling;
	struct minode_analog_driver_data driver_data;
};

#define MINODE_ANALOG_DEVICE_DEFINE(_device_name, _conn_name, _user_data,       \
					_handler_new_sampling)                                                \
	static __used struct minode_analog_device _device_name = {                    \
		.connector = STRINGIFY(_conn_name),                                         \
		.user_data = _user_data,                                                    \
		.on_new_sampling = _handler_new_sampling                                    \
	}

int minode_analog_init(struct minode_analog_device *dev, u8_t *sampling_buffer, int sampling_count, int sampling_freq_hz);
void minode_analog_start_sampling(struct minode_analog_device *dev, int period_ms);
void minode_analog_stop_sampling(struct minode_analog_device *dev);
u16_t minode_analog_retrieve(struct minode_analog_device *dev);
u16_t minode_analog_get(struct minode_analog_device *dev);

#endif