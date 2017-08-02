#include <pebble.h>
#include <enamel.h>
#include "logging.h"
#include "str.h"
#include "weather.h"
#include "weather-layer.h"

#define WEATHER_LAYER_MARGIN_TOP 7

typedef struct {
    GenericWeatherStatus status;
    GenericWeatherInfo info;
    EventHandle weather_event_handle;
} Data;

static bool prv_cmd_list_iterator_cb(GDrawCommand *cmd, uint32_t index, void *context) {
    logf();
    gdraw_command_set_fill_color(cmd, GColorBlack);
    gdraw_command_set_stroke_color(cmd, GColorWhite);
    return true;
}

static void prv_update_proc(WeatherLayer *this, GContext *ctx) {
    logf();
    GRect bounds = layer_get_bounds(this);
    Data *data = layer_get_data(this);
    GenericWeatherInfo info = data->info;
    GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

    GColor stroke_color = gcolor_legible_over(enamel_get_COLOR_SIDEBAR());
    GColor text_color = gcolor_legible_over(stroke_color);

    uint32_t resource_id;
    switch(info.condition) {
        case GenericWeatherConditionClearSky:
            resource_id = info.day ? RESOURCE_ID_PDC_WEATHER_CLEAR_DAY : RESOURCE_ID_PDC_WEATHER_CLEAR_NIGHT;
            break;
        case GenericWeatherConditionFewClouds:
            resource_id = info.day ? RESOURCE_ID_PDC_WEATHER_PARTLY_CLOUDY_DAY : RESOURCE_ID_PDC_WEATHER_PARTLY_CLOUDY_NIGHT;
            break;
        case GenericWeatherConditionScatteredClouds:
        case GenericWeatherConditionBrokenClouds:
            resource_id = RESOURCE_ID_PDC_WEATHER_CLOUDY;
            break;
        case GenericWeatherConditionShowerRain:
        case GenericWeatherConditionRain:
        case GenericWeatherConditionThunderstorm:
            resource_id = RESOURCE_ID_PDC_WEATHER_RAIN;
            break;
        case GenericWeatherConditionSnow:
            resource_id = RESOURCE_ID_PDC_WEATHER_SNOW;
            break;
        case GenericWeatherConditionMist:
            resource_id = RESOURCE_ID_PDC_WEATHER_MIST;
            break;
        default:
            resource_id = RESOURCE_ID_PDC_WEATHER_CLEAR_DAY;
            break;
    }

    GDrawCommandImage *pdc = gdraw_command_image_create_with_resource(resource_id);
    if (!gcolor_equal(stroke_color, GColorBlack)) {
        gdraw_command_list_iterate(gdraw_command_image_get_command_list(pdc), prv_cmd_list_iterator_cb, NULL);
    }

    gdraw_command_image_draw(ctx, pdc, GPoint(3, WEATHER_LAYER_MARGIN_TOP));
    gdraw_command_image_destroy(pdc);

    char s[6];
    if (data->status == GenericWeatherStatusAvailable) {
        int unit = atoi(enamel_get_WEATHER_UNIT());
        snprintf(s, sizeof(s), "%d", unit == 1 ? info.temp_f : info.temp_c);
        if (strlen(s) < 3) strcat(s, "°");
    } else if (data->status != GenericWeatherStatusPending) {
        snprintf(s, sizeof(s), "--");
    } else {
        snprintf(s, sizeof(s), "??");
    }
    GRect rect = GRect(0, WEATHER_LAYER_MARGIN_TOP + 22, bounds.size.w, bounds.size.h);
    OUTLINE_TEXT(ctx, font, s, rect, stroke_color, text_color);
}

static void prv_weather_handler(GenericWeatherInfo *info, GenericWeatherStatus status, void *this) {
    logf();
    Data *data = layer_get_data(this);
    memcpy(&data->status, &status, sizeof(GenericWeatherStatus));
    memcpy(&data->info, info, sizeof(GenericWeatherInfo));
    layer_mark_dirty(this);
}

WeatherLayer *weather_layer_create(GRect frame) {
    logf();
    WeatherLayer *this = layer_create_with_data(frame, sizeof(Data));
    layer_set_update_proc(this, prv_update_proc);
    Data *data = layer_get_data(this);

    prv_weather_handler(weather_peek(), weather_status_peek(), this);
    data->weather_event_handle = events_weather_subscribe(prv_weather_handler, this);

    return this;
}

void weather_layer_destroy(WeatherLayer *this) {
    logf();
    Data *data = layer_get_data(this);
    events_weather_unsubscribe(data->weather_event_handle);
    layer_destroy(this);
}