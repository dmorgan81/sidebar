#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <enamel.h>
#include "logging.h"
#include "status-layer.h"

typedef struct {
    bool connected;
    EventHandle connection_event_handle;
} Data;

static bool prv_cmd_list_iterator_cb(GDrawCommand *cmd, uint32_t index, void *context) {
    logf();
    gdraw_command_set_fill_color(cmd, GColorBlack);
    gdraw_command_set_stroke_color(cmd, GColorWhite);
    return true;
}

static void prv_update_proc(StatusLayer *this, GContext *ctx) {
    logf();
    Data *data = layer_get_data(this);
    GDrawCommandImage *pdc = gdraw_command_image_create_with_resource(RESOURCE_ID_PDC_STATUS);
    GDrawCommandList *list = gdraw_command_image_get_command_list(pdc);

    GColor stroke_color = gcolor_legible_over(enamel_get_COLOR_SIDEBAR());
    if (!gcolor_equal(stroke_color, GColorBlack)) {
        gdraw_command_list_iterate(list, prv_cmd_list_iterator_cb, NULL);
    }

#ifndef PBL_PLATFORM_APLITE
    if (quiet_time_is_active()) {
        for (int i = 3; i < 6; i++) {
            gdraw_command_set_hidden(gdraw_command_list_get_command(list, i), true);
        }

        for (int i = 6; i < 9; i++) {
            gdraw_command_set_hidden(gdraw_command_list_get_command(list, i), false);
        }
    }
#endif

    if (!quiet_time_is_active() && !data->connected) {
        for (int i = 3; i < 6; i++) {
            gdraw_command_set_hidden(gdraw_command_list_get_command(list, i), true);
        }

        for (int i = 9; i < 11; i++) {
            gdraw_command_set_hidden(gdraw_command_list_get_command(list, i), false);
        }
    }

    gdraw_command_image_draw(ctx, pdc, GPoint(2, 14));
    gdraw_command_image_destroy(pdc);
}

static void prv_connection_handler(bool connected, void *this) {
    logf();
    Data *data = layer_get_data(this);
    data->connected = connected;
    layer_mark_dirty(this);
}

StatusLayer *status_layer_create(GRect frame) {
    logf();
    StatusLayer *this = layer_create_with_data(frame, sizeof(Data));
    layer_set_update_proc(this, prv_update_proc);
    Data *data = layer_get_data(this);

    data->connected = connection_service_peek_pebble_app_connection();
    data->connection_event_handle = events_connection_service_subscribe_context((EventConnectionHandlers) {
        .pebble_app_connection_handler = prv_connection_handler
    }, this);

    return this;
}

void status_layer_destroy(StatusLayer *this) {
    logf();
    Data *data = layer_get_data(this);
    events_connection_service_unsubscribe(data->connection_event_handle);
    layer_destroy(this);
}
