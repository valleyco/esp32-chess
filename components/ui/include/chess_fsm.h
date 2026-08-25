#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    CHESS_FSM_IDLE = 0,
    CHESS_FSM_SELECTED,
} chess_fsm_state_t;

typedef enum
{
    CHESS_FSM_NONE = 0,
    CHESS_FSM_SELECT,  /* selected a from-square */
    CHESS_FSM_CANCEL,  /* deselected */
    CHESS_FSM_MOVE,    /* c1→c2 requested (caller runs chess_try_move) */
} chess_fsm_event_t;

typedef struct
{
    chess_fsm_state_t state;
    int selected; /* -1 if none */
    int c1;
    int c2;
} chess_fsm_t;

void chess_fsm_init(chess_fsm_t *f);

/**
 * Tap on a board square (0..63).
 * selectable: true if this square may be chosen as a from-square (caller's rule).
 * On CHESS_FSM_MOVE, f->c1 and f->c2 are set; state returns to IDLE.
 */
chess_fsm_event_t chess_fsm_tap_square(chess_fsm_t *f, int sq, bool selectable);

#ifdef __cplusplus
}
#endif
