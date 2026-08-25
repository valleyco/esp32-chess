#include <string.h>
#include "chess_api.h"
#include "chess_dirty.h"
#include "chess_geom.h"
#include "chess_ui.h"
#include "hal_display.h"
#include "ui_font.h"

/* RGB565 */
#define C_LIGHT 0xC616
#define C_DARK 0x5AEB
#define C_HI 0xFE60
#define C_STRIP 0x2104
#define C_BUSY 0xFE60
#define C_BTN 0x39E7
#define C_WHITE_P 0xFFFF
#define C_BLACK_P 0x0000
#define C_OUTLINE 0x8410
#define C_RED 0xF800
#define C_BLUE 0x001F
#define C_GREEN 0x07E0
#define C_CYAN 0x07FF
#define C_MAGENTA 0xF81F

static int8_t s_last[64];
static chess_dirty_mask_t s_dirty;
static int s_highlight = -1;
static bool s_strip_dirty = true;
static bool s_busy = false;
static chess_ui_mode_t s_mode = UI_MODE_PLAY;
static chess_status_t s_status = CHESS_STATUS_OK;
static unsigned s_think_ms = 3000;

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
    const int inset = 4 + (6 - abs_p);
    hal_display_fill_rect(x + inset, y + inset, UI_SQUARE_PX - 2 * inset,
                          UI_SQUARE_PX - 2 * inset, fg);
    if (abs_p >= 5)
    {
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

typedef struct
{
    uint16_t color;
} plot_ctx_t;

static void plot_px(int x, int y, void *user)
{
    const plot_ctx_t *ctx = (const plot_ctx_t *)user;
    hal_display_fill_rect(x, y, 1, 1, ctx->color);
}

static void draw_label(int x, int y, int w, int h, const char *label, int scale,
                       uint16_t fg)
{
    const int tw = ui_font_text_width(label, scale);
    const int th = UI_FONT_H * scale;
    int tx = x + (w - tw) / 2;
    int ty = y + (h - th) / 2;
    if (tx < x)
    {
        tx = x;
    }
    if (ty < y)
    {
        ty = y;
    }
    plot_ctx_t ctx = {.color = fg};
    ui_font_draw(label, tx, ty, scale, plot_px, &ctx);
}

static void draw_btn(ui_strip_hit_t hit, uint16_t fill, const char *label,
                     int scale, uint16_t fg)
{
    int x, y, w, h;
    chess_geom_strip_button_rect(hit, &x, &y, &w, &h);
    if (w <= 0 || h <= 0)
    {
        return;
    }
    hal_display_fill_rect(x, y, w, h, fill);
    if (label && label[0])
    {
        draw_label(x, y, w, h, label, scale, fg);
    }
}

static const char *side_label(uint16_t *fg_out)
{
    if (s_mode == UI_MODE_OVER)
    {
        *fg_out = C_WHITE_P;
        return (s_status == CHESS_STATUS_CHECKMATE) ? "MATE" : "DRAW";
    }
    if (s_busy)
    {
        *fg_out = C_BLACK_P;
        return "WAIT";
    }
    if (chess_side_to_move())
    {
        *fg_out = C_BLACK_P;
        return "W";
    }
    *fg_out = C_WHITE_P;
    return "B";
}

static void draw_strip(void)
{
    hal_display_fill_rect(UI_STRIP_X, 0, UI_STRIP_W, UI_PANEL_H, C_STRIP);

    uint16_t side = C_BTN;
    if (s_mode == UI_MODE_OVER)
    {
        side = (s_status == CHESS_STATUS_CHECKMATE) ? C_RED : C_BLUE;
    }
    else if (s_busy)
    {
        side = C_BUSY;
    }
    else
    {
        side = chess_side_to_move() ? C_WHITE_P : C_BLACK_P;
    }
    uint16_t side_fg = C_WHITE_P;
    const char *side_txt = side_label(&side_fg);
    draw_btn(UI_STRIP_SIDE, side, side_txt, 2, side_fg);

    if (s_mode == UI_MODE_PROMO)
    {
        draw_btn(UI_STRIP_PROMO_Q, C_HI, "Q", 3, C_BLACK_P);
        draw_btn(UI_STRIP_PROMO_R, C_CYAN, "R", 3, C_BLACK_P);
        draw_btn(UI_STRIP_PROMO_B, C_GREEN, "B", 3, C_BLACK_P);
        draw_btn(UI_STRIP_PROMO_N, C_MAGENTA, "N", 3, C_WHITE_P);
    }
    else
    {
        draw_btn(UI_STRIP_NEW, C_BTN, "NEW", 2, C_WHITE_P);
        draw_btn(UI_STRIP_UNDO, C_BTN, "UNDO", 2, C_WHITE_P);
        draw_btn(UI_STRIP_CALIB, C_BTN, "CAL", 2, C_WHITE_P);
        /* TIME: think seconds as main label + width bar. */
        int x, y, w, h;
        chess_geom_strip_button_rect(UI_STRIP_TIME, &x, &y, &w, &h);
        int secs = (int)(s_think_ms / 1000);
        if (secs < 1)
        {
            secs = 1;
        }
        if (secs > 5)
        {
            secs = 5;
        }
        char time_label[4];
        time_label[0] = (char)('0' + secs);
        time_label[1] = 'S';
        time_label[2] = '\0';
        draw_btn(UI_STRIP_TIME, C_BTN, time_label, 2, C_WHITE_P);
        const int bar = (w - 8) * secs / 5;
        hal_display_fill_rect(x + 4, y + h - 8, bar, 4, C_HI);
    }

    s_strip_dirty = false;
}

void chess_ui_init(void)
{
    memset(s_last, 0, sizeof(s_last));
    chess_dirty_all(&s_dirty);
    s_highlight = -1;
    s_strip_dirty = true;
    s_busy = false;
    s_mode = UI_MODE_PLAY;
    s_status = CHESS_STATUS_OK;
    s_think_ms = 3000;
}

void chess_ui_invalidate_all(void)
{
    chess_dirty_all(&s_dirty);
    s_strip_dirty = true;
}

void chess_ui_set_busy(bool busy)
{
    if (s_busy == busy)
    {
        return;
    }
    s_busy = busy;
    s_strip_dirty = true;
}

void chess_ui_set_mode(chess_ui_mode_t mode)
{
    if (s_mode == mode)
    {
        return;
    }
    s_mode = mode;
    s_strip_dirty = true;
}

void chess_ui_set_status(chess_status_t st)
{
    s_status = st;
    s_strip_dirty = true;
}

void chess_ui_set_think_ms(unsigned ms)
{
    if (s_think_ms == ms)
    {
        return;
    }
    s_think_ms = ms;
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

    s_strip_dirty = true;
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
