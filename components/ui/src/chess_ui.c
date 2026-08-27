#include <string.h>
#include "chess_api.h"
#include "chess_dirty.h"
#include "chess_geom.h"
#include "chess_hints.h"
#include "chess_ui.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "hal_display.h"
#include "ui_font.h"
#include "ui_pieces.h"

static const char *TAG_UI = "ui";
static bool s_paint_log;

/* RGB565 — strip palette */
#define C_LIGHT 0xC616
#define C_DARK 0x5AEB
#define C_HI 0xFE60       /* selection */
#define C_HINT 0x87F0     /* legal destination */
#define C_LAST_FROM 0x6DDB /* muted teal — origin */
#define C_LAST_TO 0x9E7F   /* brighter teal — destination */
#define C_STRIP 0x18C3      /* deep slate */
#define C_STRIP_GAP 0x10A2  /* darker gutter between buttons */
#define C_BUSY 0xFDC0       /* warm amber */
#define C_BTN_NEW 0x1C6B    /* green */
#define C_BTN_UNDO 0xB2E0   /* amber */
#define C_BTN_CAL 0x2A57    /* blue */
#define C_BTN_TIME 0x4A2F   /* purple */
#define C_WHITE_P 0xFFFF
#define C_BLACK_P 0x0000
#define C_RED 0xF800
#define C_BLUE 0x3A7F
#define C_GREEN 0x2E0A
#define C_CYAN 0x07FF
#define C_MAGENTA 0xD01F
#define C_TEXT_ON_DARK 0xFFFF
#define C_TEXT_ON_LIGHT 0x1082

enum
{
    STRIP_DIRTY_SIDE = 1u << 0,
    STRIP_DIRTY_BTNS = 1u << 1, /* NEW/UNDO/CAL (+ promo column) */
    STRIP_DIRTY_TIME = 1u << 2, /* TIME button / bar only */
    STRIP_DIRTY_BG = 1u << 3,
    STRIP_DIRTY_ALL =
        STRIP_DIRTY_SIDE | STRIP_DIRTY_BTNS | STRIP_DIRTY_TIME | STRIP_DIRTY_BG,
};

static int8_t s_last[64];
static chess_dirty_mask_t s_dirty;
static int s_highlight = -1;
static uint64_t s_hints;
static int s_last_from = -1;
static int s_last_to = -1;
static unsigned s_strip_dirty = STRIP_DIRTY_ALL;
static bool s_busy = false;
static chess_ui_mode_t s_mode = UI_MODE_PLAY;
static chess_status_t s_status = CHESS_STATUS_OK;
static unsigned s_think_ms = 3000;
static int s_drawn_side = -1; /* last painted side-to-move: 0/1, or -1 unknown */

static uint16_t square_color(int sq, int highlight_sq)
{
    if (sq == highlight_sq)
    {
        return C_HI;
    }
    if (s_hints & ((uint64_t)1 << sq))
    {
        return C_HINT;
    }
    if (sq == s_last_to)
    {
        return C_LAST_TO;
    }
    if (sq == s_last_from)
    {
        return C_LAST_FROM;
    }
    const int col = sq % 8;
    const int row = sq / 8;
    const bool light = ((row + col) & 1) == 0;
    return light ? C_LIGHT : C_DARK;
}

static uint16_t piece_fill(int8_t p)
{
    return p > 0 ? C_WHITE_P : C_BLACK_P;
}

static uint16_t piece_outline(int8_t p)
{
    return p > 0 ? C_BLACK_P : C_WHITE_P;
}

/* One square RGB565 scratch (30×30); avoids per-span SPI for pieces. */
static uint16_t s_sq_buf[UI_SQUARE_PX * UI_SQUARE_PX];

typedef struct
{
    uint16_t *buf;
    int ox;
    int oy;
} sq_compose_t;

static void piece_span_into_sq(int x, int y, int w, uint16_t color, void *user)
{
    sq_compose_t *c = (sq_compose_t *)user;
    int lx = x - c->ox;
    int ly = y - c->oy;
    if (ly < 0 || ly >= UI_SQUARE_PX || w <= 0)
    {
        return;
    }
    if (lx < 0)
    {
        w += lx;
        lx = 0;
    }
    if (lx + w > UI_SQUARE_PX)
    {
        w = UI_SQUARE_PX - lx;
    }
    if (w <= 0)
    {
        return;
    }
    uint16_t *row = c->buf + (size_t)ly * (size_t)UI_SQUARE_PX + (size_t)lx;
    for (int i = 0; i < w; i++)
    {
        row[i] = color;
    }
}

static void draw_one_square(int sq)
{
    int x, y, w, h;
    chess_geom_square_rect(sq, &x, &y, &w, &h);
    const int8_t p = (int8_t)chess_get_square(sq);
    const uint16_t bg = square_color(sq, s_highlight);

    for (int i = 0; i < UI_SQUARE_PX * UI_SQUARE_PX; i++)
    {
        s_sq_buf[i] = bg;
    }

    if (p != 0)
    {
        const int type = p > 0 ? p : -p;
        const int ox = x + (UI_SQUARE_PX - UI_PIECE_PX) / 2;
        const int oy = y + (UI_SQUARE_PX - UI_PIECE_PX) / 2;
        sq_compose_t ctx = {.buf = s_sq_buf, .ox = x, .oy = y};
        ui_piece_draw(type, p > 0, ox, oy, piece_fill(p), piece_outline(p),
                      piece_span_into_sq, &ctx);
    }

    hal_display_blit_rgb565(x, y, w, h, s_sq_buf);
    s_last[sq] = p;
}

typedef struct
{
    uint16_t color;
} span_ctx_t;

static void font_span(int x, int y, int w, void *user)
{
    const span_ctx_t *ctx = (const span_ctx_t *)user;
    if (w > 0)
    {
        hal_display_fill_rect(x, y, w, 1, ctx->color);
    }
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
    span_ctx_t ctx = {.color = fg};
    ui_font_draw_spans(label, tx, ty, scale, font_span, &ctx);
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
    /* Gutter behind button so partial redraws don't leave stale pixels. */
    hal_display_fill_rect(UI_STRIP_X, y - 2, UI_STRIP_W, h + 4, C_STRIP_GAP);
    hal_display_fill_rect(x, y, w, h, fill);
    /* 1px light top / left edge for a little depth. */
    hal_display_fill_rect(x, y, w, 1, C_HI);
    hal_display_fill_rect(x, y, 1, h, C_HI);
    if (label && label[0])
    {
        draw_label(x, y, w, h, label, scale, fg);
    }
}

static const char *side_label(uint16_t *fg_out)
{
    if (s_mode == UI_MODE_OVER)
    {
        *fg_out = C_TEXT_ON_DARK;
        return (s_status == CHESS_STATUS_CHECKMATE) ? "MATE" : "DRAW";
    }
    if (s_busy)
    {
        *fg_out = C_TEXT_ON_LIGHT;
        return "WAIT";
    }
    if (chess_side_to_move())
    {
        *fg_out = C_TEXT_ON_LIGHT;
        return "W";
    }
    *fg_out = C_TEXT_ON_DARK;
    return "B";
}

static void draw_strip_side(void)
{
    uint16_t side = C_BTN_TIME;
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
    uint16_t side_fg = C_TEXT_ON_DARK;
    const char *side_txt = side_label(&side_fg);
    draw_btn(UI_STRIP_SIDE, side, side_txt, 2, side_fg);
    s_drawn_side = chess_side_to_move();
}

static void draw_strip_time(void)
{
    if (s_mode == UI_MODE_PROMO)
    {
        draw_btn(UI_STRIP_PROMO_N, C_MAGENTA, "N", 3, C_TEXT_ON_DARK);
        return;
    }

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
    draw_btn(UI_STRIP_TIME, C_BTN_TIME, time_label, 2, C_TEXT_ON_DARK);
    const int bar = (w - 8) * secs / 5;
    hal_display_fill_rect(x + 4, y + h - 8, bar, 4, C_HI);
}

static void draw_strip_btns(void)
{
    if (s_mode == UI_MODE_PROMO)
    {
        draw_btn(UI_STRIP_PROMO_Q, C_HI, "Q", 3, C_TEXT_ON_LIGHT);
        draw_btn(UI_STRIP_PROMO_R, C_CYAN, "R", 3, C_TEXT_ON_LIGHT);
        draw_btn(UI_STRIP_PROMO_B, C_GREEN, "B", 3, C_TEXT_ON_DARK);
        draw_strip_time();
        return;
    }

    draw_btn(UI_STRIP_NEW, C_BTN_NEW, "NEW", 2, C_TEXT_ON_DARK);
    draw_btn(UI_STRIP_UNDO, C_BTN_UNDO, "UNDO", 2, C_TEXT_ON_LIGHT);
    draw_btn(UI_STRIP_CALIB, C_BTN_CAL, "CAL", 2, C_TEXT_ON_DARK);
    draw_strip_time();
}

static void draw_strip(void)
{
    if (s_strip_dirty & STRIP_DIRTY_BG)
    {
        hal_display_fill_rect(UI_STRIP_X, 0, UI_STRIP_W, UI_PANEL_H, C_STRIP);
    }
    if (s_strip_dirty & STRIP_DIRTY_SIDE)
    {
        draw_strip_side();
    }
    if (s_strip_dirty & STRIP_DIRTY_BTNS)
    {
        draw_strip_btns();
    }
    else if (s_strip_dirty & STRIP_DIRTY_TIME)
    {
        draw_strip_time();
    }
    s_strip_dirty = 0;
}

void chess_ui_init(void)
{
    memset(s_last, 0, sizeof(s_last));
    chess_dirty_all(&s_dirty);
    s_highlight = -1;
    s_hints = 0;
    s_last_from = -1;
    s_last_to = -1;
    s_strip_dirty = STRIP_DIRTY_ALL;
    s_busy = false;
    s_mode = UI_MODE_PLAY;
    s_status = CHESS_STATUS_OK;
    s_think_ms = 3000;
    s_drawn_side = -1;
}

void chess_ui_invalidate_all(void)
{
    chess_dirty_all(&s_dirty);
    s_strip_dirty = STRIP_DIRTY_ALL;
    s_drawn_side = -1;
    s_last_from = -1;
    s_last_to = -1;
    s_hints = 0;
}

void chess_ui_set_busy(bool busy)
{
    if (s_busy == busy)
    {
        return;
    }
    s_busy = busy;
    s_strip_dirty |= STRIP_DIRTY_SIDE;
}

void chess_ui_set_mode(chess_ui_mode_t mode)
{
    if (s_mode == mode)
    {
        return;
    }
    s_mode = mode;
    /* Promo ↔ play swaps the whole control column. */
    s_strip_dirty |= STRIP_DIRTY_SIDE | STRIP_DIRTY_BTNS | STRIP_DIRTY_BG;
}

void chess_ui_set_status(chess_status_t st)
{
    if (s_status == st)
    {
        return;
    }
    s_status = st;
    s_strip_dirty |= STRIP_DIRTY_SIDE;
}

void chess_ui_set_think_ms(unsigned ms)
{
    if (s_think_ms == ms)
    {
        return;
    }
    s_think_ms = ms;
    s_strip_dirty |= STRIP_DIRTY_TIME;
}

static void dirty_sq(int sq)
{
    if (sq >= 0 && sq < 64)
    {
        chess_dirty_add(&s_dirty, sq);
    }
}

static void set_last_move_squares(int from, int to)
{
    if (from == s_last_from && to == s_last_to)
    {
        return;
    }
    dirty_sq(s_last_from);
    dirty_sq(s_last_to);
    s_last_from = from;
    s_last_to = to;
    dirty_sq(s_last_from);
    dirty_sq(s_last_to);
}

static void set_hint_mask(uint64_t next)
{
    if (next == s_hints)
    {
        return;
    }
    const uint64_t changed = s_hints ^ next;
    s_hints = next;
    for (int i = 0; i < 64; i++)
    {
        if (changed & ((uint64_t)1 << i))
        {
            chess_dirty_add(&s_dirty, i);
        }
    }
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
        dirty_sq(s_highlight);
        dirty_sq(highlight_sq);
        s_highlight = highlight_sq;
    }

    set_hint_mask(chess_hints_collect(highlight_sq));

    int c1 = -1;
    int c2 = -1;
    if (chess_last_move(&c1, &c2))
    {
        set_last_move_squares(c1, c2);
    }
    else
    {
        set_last_move_squares(-1, -1);
    }

    const int side = chess_side_to_move();
    if (side != s_drawn_side)
    {
        s_strip_dirty |= STRIP_DIRTY_SIDE;
    }
}

void chess_ui_set_paint_log(bool enable)
{
    s_paint_log = enable;
}

void chess_ui_paint(void)
{
    const int nsq = chess_dirty_count(s_dirty);
    const unsigned strip_flags = s_strip_dirty;
    const int64_t t0 = esp_timer_get_time();

    for (int i = 0; i < 64; i++)
    {
        if (chess_dirty_test(s_dirty, i))
        {
            draw_one_square(i);
        }
    }
    chess_dirty_clear(&s_dirty);
    const int64_t t1 = esp_timer_get_time();

    if (s_strip_dirty)
    {
        draw_strip();
    }
    const int64_t t2 = esp_timer_get_time();

    if (s_paint_log)
    {
        const double board_ms = (double)(t1 - t0) / 1000.0;
        const double strip_ms = (double)(t2 - t1) / 1000.0;
        const double total_ms = (double)(t2 - t0) / 1000.0;
        const double per_sq =
            (nsq > 0) ? (board_ms / (double)nsq) : 0.0;
        ESP_LOGI(TAG_UI,
                 "paint squares=%d board=%.1fms (%.2fms/sq) strip=0x%x "
                 "%.1fms total=%.1fms",
                 nsq, board_ms, per_sq, strip_flags, strip_ms, total_ms);
    }
}

void chess_ui_get_last_drawn(int8_t out[64])
{
    if (out)
    {
        memcpy(out, s_last, 64);
    }
}
