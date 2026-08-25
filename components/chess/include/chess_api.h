#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void chess_new_game(void);

/** Apply a human move if legal. promo: 0 = queen (default), or piece type (±2..±5). */
bool chess_try_move(int c1, int c2, int promo);

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

#ifdef __cplusplus
}
#endif
