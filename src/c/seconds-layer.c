#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <enamel.h>
#include "logging.h"
#include "str.h"
#include "seconds-layer.h"

typedef struct {
    uint8_t tm_sec;
    EventHandle tick_timer_event_handle;
} Data;

static void prv_update_proc(SecondsLayer *this, GContext *ctx) {
    logf();
    GRect bounds = layer_get_bounds(this);
    Data *data = layer_get_data(this);
    GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);

    GColor stroke_color = gcolor_legible_over(enamel_get_COLOR_SIDEBAR());
    GColor text_color = gcolor_legible_over(stroke_color);

    char s[4];
    snprintf(s, sizeof(s), ":%02d", data->tm_sec);
    graphics_draw_outline_text(ctx, font, s, bounds, stroke_color, text_color);
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed, void *this) {
    logf();
    Data *data = layer_get_data(this);
    data->tm_sec = tick_time->tm_sec;
#ifdef DEMO
    data->tm_sec = 22;
#endif
    layer_mark_dirty(this);
}

SecondsLayer *seconds_layer_create(void) {
    logf();
    SecondsLayer *this = layer_create_with_data(GRect(0, 0, ACTION_BAR_WIDTH, 25), sizeof(Data));
    layer_set_update_proc(this, prv_update_proc);
    Data *data = layer_get_data(this);

    time_t now = time(NULL);
    prv_tick_handler(localtime(&now), SECOND_UNIT, this);
    data->tick_timer_event_handle = events_tick_timer_service_subscribe_context(SECOND_UNIT, prv_tick_handler, this);

    return this;
}

void seconds_layer_destroy(SecondsLayer *this) {
    logf();
    Data *data = layer_get_data(this);
    events_tick_timer_service_unsubscribe(data->tick_timer_event_handle);
    layer_destroy(this);
}
