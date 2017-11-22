#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <pebble-connection-vibes/connection-vibes.h>
#include <pebble-hourly-vibes/hourly-vibes.h>
#include <enamel.h>
#include "logging.h"
#include "weather.h"
#include "sidebar-layer.h"
#include "time-layer.h"

static Window *s_window;
static SidebarLayer *s_sidebar_layer;
static TimeLayer *s_time_layer;

static EventHandle s_settings_event_handle;

static void prv_settings_handler(void *context) {
    logf();
    Layer *root_layer = window_get_root_layer(s_window);

    connection_vibes_set_state(atoi(enamel_get_CONNECTION_VIBE()));
    hourly_vibes_set_enabled(enamel_get_HOURLY_VIBE());

#ifdef PBL_RECT
    GRect bounds = layer_get_bounds(root_layer);
    GRect frame = layer_get_frame(s_sidebar_layer);
    frame.origin.x = enamel_get_RIGHT_BAR() ? bounds.size.w - ACTION_BAR_WIDTH : 0;
    layer_set_frame(s_sidebar_layer, frame);

    frame = layer_get_frame(s_time_layer);
    frame.origin.x = enamel_get_RIGHT_BAR() ? 0 : ACTION_BAR_WIDTH;
    layer_set_frame(s_time_layer, frame);
#else
    layer_mark_dirty(root_layer);
#endif

    window_set_background_color(s_window, enamel_get_COLOR_BACKGROUND());
}

static void prv_window_load(Window *window) {
    logf();
    Layer *root_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(root_layer);

    s_sidebar_layer = sidebar_layer_create(GRect(bounds.size.w - ACTION_BAR_WIDTH, 0, ACTION_BAR_WIDTH, bounds.size.h));
#ifdef PBL_RECT
    layer_add_child(root_layer, s_sidebar_layer);
#endif

#ifdef PBL_RECT
    s_time_layer = time_layer_create(GRect(0, 12, bounds.size.w - ACTION_BAR_WIDTH, bounds.size.h - 24));
#else
    s_time_layer = time_layer_create(GRect(ACTION_BAR_WIDTH + 4, 24,  bounds.size.w - (ACTION_BAR_WIDTH * 2), bounds.size.h - 48));
#endif
    layer_add_child(root_layer, s_time_layer);

    prv_settings_handler(NULL);
    s_settings_event_handle = enamel_settings_received_subscribe(prv_settings_handler, NULL);

    logd("%d / %d", heap_bytes_used(), heap_bytes_free());
}

static void prv_window_unload(Window *window) {
    logf();
    enamel_settings_received_unsubscribe(s_settings_event_handle);
    time_layer_destroy(s_time_layer);
    sidebar_layer_destroy(s_sidebar_layer);
}

static void prv_init(void) {
    logf();
    enamel_init();
    weather_init();
    connection_vibes_init();
    hourly_vibes_init();
    uint32_t const pattern[] = { 100 };
    hourly_vibes_set_pattern((VibePattern) {
        .durations = pattern,
        .num_segments = 1
    });

#ifdef PBL_HEALTH
    connection_vibes_enable_health(true);
    hourly_vibes_enable_health(true);
#endif

    events_app_message_open();

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

    hourly_vibes_deinit();
    connection_vibes_deinit();
    weather_deinit();
    enamel_deinit();
}

int main(void) {
    prv_init();
    app_event_loop();
    prv_deinit();
}
