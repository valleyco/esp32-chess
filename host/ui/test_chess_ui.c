#include "../test_assert.h"
#include "chess_dirty.h"
#include "chess_fsm.h"
#include "chess_geom.h"

enum
{
    SQ_A8 = 0,
    SQ_H8 = 7,
    SQ_A1 = 56,
    SQ_H1 = 63,
    SQ_E2 = 52,
    SQ_E4 = 36,
    SQ_E5 = 28,
};

static void test_geom_corners(void)
{
    ASSERT_EQ_INT(SQ_A8, chess_geom_panel_to_square(0, 0));
    ASSERT_EQ_INT(SQ_A8, chess_geom_panel_to_square(15, 15));
    ASSERT_EQ_INT(SQ_H8, chess_geom_panel_to_square(239, 0));
    ASSERT_EQ_INT(SQ_A1, chess_geom_panel_to_square(0, 239));
    ASSERT_EQ_INT(SQ_H1, chess_geom_panel_to_square(239, 239));
    ASSERT_EQ_INT(SQ_E2, chess_geom_panel_to_square(4 * 30 + 10, 6 * 30 + 10));
    ASSERT_EQ_INT(SQ_E4, chess_geom_panel_to_square(4 * 30 + 10, 4 * 30 + 10));
}

static void test_geom_outside(void)
{
    ASSERT_EQ_INT(-1, chess_geom_panel_to_square(240, 0));
    ASSERT_EQ_INT(-1, chess_geom_panel_to_square(300, 100));
    ASSERT_EQ_INT(-1, chess_geom_panel_to_square(-1, 10));
    ASSERT_TRUE(chess_geom_on_strip(250, 10));
    ASSERT_TRUE(!chess_geom_on_board(250, 10));
}

static void test_geom_square_rect(void)
{
    int x = 0, y = 0, w = 0, h = 0;
    chess_geom_square_rect(SQ_E2, &x, &y, &w, &h);
    ASSERT_EQ_INT(4 * 30, x);
    ASSERT_EQ_INT(6 * 30, y);
    ASSERT_EQ_INT(30, w);
    ASSERT_EQ_INT(30, h);
}

static void test_dirty_e2e4(void)
{
    int8_t prev[64] = {0};
    int8_t curr[64] = {0};
    prev[SQ_E2] = 1;
    curr[SQ_E4] = 1;
    /* empty elsewhere already 0 */

    chess_dirty_mask_t m = 0;
    chess_dirty_from_boards(prev, curr, &m);
    ASSERT_EQ_INT(2, chess_dirty_count(m));
    ASSERT_TRUE(chess_dirty_test(m, SQ_E2));
    ASSERT_TRUE(chess_dirty_test(m, SQ_E4));
    ASSERT_TRUE(!chess_dirty_test(m, SQ_E5));
}

static void test_dirty_highlight_or(void)
{
    chess_dirty_mask_t m = 0;
    chess_dirty_add(&m, SQ_E2);
    chess_dirty_add(&m, SQ_E4);
    ASSERT_EQ_INT(2, chess_dirty_count(m));
    chess_dirty_add(&m, SQ_E2); /* idempotent */
    ASSERT_EQ_INT(2, chess_dirty_count(m));
}

static void test_fsm_select_move_cancel(void)
{
    chess_fsm_t f;
    chess_fsm_init(&f);

    ASSERT_EQ_INT(CHESS_FSM_NONE, (int)chess_fsm_tap_square(&f, SQ_E2, false));
    ASSERT_EQ_INT(CHESS_FSM_IDLE, (int)f.state);

    ASSERT_EQ_INT(CHESS_FSM_SELECT, (int)chess_fsm_tap_square(&f, SQ_E2, true));
    ASSERT_EQ_INT(CHESS_FSM_SELECTED, (int)f.state);
    ASSERT_EQ_INT(SQ_E2, f.selected);

    ASSERT_EQ_INT(CHESS_FSM_CANCEL, (int)chess_fsm_tap_square(&f, SQ_E2, true));
    ASSERT_EQ_INT(CHESS_FSM_IDLE, (int)f.state);

    chess_fsm_tap_square(&f, SQ_E2, true);
    ASSERT_EQ_INT(CHESS_FSM_MOVE, (int)chess_fsm_tap_square(&f, SQ_E4, false));
    ASSERT_EQ_INT(SQ_E2, f.c1);
    ASSERT_EQ_INT(SQ_E4, f.c2);
    ASSERT_EQ_INT(CHESS_FSM_IDLE, (int)f.state);
}

static void test_strip_hits(void)
{
    ASSERT_EQ_INT(UI_STRIP_NEW, (int)chess_geom_strip_hit(250, 70, false));
    ASSERT_EQ_INT(UI_STRIP_UNDO, (int)chess_geom_strip_hit(250, 110, false));
    ASSERT_EQ_INT(UI_STRIP_CALIB, (int)chess_geom_strip_hit(250, 160, false));
    ASSERT_EQ_INT(UI_STRIP_TIME, (int)chess_geom_strip_hit(250, 200, false));
    ASSERT_EQ_INT(UI_STRIP_PROMO_Q, (int)chess_geom_strip_hit(250, 70, true));
    ASSERT_EQ_INT(UI_STRIP_PROMO_N, (int)chess_geom_strip_hit(250, 200, true));
    ASSERT_EQ_INT(UI_STRIP_NONE, (int)chess_geom_strip_hit(10, 70, false));
}

int main(void)
{
    test_geom_corners();
    test_geom_outside();
    test_geom_square_rect();
    test_dirty_e2e4();
    test_dirty_highlight_or();
    test_fsm_select_move_cancel();
    test_strip_hits();
    return test_report();
}
