#include <string.h>

#include <minode_connector.h>

struct minode_connector_pin {
  const char *port;
  u32_t pin;
  u32_t flag;
};

struct minode_connector {
  const char *name;
  struct minode_connector_pin p[2];
};

#define MINODE_CONNECTOR(_name)                             \
	{                                                         \
		.name = #_name,                                         \
		.p = DT_ELEMENT14_MINODE_CONNECTORS_ ## _name ## _GPIOS \
	}

static struct minode_connector connectors[] = {
	MINODE_CONNECTOR(A0),
	MINODE_CONNECTOR(A1),
	MINODE_CONNECTOR(A2),
	MINODE_CONNECTOR(D12),
	MINODE_CONNECTOR(D13),
	MINODE_CONNECTOR(D14),
	MINODE_CONNECTOR(D15)
};

static u8_t channel_id_pool = 0;

struct device *minode_connector_get_gpio_device(const char* conn_name, int pin_id)
{
	int i;

	if (!conn_name) {
		return NULL;
	}

	for (i = 0; i < ARRAY_SIZE(connectors); i++) {
		if (strcmp(connectors[i].name, conn_name) == 0) {
			return device_get_binding(connectors[i].p[pin_id].port);
		}
	}

	return NULL;
}

u32_t minode_connector_get_gpio_pin(const char* conn_name, int pin_id)
{
	int i;

	if (!conn_name) {
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(connectors); i++) {
		if (strcmp(connectors[i].name, conn_name) == 0) {
			return connectors[i].p[pin_id].pin;
		}
	}

	return -ENODEV;
}

struct device *minode_connector_get_adc_device(const char* conn_name, int pin_id)
{
	if (!conn_name) {
		return NULL;
	}

	if (*conn_name != 'A') {
		return NULL;
	}

	return device_get_binding("ADC_0");
}

u8_t minode_connector_allocate_adc_channel_id()
{
	if (channel_id_pool > 32) {
		return -ENODEV;
	}

	return channel_id_pool++;
}

u8_t minode_connector_get_adc_input(const char* conn_name, int pin_id)
{
	int i;

	if (!conn_name) {
		return EINVAL;
	}

	if (*conn_name != 'A') {
		return EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(connectors); i++) {
		if (strcmp(connectors[i].name, conn_name) == 0) {
			// flag is the ADC input id, 1 indicates AIN1
			// Here we return a nrf_adc_config_input_t enum type, More please refer to
			// Nordic nRF5 SDK in ../modules/hal/nordic

			// #define ADC_CONFIG_PSEL_Pos (8UL) /*!< Position of PSEL field. */
			// #define ADC_CONFIG_PSEL_Msk (0xFFUL << ADC_CONFIG_PSEL_Pos) /*!< Bit mask of PSEL field. */
			// #define ADC_CONFIG_PSEL_Disabled (0UL) /*!< Analog input pins disabled. */
			// #define ADC_CONFIG_PSEL_AnalogInput0 (1UL) /*!< Use analog input 0 as analog input. */
			// #define ADC_CONFIG_PSEL_AnalogInput1 (2UL) /*!< Use analog input 1 as analog input. */
			// #define ADC_CONFIG_PSEL_AnalogInput2 (4UL) /*!< Use analog input 2 as analog input. */
			// #define ADC_CONFIG_PSEL_AnalogInput3 (8UL) /*!< Use analog input 3 as analog input. */
			// #define ADC_CONFIG_PSEL_AnalogInput4 (16UL) /*!< Use analog input 4 as analog input. */
			// #define ADC_CONFIG_PSEL_AnalogInput5 (32UL) /*!< Use analog input 5 as analog input. */
			// #define ADC_CONFIG_PSEL_AnalogInput6 (64UL) /*!< Use analog input 6 as analog input. */
			// #define ADC_CONFIG_PSEL_AnalogInput7 (128UL) /*!< Use analog input 7 as analog input. */

			// typedef enum
			// {
			//     NRF_ADC_CONFIG_INPUT_DISABLED = ADC_CONFIG_PSEL_Disabled,     /**< No input selected. */
			//     NRF_ADC_CONFIG_INPUT_0        = ADC_CONFIG_PSEL_AnalogInput0, /**< Input 0. */
			//     NRF_ADC_CONFIG_INPUT_1        = ADC_CONFIG_PSEL_AnalogInput1, /**< Input 1. */
			//     NRF_ADC_CONFIG_INPUT_2        = ADC_CONFIG_PSEL_AnalogInput2, /**< Input 2. */
			//     NRF_ADC_CONFIG_INPUT_3        = ADC_CONFIG_PSEL_AnalogInput3, /**< Input 3. */
			//     NRF_ADC_CONFIG_INPUT_4        = ADC_CONFIG_PSEL_AnalogInput4, /**< Input 4. */
			//     NRF_ADC_CONFIG_INPUT_5        = ADC_CONFIG_PSEL_AnalogInput5, /**< Input 5. */
			//     NRF_ADC_CONFIG_INPUT_6        = ADC_CONFIG_PSEL_AnalogInput6, /**< Input 6. */
			//     NRF_ADC_CONFIG_INPUT_7        = ADC_CONFIG_PSEL_AnalogInput7, /**< Input 7. */
			// } nrf_adc_config_input_t;

			return (1 << connectors[i].p[pin_id].flag);
		}
	}

	return -ENODEV;
}

struct device *minode_connector_get_pwm_device(const char* conn_name, int pin_id)
{
	return device_get_binding("SW_PWM");
}

u32_t minode_connector_get_pwm_pin(const char* conn_name, int pin_id)
{
	u32_t pin;

	pin = minode_connector_get_gpio_pin(conn_name, pin_id);

	if (pin < 0 || pin > 31) {
		return -ENODEV;
	}

	return 1 << pin;
}