#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** Bit i set ⇒ square i needs redraw. */
typedef uint64_t chess_dirty_mask_t;

void chess_dirty_clear(chess_dirty_mask_t *m);
void chess_dirty_add(chess_dirty_mask_t *m, int sq);
bool chess_dirty_test(chess_dirty_mask_t m, int sq);
int chess_dirty_count(chess_dirty_mask_t m);

/** Mark all 64 squares. */
void chess_dirty_all(chess_dirty_mask_t *m);

/**
 * Diff prev vs curr board (64 signed piece codes). Sets bits where values differ.
 * Does not clear existing bits in *m — OR into mask.
 */
void chess_dirty_from_boards(const int8_t prev[64], const int8_t curr[64],
                             chess_dirty_mask_t *m);

#ifdef __cplusplus
}
#endif
