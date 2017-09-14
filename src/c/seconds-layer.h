#pragma once
#include <pebble.h>

typedef Layer SecondsLayer;

SecondsLayer *seconds_layer_create(void);
void seconds_layer_destroy(SecondsLayer *this);
