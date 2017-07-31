#pragma once
#include <pebble.h>

typedef Layer SecondsLayer;

SecondsLayer *seconds_layer_create(GRect frame);
void seconds_layer_destroy(SecondsLayer *this);
