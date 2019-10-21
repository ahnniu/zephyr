#ifndef ZEPHYR_INCLUDE_LIB_MINODE_H_
#define ZEPHYR_INCLUDE_LIB_MINODE_H_

#include <minode_connector.h>
#include <minode_analog.h>

#ifdef CONFIG_ELEMENT14_MINODE_LIB_SWITCH
#include <minode_switch.h>
#endif

#ifdef CONFIG_ELEMENT14_MINODE_LIB_LIGHT_SENSOR
#include <minode_light_sensor.h>
#endif

#ifdef CONFIG_ELEMENT14_MINODE_LIB_ROTARY
#include <minode_rotary.h>
#endif

#ifdef CONFIG_ELEMENT14_MINODE_LIB_SOUND_SENSOR
#include <minode_sound_sensor.h>
#endif

#ifdef CONFIG_ELEMENT14_MINODE_LIB_DHT11
#include <minode_dht11.h>
#endif

#endif