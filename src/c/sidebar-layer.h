#pragma once
#include <pebble.h>

typedef Layer SidebarLayer;

SidebarLayer *sidebar_layer_create(GRect frame);
void sidebar_layer_destroy(SidebarLayer *this);
