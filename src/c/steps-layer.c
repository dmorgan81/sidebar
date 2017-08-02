#ifdef PBL_HEALTH
#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <pdc-transform/pdc-transform.h>
#include <enamel.h>
#include "logging.h"
#include "str.h"
#include "steps-layer.h"

#define STEPS_LAYER_MARGIN_TOP 12

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

    gdraw_command_image_draw(ctx, pdc, GPoint(2, STEPS_LAYER_MARGIN_TOP));
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
    GRect rect = GRect(0, STEPS_LAYER_MARGIN_TOP + 17, bounds.size.w, bounds.size.h);
    OUTLINE_TEXT(ctx, font, s, rect, stroke_color, text_color);
}

static void prv_health_event_handler(HealthEventType event, void *this) {
    logf();
    if (event == HealthEventSignificantUpdate || event == HealthEventMovementUpdate) {
        Data *data = layer_get_data(this);
        time_t start = time_start_of_today();
        time_t end = time(NULL);
        HealthServiceAccessibilityMask mask = health_service_metric_accessible(HealthMetricStepCount, start, end);
        if (mask & HealthServiceAccessibilityMaskAvailable) {
            data->steps = health_service_sum_today(HealthMetricStepCount);
        }
        layer_mark_dirty(this);
    }
}

StepsLayer *steps_layer_create(GRect frame) {
    logf();
    StepsLayer *this = layer_create_with_data(frame, sizeof(Data));
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
