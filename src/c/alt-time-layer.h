#pragma once
#include <pebble.h>

typedef Layer AltTimeLayer;

AltTimeLayer *alt_time_layer_create(GRect frame);
void alt_time_layer_destroy(AltTimeLayer *this);
