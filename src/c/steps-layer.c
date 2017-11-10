#ifdef PBL_HEALTH
#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <pdc-transform/pdc-transform.h>
#include <enamel.h>
#include "logging.h"
#include "str.h"
#include "health.h"
#include "steps-layer.h"

typedef struct {
    HealthValue steps;
    EventHandle health_event_handle;
} Data;

static void prv_update_proc(StepsLayer *this, GContext *ctx) {
    logf();
    GRect bounds = layer_get_bounds(this);
    Data *data = layer_get_data(this);
    GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);

    GColor stroke_color = gcolor_legible_over(enamel_get_COLOR_SIDEBAR());
    GColor text_color = gcolor_legible_over(stroke_color);

    GDrawCommandImage *pdc = gdraw_command_image_create_with_resource(RESOURCE_ID_PDC_STEPS);
    if (!gcolor_equal(stroke_color, GColorBlack)) {
        pdc_transform_recolor_image(pdc, GColorBlack, GColorWhite);
    }

    gdraw_command_image_draw(ctx, pdc, GPoint(2, 0));
    gdraw_command_image_destroy(pdc);

    HealthValue steps = data->steps;
    char s[8];
    if (steps < 1000) {
        snprintf(s, sizeof(s), "%ld", steps);
    } else if (steps < 10000) {
        snprintf(s, sizeof(s), "%ld.%ldk", steps / 1000, steps / 100 % 10);
    } else {
        snprintf(s, sizeof(s), "%ldk", steps / 1000);
    }
    GRect rect = GRect(0, 17, bounds.size.w, bounds.size.h);
    graphics_draw_outline_text(ctx, font, s, rect, stroke_color, text_color);
}

static void prv_health_event_handler(HealthEventType event, void *this) {
    logf();
    if (event == HealthEventSignificantUpdate || event == HealthEventMovementUpdate) {
        Data *data = layer_get_data(this);
        data->steps = health_get_value_today(HealthMetricStepCount);
        layer_mark_dirty(this);
    }
}

StepsLayer *steps_layer_create(void) {
    logf();
    StepsLayer *this = layer_create_with_data(GRect(0, 0, ACTION_BAR_WIDTH, 33), sizeof(Data));
    layer_set_update_proc(this, prv_update_proc);
    Data *data = layer_get_data(this);

    prv_health_event_handler(HealthEventSignificantUpdate, this);
    data->health_event_handle = events_health_service_events_subscribe(prv_health_event_handler, this);

    return this;
}

void steps_layer_destroy(StepsLayer *this) {
    logf();
    Data *data = layer_get_data(this);
    events_health_service_events_unsubscribe(data->health_event_handle);
    layer_destroy(this);
}
#endif // PBL_HEALTH
