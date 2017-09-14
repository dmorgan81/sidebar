#pragma once
#include <pebble.h>

typedef Layer AltTimeLayer;

AltTimeLayer *alt_time_layer_create(void);
void alt_time_layer_destroy(AltTimeLayer *this);
