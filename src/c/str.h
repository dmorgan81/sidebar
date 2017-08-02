#include <pebble.h>
#include <ctype.h>

inline void strupp(char *s) {
    while ((*s++ = (char) toupper((int) *s)));
}

void graphics_draw_outline_text(GContext *ctx, GFont font, char *s, GRect bounds, GColor outline_color, GColor text_color);
