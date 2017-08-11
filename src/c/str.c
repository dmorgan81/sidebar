#include <pebble.h>
#include "logging.h"

static inline void prv_draw_outline_text(GContext *ctx, GFont font, char *s, GRect bounds, GPoint offset) {
    logf();
    graphics_draw_text(ctx, s, font, GRect(bounds.origin.x + offset.x, bounds.origin.y + offset.y, bounds.size.w, bounds.size.h),
        GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

void graphics_draw_outline_text(GContext *ctx, GFont font, char *s, GRect bounds, GColor outline_color, GColor text_color) {
    logf();
    graphics_context_set_text_color((ctx), (outline_color));
    prv_draw_outline_text((ctx), (font), (s), (bounds), GPoint(2, 0));
    prv_draw_outline_text((ctx), (font), (s), (bounds), GPoint(-2, 0));
    prv_draw_outline_text((ctx), (font), (s), (bounds), GPoint(0, 2));
    prv_draw_outline_text((ctx), (font), (s), (bounds), GPoint(0, -2));
    prv_draw_outline_text((ctx), (font), (s), (bounds), GPoint(1, 1));
    prv_draw_outline_text((ctx), (font), (s), (bounds), GPoint(-1, -1));
    prv_draw_outline_text((ctx), (font), (s), (bounds), GPoint(1, -1));
    prv_draw_outline_text((ctx), (font), (s), (bounds), GPoint(-1, 1));
    graphics_context_set_text_color((ctx), (text_color));
    prv_draw_outline_text((ctx), (font), (s), (bounds), GPoint(0, 0));
}
