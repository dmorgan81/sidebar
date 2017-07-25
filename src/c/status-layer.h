#pragma once
#include <pebble.h>

typedef Layer StatusLayer;

StatusLayer *status_layer_create(GRect frame);
void status_layer_destroy(StatusLayer *this);
