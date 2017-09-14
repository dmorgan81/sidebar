#pragma once
#include <pebble.h>

typedef Layer WeatherLayer;

WeatherLayer *weather_layer_create(void);
void weather_layer_destroy(WeatherLayer *this);
