#ifndef ZEPHYR_INCLUDE_LIB_MINODE_DHT11_H_
#define ZEPHYR_INCLUDE_LIB_MINODE_DHT11_H_

#include <drivers/sensor.h>
#include <minode_connector.h>

struct minode_dht_driver_data {
	struct device *gpio;
	u32_t pin;
	u8_t sample[4];
	struct k_timer timer;
	struct k_work work;
};

struct minode_dht_device;
typedef void (*minode_dht_callback_handler_t)(struct minode_dht_device *dev);

struct minode_dht_device {
	const char *connector;
	void *user_data;
	minode_dht_callback_handler_t on_new_sampling;
	struct minode_dht_driver_data driver_data;
};

#define MINODE_DHT_DEVICE_DEFINE(_device_name, _conn_name, _user_data,          \
					_handler_new_sampling)                                                \
	static __used struct minode_dht_device _device_name = {                       \
		.connector = STRINGIFY(_conn_name),                                         \
		.user_data = _user_data,                                                    \
		.on_new_sampling = _handler_new_sampling                                    \
	}

int minode_dht_init(struct minode_dht_device *dev);

int minode_dht_sample_fetch(struct minode_dht_device *dev);
int minode_dht_channel_get(struct minode_dht_device *dev,
			   enum sensor_channel chan,
			   struct sensor_value *val);
int minode_dht_humidity_get(struct minode_dht_device *dev, struct sensor_value *val);
int minode_dht_ambient_temp_get(struct minode_dht_device *dev, struct sensor_value *val);

void minode_dht_start_sampling(struct minode_dht_device *dev, int period_ms);
void minode_dht_stop_sampling(struct minode_dht_device *dev);

#endif