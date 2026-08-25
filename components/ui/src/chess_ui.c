#include <string.h>
#include "chess_api.h"
#include "chess_dirty.h"
#include "chess_geom.h"
#include "chess_ui.h"
#include "hal_display.h"

/* RGB565 */
#define C_LIGHT 0xC616 /* warm light square */
#define C_DARK 0x5Aeb
#define C_HI 0xFE60   /* selection */
#define C_STRIP 0x2104
#define C_WHITE_P 0xFFFF
#define C_BLACK_P 0x0000
#define C_OUTLINE 0x8410

static int8_t s_last[64];
static chess_dirty_mask_t s_dirty;
static int s_highlight = -1;
static bool s_strip_dirty = true;

static uint16_t square_color(int sq, int highlight_sq)
{
    const int col = sq % 8;
    const int row = sq / 8;
    const bool light = ((row + col) & 1) == 0;
    if (sq == highlight_sq)
    {
        return C_HI;
    }
    return light ? C_LIGHT : C_DARK;
}

static uint16_t piece_color(int8_t p)
{
    return p > 0 ? C_WHITE_P : C_BLACK_P;
}

static void draw_piece_glyph(int x, int y, int8_t p)
{
    if (p == 0)
    {
        return;
    }
    const int abs_p = p > 0 ? p : -p;
    const uint16_t fg = piece_color(p);
    /* Simple nested rects — enough to tell piece presence/color until sprites. */
    const int inset = 4 + (6 - abs_p); /* king larger, pawn smaller */
    hal_display_fill_rect(x + inset, y + inset, UI_SQUARE_PX - 2 * inset,
                          UI_SQUARE_PX - 2 * inset, fg);
    if (abs_p >= 5)
    {
        /* queen/king: small center mark */
        hal_display_fill_rect(x + UI_SQUARE_PX / 2 - 2, y + UI_SQUARE_PX / 2 - 2,
                              4, 4, C_OUTLINE);
    }
}

static void draw_one_square(int sq)
{
    int x, y, w, h;
    chess_geom_square_rect(sq, &x, &y, &w, &h);
    const int8_t p = (int8_t)chess_get_square(sq);
    hal_display_fill_rect(x, y, w, h, square_color(sq, s_highlight));
    draw_piece_glyph(x, y, p);
    s_last[sq] = p;
}

static void draw_strip(void)
{
    hal_display_fill_rect(UI_STRIP_X, 0, UI_STRIP_W, UI_PANEL_H, C_STRIP);
    /* Side indicator: top block white/black */
    const uint16_t side = chess_side_to_move() ? C_WHITE_P : C_BLACK_P;
    hal_display_fill_rect(UI_STRIP_X + 16, 16, UI_STRIP_W - 32, 40, side);
    s_strip_dirty = false;
}

void chess_ui_init(void)
{
    memset(s_last, 0, sizeof(s_last));
    chess_dirty_all(&s_dirty);
    s_highlight = -1;
    s_strip_dirty = true;
}

void chess_ui_invalidate_all(void)
{
    chess_dirty_all(&s_dirty);
    s_strip_dirty = true;
}

void chess_ui_sync_from_game(int highlight_sq)
{
    int8_t curr[64];
    for (int i = 0; i < 64; i++)
    {
        curr[i] = (int8_t)chess_get_square(i);
    }
    chess_dirty_from_boards(s_last, curr, &s_dirty);

    if (highlight_sq != s_highlight)
    {
        if (s_highlight >= 0)
        {
            chess_dirty_add(&s_dirty, s_highlight);
        }
        if (highlight_sq >= 0)
        {
            chess_dirty_add(&s_dirty, highlight_sq);
        }
        s_highlight = highlight_sq;
    }

    s_strip_dirty = true; /* side-to-move may have changed */
}

void chess_ui_paint(void)
{
    for (int i = 0; i < 64; i++)
    {
        if (chess_dirty_test(s_dirty, i))
        {
            draw_one_square(i);
        }
    }
    chess_dirty_clear(&s_dirty);
    if (s_strip_dirty)
    {
        draw_strip();
    }
}

void chess_ui_get_last_drawn(int8_t out[64])
{
    if (out)
    {
        memcpy(out, s_last, 64);
    }
}
