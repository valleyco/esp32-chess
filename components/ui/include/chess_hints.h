#pragma once
#include <stdint.h>
#include "chess_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** Bit i set ⇒ square i is a legal destination from `from`. */
uint64_t chess_hints_from_moves(int from, const chess_move_t *moves, int n);

/**
 * Collect legal destinations for `from` via chess_legal_moves().
 * from < 0 ⇒ returns 0.
 */
uint64_t chess_hints_collect(int from);

#ifdef __cplusplus
}
#endif
