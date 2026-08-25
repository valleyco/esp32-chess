#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "chess_api.h"
#include "chess_dirty.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    UI_MODE_PLAY = 0,
    UI_MODE_PROMO,
    UI_MODE_OVER, /* mate / stalemate */
} chess_ui_mode_t;

/** Init UI state (last_drawn empty, full dirty). Does not paint. */
void chess_ui_init(void);

/** Force full board + strip redraw on next paint. */
void chess_ui_invalidate_all(void);

/** Mark squares dirty from pole diff vs last_drawn; OR highlight square (-1 none). */
void chess_ui_sync_from_game(int highlight_sq);

/** Status strip shows "busy" (engine thinking). Redraws side badge only. */
void chess_ui_set_busy(bool busy);

/** Play / promotion picker / game-over strip layout (side + buttons). */
void chess_ui_set_mode(chess_ui_mode_t mode);

/** Game-over / info banner (mate, stalemate). Side badge only. */
void chess_ui_set_status(chess_status_t st);

/** Think-time shown on TIME button (milliseconds). Buttons column only. */
void chess_ui_set_think_ms(unsigned ms);

/** Paint dirty squares + dirty strip regions (side / buttons / full). */
void chess_ui_paint(void);

/** Snapshot last_drawn (for host tests via separate build — device uses paint). */
void chess_ui_get_last_drawn(int8_t out[64]);

#ifdef __cplusplus
}
#endif
