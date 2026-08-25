#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

enum
{
    UI_PIECE_PX = 24,
};

/** Horizontal span: fill [x, x+w) at row y with color. */
typedef void (*ui_piece_span_fn)(int x, int y, int w, uint16_t color,
                                 void *user);

/**
 * Draw piece type 1..6 (pawn..king): outline then fill, as coalesced spans.
 * white is reserved (colors come from fill/outline args).
 */
void ui_piece_draw(int type, bool white, int x, int y, uint16_t fill,
                   uint16_t outline, ui_piece_span_fn span, void *user);

/** Non-zero pixel count of the base silhouette (no outline). */
int ui_piece_fill_count(int type);

#ifdef __cplusplus
}
#endif
