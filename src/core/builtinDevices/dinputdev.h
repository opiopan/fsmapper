//
// dinputdev.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include "mapperplugin.h"

static constexpr auto JOYSTICK_AXIS_VALUE_MAX = 1023;
static constexpr auto JOYSTICK_AXIS_VALUE_MIN = -1023;

extern MAPPER_PLUGIN_DEVICE_OPS* dinput_PluginDeviceOps;
