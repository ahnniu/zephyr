#include <drivers/gpio.h>
#include <string.h>

#include "minode_connector.h"

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

struct device *minode_connector_get_gpio_device(const char* conn_name, int pin_id)
{
	int i;
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
	for (i = 0; i < ARRAY_SIZE(connectors); i++) {
		if (strcmp(connectors[i].name, conn_name) == 0) {
			return connectors[i].p[pin_id].pin;
		}
	}

	return -1;
}