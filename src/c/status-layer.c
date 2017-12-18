#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <pdc-transform/pdc-transform.h>
#include <enamel.h>
#include "logging.h"
#include "status-layer.h"

#define STATUS_LAYER_CONN_IDX 3
#define STATUS_LAYER_NO_CONN_IDX 6
#define STATUS_LAYER_QT_IDX 8

typedef struct {
    bool connected;
    EventHandle connection_event_handle;
} Data;

static void prv_update_proc(StatusLayer *this, GContext *ctx) {
    logf();
    Data *data = layer_get_data(this);
    GDrawCommandImage *pdc = gdraw_command_image_create_with_resource(RESOURCE_ID_PDC_STATUS);
    GDrawCommandList *list = gdraw_command_image_get_command_list(pdc);
    uint32_t num_cmds = gdraw_command_list_get_num_commands(list);

    GColor stroke_color = gcolor_legible_over(enamel_get_COLOR_SIDEBAR());
    if (!gcolor_equal(stroke_color, GColorBlack)) {
        pdc_transform_recolor_image(pdc, GColorBlack, GColorWhite);
    }

    if (!data->connected) {
        for (uint i = STATUS_LAYER_CONN_IDX; i < STATUS_LAYER_NO_CONN_IDX; i++) {
            gdraw_command_set_hidden(gdraw_command_list_get_command(list, i), true);
        }

        for (uint i = STATUS_LAYER_NO_CONN_IDX; i < STATUS_LAYER_QT_IDX; i++) {
            gdraw_command_set_hidden(gdraw_command_list_get_command(list, i), false);
        }

        for (uint i = STATUS_LAYER_QT_IDX; i < num_cmds; i++) {
            gdraw_command_set_hidden(gdraw_command_list_get_command(list, i), true);
        }
    }

#ifndef PBL_PLATFORM_APLITE
    else if (quiet_time_is_active()) {
        for (uint i = STATUS_LAYER_CONN_IDX; i < STATUS_LAYER_QT_IDX; i++) {
            gdraw_command_set_hidden(gdraw_command_list_get_command(list, i), true);
        }

        for (uint i = STATUS_LAYER_QT_IDX; i < num_cmds; i++) {
            gdraw_command_set_hidden(gdraw_command_list_get_command(list, i), false);
        }
    }
#endif

    gdraw_command_image_draw(ctx, pdc, GPoint(PBL_IF_RECT_ELSE(2, 6), 0));
    gdraw_command_image_destroy(pdc);
}

static void prv_connection_handler(bool connected, void *this) {
    logf();
    Data *data = layer_get_data(this);
    data->connected = connected;
    layer_mark_dirty(this);
}

StatusLayer *status_layer_create(void) {
    logf();
    StatusLayer *this = layer_create_with_data(GRect(0, 0, ACTION_BAR_WIDTH, 27), sizeof(Data));
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
