#
# Copyright (c) 2019 Embest Tech Co., Ltd
#
# SPDX -License-Identifier: Apache-2.0
#

zephyr_system_include_directories(include)

zephyr_library()

zephyr_library_sources_ifdef(
  CONFIG_ELEMENT14_MINODE_LIB
  source/minode_connector.c
  source/minode_analog.c
)

zephyr_library_sources_ifdef(CONFIG_MICROBIT_LIB_BUTTON source/mb_button.c)
zephyr_library_sources_ifdef(CONFIG_ELEMENT14_MINODE_LIB_SWITCH source/minode_switch.c)
zephyr_library_sources_ifdef(CONFIG_ELEMENT14_MINODE_LIB_LIGHT_SENSOR source/minode_light_sensor.c)
zephyr_library_sources_ifdef(CONFIG_ELEMENT14_MINODE_LIB_ROTARY source/minode_rotary.c)
zephyr_library_sources_ifdef(CONFIG_ELEMENT14_MINODE_LIB_SOUND_SENSOR source/minode_sound_sensor.c)
zephyr_library_sources_ifdef(CONFIG_ELEMENT14_MINODE_LIB_DHT11 source/minode_dht11.c)


