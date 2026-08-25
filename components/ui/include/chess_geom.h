#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

enum
{
    UI_PANEL_W = 320,
    UI_PANEL_H = 240,
    UI_BOARD_X = 0,
    UI_BOARD_Y = 0,
    UI_SQUARE_PX = 30,
    UI_BOARD_PX = 240, /* 8 * 30 */
    UI_STRIP_X = 240,
    UI_STRIP_W = 80,
};

/** True if (px,py) is inside the 240×240 board. */
bool chess_geom_on_board(int px, int py);

/**
 * Panel pixel → square index (a8=0 … h1=63).
 * Returns -1 if outside the board (including status strip).
 */
int chess_geom_panel_to_square(int px, int py);

/** Square index → top-left panel rect of that square. */
void chess_geom_square_rect(int sq, int *x, int *y, int *w, int *h);

/** True if (px,py) is in the right-hand status strip. */
bool chess_geom_on_strip(int px, int py);

#ifdef __cplusplus
}
#endif
