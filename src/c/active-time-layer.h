#pragma once
#include <pebble.h>

typedef Layer ActiveTimeLayer;

ActiveTimeLayer *active_time_layer_create(void);
void active_time_layer_destroy(ActiveTimeLayer *this);
