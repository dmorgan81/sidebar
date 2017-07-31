#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/ffont.h>
#include <enamel.h>
#include "logging.h"
#include "time-layer.h"

typedef struct {
    struct tm tick_time;
    EventHandle tick_timer_event_handle;
} Data;

typedef struct {
    uint32_t resource_id;
    int16_t x_offset;
    int16_t y_offset;
} FontData;

static FontData s_font_data[] = {
     { RESOURCE_ID_FONT_LECO, -1, 0 },
     { RESOURCE_ID_FONT_LATO, -4, -10 }
};

static void prv_update_proc(TimeLayer *this, GContext *ctx) {
    logf();
    GRect bounds = layer_get_unobstructed_bounds(this);
    GRect frame = layer_get_frame(this);
    GPoint offset = gpoint_add(frame.origin, bounds.origin);
    Data *data = layer_get_data(this);
    FontData font_data = s_font_data[atoi(enamel_get_FONT())];
    FFont *font = ffont_create_from_resource(font_data.resource_id);

    FContext fctx;
    fctx_init_context(&fctx, ctx);

    fctx_set_fill_color(&fctx, gcolor_legible_over(enamel_get_COLOR_BACKGROUND()));
    fctx_set_text_em_height(&fctx, font, (bounds.size.h * 5) / 8);

#if PBL_API_EXISTS(unobstructed_area_service_subscribe)
    GRect full_bounds = layer_get_bounds(this);
    int p = (full_bounds.size.h - bounds.size.h) *  50 / full_bounds.size.h;
    FPoint pivot = FPointI(0, -p);
    offset.y += p / 2;
    bounds.size.h -= p;
    fctx_set_pivot(&fctx, pivot);
#endif

    fctx_set_offset(&fctx, FPointI(offset.x + bounds.size.w + font_data.x_offset, offset.y + font_data.y_offset));

    char s[3];
    if (enamel_get_LEADING_ZERO()) strftime(s, sizeof(s), clock_is_24h_style() ? "%H" : "%I", &data->tick_time);
    else strftime(s, sizeof(s), clock_is_24h_style() ? "%k" : "%l", &data->tick_time);
    fctx_begin_fill(&fctx);
    fctx_draw_string(&fctx, s, font, GTextAlignmentRight, FTextAnchorTop);
    fctx_end_fill(&fctx);

    fctx_set_offset(&fctx, FPointI(offset.x + bounds.size.w + font_data.x_offset, offset.y + bounds.size.h));

    strftime(s, sizeof(s), "%M", &data->tick_time);
    fctx_begin_fill(&fctx);
    fctx_draw_string(&fctx, s, font, GTextAlignmentRight, FTextAnchorBaseline);
    fctx_end_fill(&fctx);

    fctx_deinit_context(&fctx);
    ffont_destroy(font);
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed, void *this) {
    logf();
    Data *data = layer_get_data(this);
    memcpy(&data->tick_time, tick_time, sizeof(struct tm));
    layer_mark_dirty(this);
}

TimeLayer *time_layer_create(GRect frame) {
    logf();
    TimeLayer *this = layer_create_with_data(frame, sizeof(Data));
    layer_set_update_proc(this, prv_update_proc);
    Data *data = layer_get_data(this);

    time_t now = time(NULL);
    prv_tick_handler(localtime(&now), MINUTE_UNIT, this);
    data->tick_timer_event_handle = events_tick_timer_service_subscribe_context(MINUTE_UNIT, prv_tick_handler, this);

    return this;
}

void time_layer_destroy(TimeLayer *this) {
    logf();
    Data *data = layer_get_data(this);
    events_tick_timer_service_unsubscribe(data->tick_timer_event_handle);
    layer_destroy(this);
}
