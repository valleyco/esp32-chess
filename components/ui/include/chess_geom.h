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

typedef enum
{
    UI_STRIP_NONE = 0,
    UI_STRIP_SIDE,  /* top status / side indicator */
    UI_STRIP_NEW,
    UI_STRIP_UNDO,
    UI_STRIP_CALIB,
    UI_STRIP_TIME,
    UI_STRIP_PROMO_Q,
    UI_STRIP_PROMO_R,
    UI_STRIP_PROMO_B,
    UI_STRIP_PROMO_N,
} ui_strip_hit_t;

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

/**
 * Hit-test strip controls.
 * If promo_mode, maps mid strip to Q/R/B/N instead of NEW/UNDO/CAL/TIME.
 */
ui_strip_hit_t chess_geom_strip_hit(int px, int py, bool promo_mode);

/** Button rect for drawing (hit id → x,y,w,h). */
void chess_geom_strip_button_rect(ui_strip_hit_t hit, int *x, int *y, int *w,
                                  int *h);

#ifdef __cplusplus
}
#endif
