#include <pebble.h>
#include <ctype.h>

inline void strupp(char *s) {
    while ((*s++ = (char) toupper((int) *s)));
}

inline void graphics_draw_outline_text(GContext *ctx, GFont font, char *s, GRect bounds, GPoint offset) {
    graphics_draw_text(ctx, s, font, GRect(bounds.origin.x + offset.x, bounds.origin.y + offset.y, bounds.size.w, bounds.size.h),
        GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

#define OUTLINE_TEXT(ctx, font, s, bounds, outline_color, text_color) \
    graphics_context_set_text_color((ctx), (outline_color)); \
    graphics_draw_outline_text((ctx), (font), (s), (bounds), GPoint(2, 0)); \
    graphics_draw_outline_text((ctx), (font), (s), (bounds), GPoint(-2, 0)); \
    graphics_draw_outline_text((ctx), (font), (s), (bounds), GPoint(0, 2)); \
    graphics_draw_outline_text((ctx), (font), (s), (bounds), GPoint(0, -2)); \
    graphics_draw_outline_text((ctx), (font), (s), (bounds), GPoint(1, 1)); \
    graphics_draw_outline_text((ctx), (font), (s), (bounds), GPoint(-1, -1)); \
    graphics_draw_outline_text((ctx), (font), (s), (bounds), GPoint(1, -1)); \
    graphics_draw_outline_text((ctx), (font), (s), (bounds), GPoint(-1, 1)); \
    graphics_context_set_text_color((ctx), (text_color)); \
    graphics_draw_outline_text((ctx), (font), (s), (bounds), GPoint(0, 0))
