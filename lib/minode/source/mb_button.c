#include <string.h>
#include <drivers/gpio.h>
#include <mb_button.h>

struct mb_button_pin {
	const char *port;
	u32_t pin;
	u32_t flag;
};

struct mb_button_device {
	const char *name;
	void *user_data;
	mb_button_callback_handler_t on_pressed;
	u32_t uptime_pressed;
	struct mb_button_pin p;
	struct device *gpio_dev;
	struct gpio_callback io_callback;
	struct k_work work;
};

#define MB_BUTTON(id)                                           \
	{                                                             \
		.name = _CONCAT(_CONCAT(DT_GPIO_KEYS_BUTTON_, id), _LABEL), \
		.user_data = NULL,                                          \
		.on_pressed = NULL,                                         \
		.uptime_pressed = 0,                                        \
		.p = _CONCAT(_CONCAT(DT_GPIO_KEYS_BUTTON_, id), _GPIOS)     \
	}

static struct mb_button_device buttons[] = {
	MB_BUTTON(0),
	MB_BUTTON(1)
};

struct mb_button_device *mb_button_device_get(const char *name)
{
	int i;

	if (!name) {
		return NULL;
	}

	for (i = 0; i < ARRAY_SIZE(buttons); i++) {
		if (strcmp(buttons[i].name, name) == 0) {
			return &buttons[i];
		}
	}

	return NULL;
}

static void button_pressed_worker(struct k_work *work)
{
	struct mb_button_device *dev = CONTAINER_OF(work, struct mb_button_device, work);

	if (dev->on_pressed) {
		dev->on_pressed(dev, dev->user_data);
	}
}

static void button_pressed(struct device *gpio_dev, struct gpio_callback *cb,
			   u32_t pins)
{
	u32_t time;
	struct mb_button_device *dev;

	time = k_uptime_get_32();
	dev = CONTAINER_OF(cb, struct mb_button_device, io_callback);

	if(time < dev->uptime_pressed + MB_BUTTON_DEBOUNCE_DELAY_MS) {
		dev->uptime_pressed = time;
		return;
	}

	dev->uptime_pressed = time;
	k_work_submit(&dev->work);
}

int mb_button_init(struct mb_button_device *dev,
			mb_button_callback_handler_t on_pressed_handler,
			void *user_data,
			bool if_enable_callback)
{
	struct device *gpio_dev;

	if (!dev) {
		return -EINVAL;
	}

	if (!on_pressed_handler) {
		return -EINVAL;
	}

	dev->on_pressed = on_pressed_handler;
	dev->user_data = user_data;

	k_work_init(&dev->work, button_pressed_worker);

	gpio_dev = device_get_binding(dev->p.port);

	if (!gpio_dev) {
		return -ENODEV;
	}

	dev->gpio_dev = gpio_dev;

	gpio_pin_configure(gpio_dev, dev->p.pin,
			   (GPIO_DIR_IN | GPIO_INT | GPIO_INT_EDGE | GPIO_INT_ACTIVE_LOW));

	gpio_init_callback(&dev->io_callback, button_pressed, BIT(dev->p.pin));
	gpio_add_callback(gpio_dev, &dev->io_callback);

	if (if_enable_callback) {
		return gpio_pin_enable_callback(gpio_dev, dev->p.pin);
	}

	return 0;
}

int mb_button_enable_callback(struct mb_button_device *dev)
{
	if (!dev) {
		return -EINVAL;
	}

	return gpio_pin_enable_callback(dev->gpio_dev, dev->p.pin);
}

int mb_button_disable_callback(struct mb_button_device *dev)
{
	if (!dev) {
		return -EINVAL;
	}

	return gpio_pin_disable_callback(dev->gpio_dev, dev->p.pin);
}