#pragma once
#include <stdint.h>
#include "chess_dirty.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** Init UI state (last_drawn empty, full dirty). Does not paint. */
void chess_ui_init(void);

/** Force full board + strip redraw on next paint. */
void chess_ui_invalidate_all(void);

/** Mark squares dirty from pole diff vs last_drawn; OR highlight square (-1 none). */
void chess_ui_sync_from_game(int highlight_sq);

/** Paint all dirty squares + strip if needed. Updates last_drawn. */
void chess_ui_paint(void);

/** Snapshot last_drawn (for host tests via separate build — device uses paint). */
void chess_ui_get_last_drawn(int8_t out[64]);

#ifdef __cplusplus
}
#endif
