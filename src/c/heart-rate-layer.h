#pragma once
#include <pebble.h>

typedef Layer HeartRateLayer;

HeartRateLayer *heart_rate_layer_create(GRect frame);
void heart_rate_layer_destroy(HeartRateLayer *this);
