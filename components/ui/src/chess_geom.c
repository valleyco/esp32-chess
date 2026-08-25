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
