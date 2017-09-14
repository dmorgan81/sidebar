#pragma once
#include <pebble.h>

typedef Layer BatteryLayer;

BatteryLayer *battery_layer_create(void);
void battery_layer_destroy(BatteryLayer *this);
