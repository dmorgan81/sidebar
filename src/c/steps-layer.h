#pragma once
#include <pebble.h>

typedef Layer StepsLayer;

StepsLayer *steps_layer_create(void);
void steps_layer_destroy(StepsLayer *this);
