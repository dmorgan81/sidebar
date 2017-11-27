#ifdef PBL_HEALTH
#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <pdc-transform/pdc-transform.h>
#include <enamel.h>
#include "logging.h"
#include "str.h"
#include "health.h"
#include "active-time-layer.h"

typedef struct {
    HealthValue active_seconds;
    EventHandle health_event_handle;
} Data;

static void prv_update_proc(ActiveTimeLayer *this, GContext *ctx) {
    logf();
    GRect bounds = layer_get_bounds(this);
    Data *data = layer_get_data(this);
    GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);

    GColor stroke_color = gcolor_legible_over(enamel_get_COLOR_SIDEBAR());
    GColor text_color = gcolor_legible_over(stroke_color);

    GDrawCommandImage *pdc = gdraw_command_image_create_with_resource(RESOURCE_ID_PDC_ACTIVE_TIME);
    if (!gcolor_equal(stroke_color, GColorBlack)) {
        pdc_transform_recolor_image(pdc, GColorBlack, GColorWhite);
    }

    gdraw_command_image_draw(ctx, pdc, GPoint(PBL_IF_RECT_ELSE(2, 6), 0));
    gdraw_command_image_destroy(pdc);

    HealthValue active_seconds = data->active_seconds;
    uint minutes = active_seconds / 60;
    uint hours = minutes / 60;
    minutes %= 60;

    struct tm t = {
        .tm_hour = hours,
        .tm_min = minutes
    };
    char s[8];
    strftime(s, sizeof(s), "%H:%M", &t);

    GRect rect = GRect(0, 25, bounds.size.w, bounds.size.h);
    graphics_draw_outline_text(ctx, font, (s + ((s[0] == '0') ? 1 : 0)), rect, stroke_color, text_color);
}

static void prv_health_event_handler(HealthEventType event, void *this) {
    logf();
    if (event == HealthEventSignificantUpdate || event == HealthEventMovementUpdate) {
        Data *data = layer_get_data(this);
        data->active_seconds = health_get_value_today(HealthMetricActiveSeconds);
#ifdef DEMO
        data->active_seconds = 5400;
#endif
        layer_mark_dirty(this);
    }
}

ActiveTimeLayer *active_time_layer_create(void) {
    logf();
    ActiveTimeLayer *this = layer_create_with_data(GRect(0, 0, ACTION_BAR_WIDTH, 41), sizeof(Data));
    layer_set_update_proc(this, prv_update_proc);
    Data *data = layer_get_data(this);

    prv_health_event_handler(HealthEventSignificantUpdate, this);
    data->health_event_handle = events_health_service_events_subscribe(prv_health_event_handler, this);

    return this;
}

void active_time_layer_destroy(ActiveTimeLayer *this) {
    logf();
    Data *data = layer_get_data(this);
    events_health_service_events_unsubscribe(data->health_event_handle);
    layer_destroy(this);
}
#endif // PBL_HEALTH
