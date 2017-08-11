#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <enamel.h>
#include "logging.h"
#include "str.h"
#include "alt-time-layer.h"

#define ALT_TIME_LAYER_MARGIN_TOP 1

typedef struct {
    struct tm tick_time;
    EventHandle tick_timer_event_handle;
    EventHandle settings_event_handle;
} Data;

static void prv_update_proc(AltTimeLayer *this, GContext *ctx) {
    logf();
    GRect bounds = layer_get_bounds(this);
    Data *data = layer_get_data(this);
    GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);

    GColor stroke_color = gcolor_legible_over(enamel_get_COLOR_SIDEBAR());
    GColor text_color = gcolor_legible_over(stroke_color);

    GRect rect = GRect(0, ALT_TIME_LAYER_MARGIN_TOP, bounds.size.w, bounds.size.h);
    graphics_draw_outline_text(ctx, font, (char *) enamel_get_ALT_TIME_LABEL(), rect, stroke_color, text_color);

    font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    rect.origin.y += 13;
    char s[6];
    strftime(s, sizeof(s), clock_is_24h_style() ? "%H\n%M" : "%I\n%M", &data->tick_time);
    graphics_draw_outline_text(ctx, font, s, rect, stroke_color, text_color);
}

static int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed, void *this) {
    logf();
    Data *data = layer_get_data(this);

    memcpy(&data->tick_time, tick_time, sizeof(struct tm));
    time_t t = mktime(&data->tick_time);
    struct tm *tick = gmtime(&t);
    memcpy(&data->tick_time, tick, sizeof(struct tm));

    data->tick_time.tm_hour += enamel_get_ALT_TIME();
    if (clock_is_24h_style()) {
        data->tick_time.tm_hour = mod(data->tick_time.tm_hour, 24);
    } else {
        data->tick_time.tm_hour = mod(data->tick_time.tm_hour, 24);
        if (data->tick_time.tm_hour == 0) data->tick_time.tm_hour = 12;
    }

    layer_mark_dirty(this);
}

static void prv_settings_handler(void *this) {
    logf();
    time_t now = time(NULL);
    prv_tick_handler(localtime(&now), MINUTE_UNIT, this);
}

AltTimeLayer *alt_time_layer_create(GRect frame) {
    logf();
    AltTimeLayer *this = layer_create_with_data(frame, sizeof(Data));
    layer_set_update_proc(this, prv_update_proc);
    Data *data = layer_get_data(this);

    prv_settings_handler(this);
    data->settings_event_handle = enamel_settings_received_subscribe(prv_settings_handler, this);
    data->tick_timer_event_handle = events_tick_timer_service_subscribe_context(MINUTE_UNIT, prv_tick_handler, this);

    return this;
}

void alt_time_layer_destroy(AltTimeLayer *this) {
    logf();
    Data *data = layer_get_data(this);
    events_tick_timer_service_unsubscribe(data->tick_timer_event_handle);
    enamel_settings_received_unsubscribe(data->settings_event_handle);
    layer_destroy(this);
}
