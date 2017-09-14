#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <pdc-transform/pdc-transform.h>
#include <enamel.h>
#include "logging.h"
#include "str.h"
#include "date-layer.h"

typedef struct {
    struct tm tick_time;
    EventHandle tick_timer_event_handle;
} Data;

static void prv_update_proc(DateLayer *this, GContext *ctx) {
    logf();
    GRect bounds = layer_get_bounds(this);
    Data *data = layer_get_data(this);
    GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);

    GColor stroke_color = gcolor_legible_over(enamel_get_COLOR_SIDEBAR());
    GColor text_color = gcolor_legible_over(stroke_color);

    char s[4];
    strftime(s, sizeof(s), "%b", &data->tick_time);
    strupp(s);
    GRect rect = GRect(0, -2, bounds.size.w, bounds.size.h);
    graphics_draw_outline_text(ctx, font, s, rect, stroke_color, text_color);

    GDrawCommandImage *pdc = gdraw_command_image_create_with_resource(RESOURCE_ID_PDC_CALENDAR);
    if (!gcolor_equal(stroke_color, GColorBlack)) {
        pdc_transform_recolor_image(pdc, GColorBlack, GColorWhite);
    }

    gdraw_command_image_draw(ctx, pdc, GPoint(2, 14));
    gdraw_command_image_destroy(pdc);

    snprintf(s, sizeof(s), "%d", data->tick_time.tm_mday);
    graphics_context_set_text_color(ctx, stroke_color);
    graphics_draw_text(ctx, s, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
        GRect(0, 18, bounds.size.w, bounds.size.h), GTextOverflowModeFill, GTextAlignmentCenter, NULL);

    strftime(s, sizeof(s), "%a", &data->tick_time);
    strupp(s);
    rect = GRect(0, 40, bounds.size.w, bounds.size.h);
    graphics_draw_outline_text(ctx, font, s, rect, stroke_color, text_color);
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed, void *this) {
    logf();
    Data *data = layer_get_data(this);
    memcpy(&data->tick_time, tick_time, sizeof(struct tm));
    layer_mark_dirty(this);
}

DateLayer *date_layer_create(void) {
    logf();
    DateLayer *this = layer_create_with_data(GRect(0, 0, ACTION_BAR_WIDTH, 56), sizeof(Data));
    layer_set_update_proc(this, prv_update_proc);
    Data *data = layer_get_data(this);

    time_t now = time(NULL);
    prv_tick_handler(localtime(&now), MINUTE_UNIT, this);
    data->tick_timer_event_handle = events_tick_timer_service_subscribe_context(MINUTE_UNIT, prv_tick_handler, this);

    return this;
}

void date_layer_destroy(DateLayer *this) {
    logf();
    Data *data = layer_get_data(this);
    events_tick_timer_service_unsubscribe(data->tick_timer_event_handle);
    layer_destroy(this);
}
