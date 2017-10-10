#ifdef PBL_HEALTH
#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <pdc-transform/pdc-transform.h>
#include <enamel.h>
#include "logging.h"
#include "str.h"
#include "distance-layer.h"

typedef struct {
    HealthValue distance;
    EventHandle health_event_handle;
} Data;

static void prv_update_proc(DistanceLayer *this, GContext *ctx) {
    logf();
    GRect bounds = layer_get_bounds(this);
    Data *data = layer_get_data(this);
    GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);

    GColor stroke_color = gcolor_legible_over(enamel_get_COLOR_SIDEBAR());
    GColor text_color = gcolor_legible_over(stroke_color);

    GDrawCommandImage *pdc = gdraw_command_image_create_with_resource(RESOURCE_ID_PDC_DISTANCE);
    if (!gcolor_equal(stroke_color, GColorBlack)) {
        pdc_transform_recolor_image(pdc, GColorBlack, GColorWhite);
    }

    gdraw_command_image_draw(ctx, pdc, GPoint(2, 0));
    gdraw_command_image_destroy(pdc);

    HealthValue distance = data->distance;
    char s[8];
    MeasurementSystem system = health_service_get_measurement_system_for_display(HealthMetricWalkedDistanceMeters);
    if (system != MeasurementSystemImperial) {
        if (distance < 100) {
            snprintf(s, sizeof(s), "%ldm", distance);
        } else if (distance < 1000) {
            distance /= 100;
            snprintf(s, sizeof(s), ".%ldkm", distance);
        } else {
            distance /= 1000;
            snprintf(s, sizeof(s), "%ldkm", distance);
        }
    } else {
        int tenths = distance * 10 / 1609 % 10;
        int whole = distance / 1609;
        if (whole < 10) {
            snprintf(s, sizeof(s), "%d.%dmi", whole, tenths);
        } else {
            snprintf(s, sizeof(s), "%dmi", whole);
        }
    }
    GRect rect = GRect(0, 17, bounds.size.w, bounds.size.h);
    graphics_draw_outline_text(ctx, font, s, rect, stroke_color, text_color);
}

static void prv_health_event_handler(HealthEventType event, void *this) {
    logf();
    if (event == HealthEventSignificantUpdate || event == HealthEventMovementUpdate) {
        Data *data = layer_get_data(this);
        time_t start = time_start_of_today();
        time_t end = time(NULL);
        HealthServiceAccessibilityMask mask = health_service_metric_accessible(HealthMetricWalkedDistanceMeters, start, end);
        if (mask & HealthServiceAccessibilityMaskAvailable) {
            data->distance = health_service_sum_today(HealthMetricWalkedDistanceMeters);
        }
        layer_mark_dirty(this);
    }
}

DistanceLayer *distance_layer_create(void) {
    logf();
    DistanceLayer *this = layer_create_with_data(GRect(0, 0, ACTION_BAR_WIDTH, 32), sizeof(Data));
    layer_set_update_proc(this, prv_update_proc);
    Data *data = layer_get_data(this);

    prv_health_event_handler(HealthEventSignificantUpdate, this);
    data->health_event_handle = events_health_service_events_subscribe(prv_health_event_handler, this);

    return this;
}

void distance_layer_destroy(DistanceLayer *this) {
    logf();
    Data *data = layer_get_data(this);
    events_health_service_events_unsubscribe(data->health_event_handle);
    layer_destroy(this);
}
#endif // PBL_HEALTH
