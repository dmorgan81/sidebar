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
#include "weather-layer.h"

#define WIDGET_WIDTH ACTION_BAR_WIDTH
#define WIDGET_HEIGHT 56

static const GRect WIDGET_RECT = {
    { 0, 0 },
    { WIDGET_WIDTH, WIDGET_HEIGHT }
};

typedef enum {
    WidgetTypeNone = 0,
    WidgetTypeDate,
    WidgetTypeStatus,
    WidgetTypeBattery,
    WidgetTypeSeconds,
#ifdef PBL_HEALTH
    WidgetTypeSteps,
    WidgetTypeHeartRate,
#endif
    WidgetTypeWeather,
} WidgetType;

typedef struct {
    WidgetType type;
    Layer *layer;
} Widget;

static const char* (* const s_widget_settings[])() = {
    enamel_get_WIDGET_0,
    enamel_get_WIDGET_1,
    enamel_get_WIDGET_2
};

static Layer* (* const s_widget_create_funcs[])(GRect) = {
    NULL,
    date_layer_create,
    status_layer_create,
    battery_layer_create,
    seconds_layer_create,
#ifdef PBL_HEALTH
    steps_layer_create,
    heart_rate_layer_create,
#endif
    weather_layer_create,
};

static void (* const s_widget_destroy_funcs[])(Layer *) = {
    NULL,
    date_layer_destroy,
    status_layer_destroy,
    battery_layer_destroy,
    seconds_layer_destroy,
#ifdef PBL_HEALTH
    steps_layer_destroy,
    heart_rate_layer_destroy,
#endif
    weather_layer_destroy,
};

typedef struct {
    Widget *widgets[3];
    StatusLayer *status_layer;
    EventHandle settings_event_handle;
    EventHandle connection_event_handle;
} Data;

static void prv_update_proc(SidebarLayer *this, GContext *ctx) {
    logf();
    Data *data = layer_get_data(this);

    graphics_context_set_fill_color(ctx, enamel_get_COLOR_SIDEBAR());
    graphics_fill_rect(ctx, layer_get_bounds(this), 0, GCornerNone);

    if (data->status_layer != NULL) {
        Widget *widget = data->widgets[1];
        bool connected = connection_service_peek_pebble_app_connection();
#ifndef PBL_PLATFORM_APLITE
        bool quiet_time = quiet_time_is_active();
        layer_set_hidden(data->status_layer, !quiet_time && connected);
        if (widget->type != WidgetTypeNone) layer_set_hidden(widget->layer, quiet_time || !connected);
#else
        layer_set_hidden(data->status_layer, connected);
        if (widget->type != WidgetTypeNone) layer_set_hidden(widget->layer, !connected);
#endif
    }
}

static Widget *prv_widget_create(WidgetType type) {
    logf();
    Widget *this = malloc(sizeof(Widget));
    this->type = type;
    Layer* (*create_func)(GRect) = s_widget_create_funcs[type];
    this->layer = create_func != NULL ? create_func(WIDGET_RECT) : NULL;
    return this;
}

static void prv_widget_destroy(Widget *this) {
    logf();
    void (*destroy_func)(Layer *) = s_widget_destroy_funcs[this->type];
    if (destroy_func != NULL) destroy_func(this->layer);
    free(this);
}

static void prv_connection_handler(bool connected, void *this) {
    logf();
    Data *data = layer_get_data(this);
    if (data->status_layer != NULL) {
        layer_set_hidden(data->status_layer, connected);
        Widget *widget = data->widgets[1];
        if (widget->type != WidgetTypeNone) layer_set_hidden(widget->layer, !connected);
    }
    layer_mark_dirty(this);
}

static void prv_settings_handler(void *this) {
    logf();
    Data *data = layer_get_data(this);

    for (uint i = 0; i < ARRAY_LENGTH(data->widgets); i++) {
        Widget *widget = data->widgets[i];
        WidgetType type = atoi(s_widget_settings[i]());
        if (type != widget->type) {
            if (widget->type != WidgetTypeNone) layer_remove_from_parent(widget->layer);
            prv_widget_destroy(widget);

            widget = prv_widget_create(type);
            data->widgets[i] = widget;

            if (widget->type == WidgetTypeNone) continue;

            layer_add_child(this, widget->layer);
        }

        if (widget->type == WidgetTypeNone) continue;

        Layer *layer = widget->layer;
        GRect frame = layer_get_frame(layer);
        frame.origin.y = i * WIDGET_HEIGHT;
        layer_set_frame(layer, frame);
    }

    WidgetType top_type = atoi(enamel_get_WIDGET_0());
    WidgetType mid_type = atoi(enamel_get_WIDGET_1());
    WidgetType bot_type = atoi(enamel_get_WIDGET_2());
    bool have_status = (top_type == WidgetTypeStatus || mid_type == WidgetTypeStatus || bot_type == WidgetTypeStatus);

    if (have_status && data->status_layer != NULL) {
        layer_remove_from_parent(data->status_layer);
        status_layer_destroy(data->status_layer);
        data->status_layer = NULL;
        Widget *widget = data->widgets[1];
        if (widget->type != WidgetTypeNone) layer_set_hidden(widget->layer, false);
    } else if (!have_status && data->status_layer == NULL) {
        data->status_layer = status_layer_create(GRect(0, WIDGET_HEIGHT, WIDGET_WIDTH, WIDGET_HEIGHT));
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
