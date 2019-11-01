#ifndef ZEPHYR_INCLUDE_LIB_MB_BUTTON_H_
#define ZEPHYR_INCLUDE_LIB_MB_BUTTON_H_

#define MB_BUTTON_DEBOUNCE_DELAY_MS 250

struct mb_button_device;
typedef void (*mb_button_callback_handler_t)(struct mb_button_device *dev, void *user_data);

struct mb_button_device *mb_button_device_get(const char *name);
int mb_button_init(struct mb_button_device *dev,
			mb_button_callback_handler_t on_pressed_handler,
			void *user_data,
			bool if_enable_callback);
int mb_button_enable_callback(struct mb_button_device *dev);
int mb_button_disable_callback(struct mb_button_device *dev);

#endif