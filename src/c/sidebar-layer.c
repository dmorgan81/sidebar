#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <enamel.h>
#include "logging.h"
#include "sidebar-layer.h"
#include "date-layer.h"
#include "status-layer.h"
#include "battery-layer.h"
#include "seconds-layer.h"
#include "steps-layer.h"
#include "heart-rate-layer.h"
#include "distance-layer.h"
#include "active-time-layer.h"
#include "weather-layer.h"
#include "alt-time-layer.h"

#define WIDGET_EDGE_MARGIN 5

typedef enum {
    WidgetTypeNone = 0,
    WidgetTypeDate,
    WidgetTypeStatus,
    WidgetTypeBattery,
    WidgetTypeSeconds,
    WidgetTypeWeather,
    WidgetTypeAltTime,
#ifdef PBL_HEALTH
    WidgetTypeSteps,
    WidgetTypeDistance,
    WidgetTypeHeartRate,
    WidgetTypeActiveTime,
#endif
} WidgetType;

typedef struct {
    WidgetType type;
    Layer *layer;
} Widget;

static const char* (* const s_widget_settings[])() = {
    enamel_get_WIDGET_0,
    enamel_get_WIDGET_1,
#ifdef PBL_RECT
    enamel_get_WIDGET_2
#endif
};

static Layer *none_layer_create(void) {
    return layer_create(GRectZero);
}

#define none_layer_destroy layer_destroy

static Layer* (* const s_widget_create_funcs[])(void) = {
    none_layer_create,
    date_layer_create,
    status_layer_create,
    battery_layer_create,
    seconds_layer_create,
    weather_layer_create,
    alt_time_layer_create,
#ifdef PBL_HEALTH
    steps_layer_create,
    distance_layer_create,
    heart_rate_layer_create,
    active_time_layer_create,
#endif
};

static void (* const s_widget_destroy_funcs[])(Layer *) = {
    none_layer_destroy,
    date_layer_destroy,
    status_layer_destroy,
    battery_layer_destroy,
    seconds_layer_destroy,
    weather_layer_destroy,
    alt_time_layer_destroy,
#ifdef PBL_HEALTH
    steps_layer_destroy,
    distance_layer_destroy,
    heart_rate_layer_destroy,
    active_time_layer_destroy,
#endif
};

typedef struct {
    Widget *widgets[PBL_IF_RECT_ELSE(3, 2)];
    StatusLayer *status_layer;
    bool connected;
    EventHandle settings_event_handle;
    EventHandle connection_event_handle;
} Data;

static void prv_update_proc(SidebarLayer *this, GContext *ctx) {
    logf();
    GRect bounds = layer_get_unobstructed_bounds(this);
    Data *data = layer_get_data(this);

    graphics_context_set_fill_color(ctx, enamel_get_COLOR_SIDEBAR());

#ifdef PBL_RECT
    graphics_fill_rect(ctx, layer_get_bounds(this), 0, GCornerNone);

    Widget *widget = data->widgets[0];
    GRect rect = layer_get_bounds(widget->layer);
    uint8_t h1 = rect.size.h;
    uint8_t y1 = rect.origin.y;

    widget = data->widgets[2];
    rect = layer_get_bounds(widget->layer);
    uint8_t h3 = rect.size.h;

    widget = data->widgets[1];
    bool show_status_layer = quiet_time_is_active() || !data->connected;
    Layer *middle_layer;
    if (data->status_layer != NULL) {
        layer_set_hidden(data->status_layer, !show_status_layer);
        layer_set_hidden(widget->layer, show_status_layer);
        middle_layer = show_status_layer ? data->status_layer : widget->layer;
    } else {
        middle_layer = widget->layer;
    }
    rect = layer_get_bounds(middle_layer);
    uint8_t h2 = rect.size.h;

    if (h1 + h2 + h3 > bounds.size.h - (WIDGET_EDGE_MARGIN * 2)) {
        layer_set_hidden(middle_layer, true);
        h2 = 0;
    } else {
        layer_set_hidden(middle_layer, false);
    }

    uint8_t y3 = bounds.size.h - WIDGET_EDGE_MARGIN;
    if (h3 != 0) {
        widget = data->widgets[2];
        rect = layer_get_frame(widget->layer);
        y3 = rect.origin.y = bounds.size.h - rect.size.h - WIDGET_EDGE_MARGIN;
        layer_set_frame(widget->layer, rect);
    }

    if (h2 != 0) {
        rect = layer_get_frame(middle_layer);
        rect.origin.y = ((y3 - rect.size.h) + (y1 + h1)) / 2;
        layer_set_frame(middle_layer, rect);
    }
#else
    uint8_t radius = bounds.size.h / 2;
    graphics_fill_circle(ctx, GPoint(-radius / 4, radius), radius);
    graphics_fill_circle(ctx, GPoint(bounds.size.w + (radius / 4), radius), radius);

    Widget *widget = data->widgets[0];
    bool show_status_layer = quiet_time_is_active() || !data->connected;
    Layer *left_layer;
    if (data->status_layer != NULL) {
        layer_set_hidden(data->status_layer, !show_status_layer);
        layer_set_hidden(widget->layer, show_status_layer);
        left_layer = show_status_layer ? data->status_layer : widget->layer;
    } else {
        left_layer = widget->layer;
    }
    GRect rect = layer_get_bounds(left_layer);
    uint8_t h = rect.size.h;
    if (h != 0) {
        rect = layer_get_frame(left_layer);
        rect.origin.x = 0;
        rect.origin.y = radius - (h / 2);
        layer_set_frame(left_layer, rect);
    }

    widget = data->widgets[1];
    rect = layer_get_bounds(widget->layer);
    h = rect.size.h;
    if (h != 0) {
        rect = layer_get_frame(widget->layer);
        rect.origin.x = bounds.size.w - radius + (radius / 4);
        rect.origin.y = radius - (h / 2);
        layer_set_frame(widget->layer, rect);
    }
#endif
}

static Widget *prv_widget_create(WidgetType type) {
    logf();
    Widget *this = malloc(sizeof(Widget));
    this->type = type;
    Layer* (*create_func)(void) = s_widget_create_funcs[type];
    this->layer = create_func();
    return this;
}

static void prv_widget_destroy(Widget *this) {
    logf();
    void (*destroy_func)(Layer *) = s_widget_destroy_funcs[this->type];
    destroy_func(this->layer);
    free(this);
}

static void prv_connection_handler(bool connected, void *this) {
    logf();
    Data *data = layer_get_data(this);
    data->connected = connected;
    layer_mark_dirty(this);
}

static void prv_settings_handler(void *this) {
    logf();
    Data *data = layer_get_data(this);

    for (uint i = 0; i < ARRAY_LENGTH(data->widgets); i++) {
        Widget *widget = data->widgets[i];
        WidgetType type = atoi(s_widget_settings[i]());
        if (type != widget->type) {
            layer_remove_from_parent(widget->layer);
            prv_widget_destroy(widget);

            widget = prv_widget_create(type);
            data->widgets[i] = widget;

            layer_add_child(this, widget->layer);
        }
    }

    Widget *widget = data->widgets[0];
    GRect frame = layer_get_frame(widget->layer);
    frame.origin.y = WIDGET_EDGE_MARGIN;
    layer_set_frame(widget->layer, frame);

    WidgetType top_type = atoi(enamel_get_WIDGET_0());
    WidgetType mid_type = atoi(enamel_get_WIDGET_1());
    WidgetType bot_type = PBL_IF_RECT_ELSE(atoi(enamel_get_WIDGET_2()), WidgetTypeNone);
    bool have_status = (top_type == WidgetTypeStatus || mid_type == WidgetTypeStatus || bot_type == WidgetTypeStatus);

    if (have_status && data->status_layer != NULL) {
        layer_remove_from_parent(data->status_layer);
        status_layer_destroy(data->status_layer);
        data->status_layer = NULL;
        Widget *widget = data->widgets[1];
        layer_set_hidden(widget->layer, false);
    } else if (!have_status && data->status_layer == NULL) {
        data->status_layer = status_layer_create();
        layer_add_child(this, data->status_layer);
        prv_connection_handler(connection_service_peek_pebble_app_connection(), this);
    }

    layer_mark_dirty(this);
}

SidebarLayer *sidebar_layer_create(GRect frame) {
    logf();
    SidebarLayer *this = layer_create_with_data(frame, sizeof(Data));
    layer_set_update_proc(this, prv_update_proc);
    Data *data = layer_get_data(this);

    for (uint i = 0; i < ARRAY_LENGTH(data->widgets); i++) {
        data->widgets[i] = prv_widget_create(WidgetTypeNone);
    }

    prv_settings_handler(this);
    data->settings_event_handle = enamel_settings_received_subscribe(prv_settings_handler, this);
    data->connection_event_handle = events_connection_service_subscribe_context((EventConnectionHandlers) {
        .pebble_app_connection_handler = prv_connection_handler
    }, this);

    return this;
}

void sidebar_layer_destroy(SidebarLayer *this) {
    logf();
    Data *data = layer_get_data(this);
    events_connection_service_unsubscribe(data->connection_event_handle);
    enamel_settings_received_unsubscribe(data->settings_event_handle);
    if (data->status_layer != NULL) status_layer_destroy(data->status_layer);
    for (uint i = 0; i < ARRAY_LENGTH(data->widgets); i++) {
        prv_widget_destroy(data->widgets[i]);
    }
    layer_destroy(this);
}
