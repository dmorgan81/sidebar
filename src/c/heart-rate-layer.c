#ifdef PBL_HEALTH
#include <pebble.h>
#include <pdc-transform/pdc-transform.h>
#include <enamel.h>
#include <pebble-events/pebble-events.h>
#include "logging.h"
#include "str.h"
#include "heart-rate-layer.h"

typedef struct {
    HealthValue heart_rate;
    EventHandle health_event_handle;
} Data;

static void prv_update_proc(HeartRateLayer *this, GContext *ctx) {
    logf();
    GRect bounds = layer_get_bounds(this);
    Data *data = layer_get_data(this);
    GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);

    GColor stroke_color = gcolor_legible_over(enamel_get_COLOR_SIDEBAR());
    GColor text_color = gcolor_legible_over(stroke_color);

    GDrawCommandImage *pdc = gdraw_command_image_create_with_resource(RESOURCE_ID_PDC_HEART_RATE);
    if (!gcolor_equal(stroke_color, GColorBlack)) {
        pdc_transform_recolor_image(pdc, GColorBlack, GColorWhite);
    }

    gdraw_command_image_draw(ctx, pdc, GPoint(3, 0));
    gdraw_command_image_destroy(pdc);

    char s[4];
    if (data->health_event_handle != NULL) {
        snprintf(s, sizeof(s), "%ld", data->heart_rate);
    } else {
        snprintf(s, sizeof(s), "N/A");
    }

    GRect rect = GRect(0, 25, bounds.size.w, bounds.size.h);
    graphics_draw_outline_text(ctx, font, s, rect, stroke_color, text_color);
}

static void prv_health_event_handler(HealthEventType event, void *this) {
    logf();
    if (event == HealthEventSignificantUpdate || event == HealthEventHeartRateUpdate) {
        Data *data = layer_get_data(this);
        time_t now = time(NULL);
        HealthServiceAccessibilityMask mask = health_service_metric_accessible(HealthMetricHeartRateBPM, now, now);
        if (mask & HealthServiceAccessibilityMaskAvailable) {
            data->heart_rate = health_service_peek_current_value(HealthMetricHeartRateBPM);
        } else {
            data->heart_rate = 0;
        }
#ifdef DEMO
        data->heart_rate = 70;
#endif
    }
    layer_mark_dirty(this);
}

HeartRateLayer *heart_rate_layer_create(void) {
    logf();
    HeartRateLayer *this = layer_create_with_data(GRect(0, 0, ACTION_BAR_WIDTH, 41), sizeof(Data));
    layer_set_update_proc(this, prv_update_proc);
    Data *data = layer_get_data(this);

    WatchInfoModel model = watch_info_get_model();
#if defined(DEMO) && defined(PBL_PLATFORM_DIORITE)
    model = WATCH_INFO_MODEL_PEBBLE_2_HR;
#endif
    if (model == WATCH_INFO_MODEL_PEBBLE_2_HR) {
        prv_health_event_handler(HealthEventSignificantUpdate, this);
        data->health_event_handle = events_health_service_events_subscribe(prv_health_event_handler, this);
    } else {
        data->health_event_handle = NULL;
    }

    return this;
}

void heart_rate_layer_destroy(HeartRateLayer *this) {
    logf();
    Data *data = layer_get_data(this);
    if (data->health_event_handle != NULL) events_health_service_events_unsubscribe(data->health_event_handle);
    layer_destroy(this);
}
#endif // PBL_HEALTH
