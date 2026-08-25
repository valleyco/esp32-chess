#include "chess_geom.h"

bool chess_geom_on_board(int px, int py)
{
    return px >= UI_BOARD_X && px < UI_BOARD_X + UI_BOARD_PX &&
           py >= UI_BOARD_Y && py < UI_BOARD_Y + UI_BOARD_PX;
}

bool chess_geom_on_strip(int px, int py)
{
    return px >= UI_STRIP_X && px < UI_STRIP_X + UI_STRIP_W && py >= 0 &&
           py < UI_PANEL_H;
}

int chess_geom_panel_to_square(int px, int py)
{
    if (!chess_geom_on_board(px, py))
    {
        return -1;
    }
    const int col = (px - UI_BOARD_X) / UI_SQUARE_PX;
    const int row = (py - UI_BOARD_Y) / UI_SQUARE_PX;
    if (col < 0 || col > 7 || row < 0 || row > 7)
    {
        return -1;
    }
    return row * 8 + col;
}

void chess_geom_square_rect(int sq, int *x, int *y, int *w, int *h)
{
    if (sq < 0 || sq > 63)
    {
        if (x)
        {
            *x = 0;
        }
        if (y)
        {
            *y = 0;
        }
        if (w)
        {
            *w = 0;
        }
        if (h)
        {
            *h = 0;
        }
        return;
    }
    const int col = sq % 8;
    const int row = sq / 8;
    if (x)
    {
        *x = UI_BOARD_X + col * UI_SQUARE_PX;
    }
    if (y)
    {
        *y = UI_BOARD_Y + row * UI_SQUARE_PX;
    }
    if (w)
    {
        *w = UI_SQUARE_PX;
    }
    if (h)
    {
        *h = UI_SQUARE_PX;
    }
}

/*
 * Strip layout (x=240..319):
 *   0..49   SIDE
 *  50..94   NEW  / promo Q
 *  95..139  UNDO / promo R
 * 140..184  CAL  / promo B
 * 185..229  TIME / promo N
 */
static ui_strip_hit_t strip_row_hit(int py, bool promo_mode)
{
    if (py < 50)
    {
        return UI_STRIP_SIDE;
    }
    if (py < 95)
    {
        return promo_mode ? UI_STRIP_PROMO_Q : UI_STRIP_NEW;
    }
    if (py < 140)
    {
        return promo_mode ? UI_STRIP_PROMO_R : UI_STRIP_UNDO;
    }
    if (py < 185)
    {
        return promo_mode ? UI_STRIP_PROMO_B : UI_STRIP_CALIB;
    }
    if (py < 230)
    {
        return promo_mode ? UI_STRIP_PROMO_N : UI_STRIP_TIME;
    }
    return UI_STRIP_NONE;
}

ui_strip_hit_t chess_geom_strip_hit(int px, int py, bool promo_mode)
{
    if (!chess_geom_on_strip(px, py))
    {
        return UI_STRIP_NONE;
    }
    return strip_row_hit(py, promo_mode);
}

void chess_geom_strip_button_rect(ui_strip_hit_t hit, int *x, int *y, int *w,
                                  int *h)
{
    int yy = 0;
    int hh = 40;
    switch (hit)
    {
    case UI_STRIP_SIDE:
        yy = 8;
        hh = 40;
        break;
    case UI_STRIP_NEW:
    case UI_STRIP_PROMO_Q:
        yy = 52;
        hh = 40;
        break;
    case UI_STRIP_UNDO:
    case UI_STRIP_PROMO_R:
        yy = 97;
        hh = 40;
        break;
    case UI_STRIP_CALIB:
    case UI_STRIP_PROMO_B:
        yy = 142;
        hh = 40;
        break;
    case UI_STRIP_TIME:
    case UI_STRIP_PROMO_N:
        yy = 187;
        hh = 40;
        break;
    default:
        yy = 0;
        hh = 0;
        break;
    }
    if (x)
    {
        *x = UI_STRIP_X + 8;
    }
    if (y)
    {
        *y = yy;
    }
    if (w)
    {
        *w = UI_STRIP_W - 16;
    }
    if (h)
    {
        *h = hh;
    }
}
