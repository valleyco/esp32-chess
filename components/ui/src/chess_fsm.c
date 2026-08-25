#include "chess_fsm.h"

void chess_fsm_init(chess_fsm_t *f)
{
    if (!f)
    {
        return;
    }
    f->state = CHESS_FSM_IDLE;
    f->selected = -1;
    f->c1 = -1;
    f->c2 = -1;
}

chess_fsm_event_t chess_fsm_tap_square(chess_fsm_t *f, int sq, bool selectable)
{
    if (!f || sq < 0 || sq > 63)
    {
        return CHESS_FSM_NONE;
    }

    if (f->state == CHESS_FSM_IDLE)
    {
        if (!selectable)
        {
            return CHESS_FSM_NONE;
        }
        f->state = CHESS_FSM_SELECTED;
        f->selected = sq;
        f->c1 = sq;
        f->c2 = -1;
        return CHESS_FSM_SELECT;
    }

    /* SELECTED */
    if (sq == f->selected)
    {
        f->state = CHESS_FSM_IDLE;
        f->selected = -1;
        f->c1 = -1;
        f->c2 = -1;
        return CHESS_FSM_CANCEL;
    }

    f->c1 = f->selected;
    f->c2 = sq;
    f->state = CHESS_FSM_IDLE;
    f->selected = -1;
    return CHESS_FSM_MOVE;
}
