#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

enum
{
    UI_FONT_W = 5,
    UI_FONT_H = 7,
};

/** 5×7 glyph rows (MSB = leftmost). NULL if unknown → treat as space. */
const uint8_t *ui_font_glyph(char c);

/** Pixel width of string at given scale (gap = scale between glyphs). */
int ui_font_text_width(const char *s, int scale);

/**
 * Draw text by calling plot(x,y,user) for each foreground pixel.
 * scale >= 1; baseline is top-left of the first glyph.
 */
typedef void (*ui_font_plot_fn)(int x, int y, void *user);
void ui_font_draw(const char *s, int x, int y, int scale, ui_font_plot_fn plot,
                  void *user);

/**
 * Draw text as coalesced horizontal spans (after scale).
 * span(x, y, w, user) fills [x, x+w) at row y — prefer this for LCD.
 */
typedef void (*ui_font_span_fn)(int x, int y, int w, void *user);
void ui_font_draw_spans(const char *s, int x, int y, int scale,
                        ui_font_span_fn span, void *user);

#ifdef __cplusplus
}
#endif
