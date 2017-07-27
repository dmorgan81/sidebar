#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <pebble-connection-vibes/connection-vibes.h>
#include <pebble-hourly-vibes/hourly-vibes.h>
#include <enamel.h>
#include "logging.h"
#include "time-layer.h"
#include "date-layer.h"
#include "status-layer.h"
#include "battery-layer.h"

#define WIDGET_WIDTH ACTION_BAR_WIDTH
#define WIDGET_HEIGHT 56

typedef enum {
    WidgetTypeNone = 0,
    WidgetTypeDate,
    WidgetTypeStatus,
    WidgetTypeBattery,
} WidgetType;

typedef struct {
    WidgetType type;
    Layer *layer;
} Widget;

static Widget *s_widgets[3];
static const char* (*s_widget_settings[])() = {
    enamel_get_WIDGET_0,
    enamel_get_WIDGET_1,
    enamel_get_WIDGET_2
};

static Window *s_window;
static TimeLayer *s_time_layer;

static EventHandle s_settings_event_handle;

static Widget *prv_widget_create(WidgetType type) {
    logf();
    Widget *this = malloc(sizeof(Widget));
    this->type = type;
    switch (type) {
        case WidgetTypeDate: this->layer = date_layer_create(GRect(0, 0, WIDGET_WIDTH, WIDGET_HEIGHT)); break;
        case WidgetTypeStatus: this->layer = status_layer_create(GRect(0, 0, WIDGET_WIDTH, WIDGET_HEIGHT)); break;
        case WidgetTypeBattery: this->layer = battery_layer_create(GRect(0, 0, WIDGET_WIDTH, WIDGET_HEIGHT)); break;
        default: this->layer = NULL; break;
    }
    return this;
}

static void prv_widget_destroy(Widget *this) {
    logf();
    switch(this->type) {
        case WidgetTypeDate: date_layer_destroy(this->layer); break;
        case WidgetTypeStatus: status_layer_destroy(this->layer); break;
        case WidgetTypeBattery: battery_layer_destroy(this->layer); break;
        default: break;
    }
    free(this);
}

static void prv_settings_handler(void *context) {
    logf();
    connection_vibes_set_state(atoi(enamel_get_CONNECTION_VIBE()));
    hourly_vibes_set_enabled(enamel_get_HOURLY_VIBE());

    GRect frame = layer_get_frame(s_time_layer);
    frame.origin.x = (enamel_get_RIGHT_BAR() ? 0 : WIDGET_WIDTH) - 1;
    layer_set_frame(s_time_layer, frame);

    Layer *root_layer = window_get_root_layer(s_window);
    GRect bounds = layer_get_bounds(root_layer);
    for (uint i = 0; i < ARRAY_LENGTH(s_widgets); i++) {
        Widget *widget = s_widgets[i];
        WidgetType type = atoi(s_widget_settings[i]());
        if (type != widget->type) {
            if (widget->type != WidgetTypeNone) layer_remove_from_parent(widget->layer);
            prv_widget_destroy(widget);

            widget = prv_widget_create(type);
            s_widgets[i] = widget;

            if (widget->type == WidgetTypeNone) continue;

            layer_add_child(root_layer, widget->layer);
        }

        if (widget->type == WidgetTypeNone) continue;

        Layer *layer = widget->layer;
        GRect frame = layer_get_frame(layer);
        frame.origin.x = enamel_get_RIGHT_BAR() ? bounds.size.w - WIDGET_WIDTH : 0;
        frame.origin.y = i * WIDGET_HEIGHT;
        layer_set_frame(layer, frame);
    }

    layer_mark_dirty(window_get_root_layer(s_window));
}

static void prv_update_proc(Layer *this, GContext *ctx) {
    logf();
    GRect bounds = layer_get_bounds(this);

    graphics_context_set_fill_color(ctx, enamel_get_COLOR_BACKGROUND());
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    graphics_context_set_fill_color(ctx, enamel_get_COLOR_SIDEBAR());
    graphics_fill_rect(ctx, GRect((enamel_get_RIGHT_BAR() ? bounds.size.w - ACTION_BAR_WIDTH : 0), 0, ACTION_BAR_WIDTH, bounds.size.h), 0, GCornerNone);
}

static void prv_window_load(Window *window) {
    logf();
    Layer *root_layer = window_get_root_layer(window);
    layer_set_update_proc(root_layer, prv_update_proc);
    GRect bounds = layer_get_bounds(root_layer);

    s_time_layer = time_layer_create(GRect(-1, 12, bounds.size.w - ACTION_BAR_WIDTH, bounds.size.h - 24));
    layer_add_child(root_layer, s_time_layer);

    for (uint i = 0; i < ARRAY_LENGTH(s_widgets); i++) {
        s_widgets[i] = prv_widget_create(WidgetTypeNone);
    }

    prv_settings_handler(NULL);
    s_settings_event_handle = enamel_settings_received_subscribe(prv_settings_handler, NULL);

    logd("%d / %d", heap_bytes_used(), heap_bytes_free());
}

static void prv_window_unload(Window *window) {
    logf();
    enamel_settings_received_unsubscribe(s_settings_event_handle);

    for (uint i = 0; i < ARRAY_LENGTH(s_widgets); i++) {
        prv_widget_destroy(s_widgets[i]);
    }

    time_layer_destroy(s_time_layer);
}

static void prv_init(void) {
    logf();
    enamel_init();
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
    enamel_deinit();
}

int main(void) {
    prv_init();
    app_event_loop();
    prv_deinit();
}
