#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CHESS_STATUS_OK = 0,
    CHESS_STATUS_CHECKMATE,
    CHESS_STATUS_STALEMATE,
} chess_status_t;

void chess_new_game(void);

/** Apply a human move if legal. promo: 0 = queen (default), or piece type 2..5 (NBRQ). */
bool chess_try_move(int c1, int c2, int promo);

/** True if c1→c2 is a legal promotion (needs piece choice). */
bool chess_is_promotion_move(int c1, int c2);

/** Search for timeout_ms, apply best move. Returns false if no move found. */
bool chess_think(unsigned timeout_ms);

/** Undo last human+engine pair when possible (game_ply > 1). */
bool chess_undo(void);

/** Board square value at index 0..63 (a8=0 … h1=63); ±1 pawn … ±6 king. */
int chess_get_square(int i);

/** 1 if White to move, 0 if Black. */
int chess_side_to_move(void);

/** Half-moves played in the current game line. */
int chess_ply(void);

/**
 * Last applied half-move (game_steps[ply-1]).
 * Returns false if no moves yet; otherwise writes c1/c2 (a8=0 … h1=63).
 */
bool chess_last_move(int *c1, int *c2);

/** Mate / stalemate for the side to move. */
chess_status_t chess_status(void);

#ifdef __cplusplus
}
#endif
