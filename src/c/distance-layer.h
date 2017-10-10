#pragma once
#include <pebble.h>

typedef Layer DistanceLayer;

DistanceLayer *distance_layer_create(void);
void distance_layer_destroy(DistanceLayer *this);
