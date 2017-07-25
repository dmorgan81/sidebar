#include <pebble.h>
#include "logging.h"
#include "time-layer.h"
#include "date-layer.h"
#include "status-layer.h"
#include "battery-layer.h"

#define WIDGET_HEIGHT 56

static Window *s_window;
static TimeLayer *s_time_layer;
static DateLayer *s_date_layer;
static StatusLayer *s_status_layer;
static BatteryLayer *s_battery_layer;

static void prv_update_proc(Layer *this, GContext *ctx) {
    logf();
    GRect bounds = layer_get_bounds(this);

    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorVividCerulean, GColorLightGray));
    graphics_fill_rect(ctx, GRect(bounds.size.w - ACTION_BAR_WIDTH, 0, ACTION_BAR_WIDTH, bounds.size.h), 0, GCornerNone);
}

static void prv_window_load(Window *window) {
    logf();
    Layer *root_layer = window_get_root_layer(window);
    layer_set_update_proc(root_layer, prv_update_proc);
    GRect bounds = layer_get_bounds(root_layer);

    s_time_layer = time_layer_create(GRect(-1, 12, bounds.size.w - ACTION_BAR_WIDTH, bounds.size.h - 24));
    layer_add_child(root_layer, s_time_layer);

    int16_t x = bounds.size.w - ACTION_BAR_WIDTH;

    s_date_layer = date_layer_create(GRect(x, 0, ACTION_BAR_WIDTH, WIDGET_HEIGHT));
    layer_add_child(root_layer, s_date_layer);

    s_status_layer = status_layer_create(GRect(x, WIDGET_HEIGHT, ACTION_BAR_WIDTH, WIDGET_HEIGHT));
    layer_add_child(root_layer, s_status_layer);

    s_battery_layer = battery_layer_create(GRect(x, bounds.size.h - WIDGET_HEIGHT, ACTION_BAR_WIDTH, WIDGET_HEIGHT));
    layer_add_child(root_layer, s_battery_layer);
}

static void prv_window_unload(Window *window) {
    logf();
    battery_layer_destroy(s_battery_layer);
    status_layer_destroy(s_status_layer);
    date_layer_destroy(s_date_layer);
    time_layer_destroy(s_time_layer);
}

static void prv_init(void) {
    logf();
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = prv_window_load,
        .unload = prv_window_unload
    });
    window_stack_push(s_window, true);
}

static void prv_deinit(void) {
    logf();
    window_destroy(s_window);
}

int main(void) {
    prv_init();
    app_event_loop();
    prv_deinit();
}
