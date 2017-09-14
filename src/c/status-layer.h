#pragma once
#include <pebble.h>

typedef Layer StatusLayer;

StatusLayer *status_layer_create(void);
void status_layer_destroy(StatusLayer *this);
