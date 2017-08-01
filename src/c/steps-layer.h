#pragma once
#include <pebble.h>

typedef Layer StepsLayer;

StepsLayer *steps_layer_create(GRect frame);
void steps_layer_destroy(StepsLayer *this);
