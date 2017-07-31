#include <pebble.h>
#include <enamel.h>
#include "logging.h"
#include "sidebar-layer.h"
#include "date-layer.h"
#include "status-layer.h"
#include "battery-layer.h"
#include "seconds-layer.h"

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
} WidgetType;

typedef struct {
    WidgetType type;
    Layer *layer;
} Widget;

static const char* (*s_widget_settings[])() = {
    enamel_get_WIDGET_0,
    enamel_get_WIDGET_1,
    enamel_get_WIDGET_2
};

typedef struct {
    Widget *widgets[3];
    EventHandle settings_event_handle;
} Data;

static void prv_update_proc(SidebarLayer *this, GContext *ctx) {
    logf();
    graphics_context_set_fill_color(ctx, enamel_get_COLOR_SIDEBAR());
    graphics_fill_rect(ctx, layer_get_bounds(this), 0, GCornerNone);
}

static Widget *prv_widget_create(WidgetType type) {
    logf();
    Widget *this = malloc(sizeof(Widget));
    this->type = type;
    switch (type) {
        case WidgetTypeDate: this->layer = date_layer_create(WIDGET_RECT); break;
        case WidgetTypeStatus: this->layer = status_layer_create(WIDGET_RECT); break;
        case WidgetTypeBattery: this->layer = battery_layer_create(WIDGET_RECT); break;
        case WidgetTypeSeconds: this->layer = seconds_layer_create(WIDGET_RECT); break;
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
        case WidgetTypeSeconds: seconds_layer_destroy(this->layer); break;
        default: break;
    }
    free(this);
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

    return this;
}

void sidebar_layer_destroy(SidebarLayer *this) {
    logf();
    Data *data = layer_get_data(this);
    enamel_settings_received_unsubscribe(data->settings_event_handle);
    for (uint i = 0; i < ARRAY_LENGTH(data->widgets); i++) {
        prv_widget_destroy(data->widgets[i]);
    }
    layer_destroy(this);
}
