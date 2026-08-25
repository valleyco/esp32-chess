#include "../test_assert.h"
#include "../../components/chess/include/chess_api.h"

enum {
    SQ_E2 = 52,
    SQ_E4 = 36,
    SQ_E5 = 28,
    SQ_E7 = 12,
};

static void test_start_position(void)
{
    chess_new_game();
    ASSERT_EQ_INT(1, chess_get_square(SQ_E2));
    ASSERT_EQ_INT(-1, chess_get_square(SQ_E7));
    ASSERT_EQ_INT(1, chess_side_to_move());
    ASSERT_EQ_INT(0, chess_ply());
}

static void test_e2e4_legal(void)
{
    chess_new_game();
    ASSERT_TRUE(chess_try_move(SQ_E2, SQ_E4, 0));
    ASSERT_EQ_INT(0, chess_get_square(SQ_E2));
    ASSERT_EQ_INT(1, chess_get_square(SQ_E4));
    ASSERT_EQ_INT(0, chess_side_to_move());
    ASSERT_EQ_INT(1, chess_ply());
}

static void test_e2e5_illegal(void)
{
    chess_new_game();
    ASSERT_TRUE(!chess_try_move(SQ_E2, SQ_E5, 0));
    ASSERT_EQ_INT(1, chess_get_square(SQ_E2));
    ASSERT_EQ_INT(1, chess_side_to_move());
}

static void test_think_applies_legal_black_move(void)
{
    chess_new_game();
    ASSERT_TRUE(chess_try_move(SQ_E2, SQ_E4, 0));

    short before[64];
    for (int i = 0; i < 64; i++) {
        before[i] = (short)chess_get_square(i);
    }

    ASSERT_TRUE(chess_think(500));
    ASSERT_EQ_INT(1, chess_side_to_move());
    ASSERT_EQ_INT(2, chess_ply());

    int changed = 0;
    for (int i = 0; i < 64; i++) {
        if (chess_get_square(i) != before[i]) {
            changed++;
        }
    }
    ASSERT_TRUE(changed >= 2);
}

static void test_undo_restores(void)
{
    chess_new_game();
    ASSERT_TRUE(chess_try_move(SQ_E2, SQ_E4, 0));
    ASSERT_TRUE(chess_think(500));
    ASSERT_TRUE(chess_undo());

    ASSERT_EQ_INT(1, chess_get_square(SQ_E2));
    ASSERT_EQ_INT(0, chess_get_square(SQ_E4));
    ASSERT_EQ_INT(1, chess_side_to_move());
    ASSERT_EQ_INT(0, chess_ply());
}

int main(void)
{
    test_start_position();
    test_e2e4_legal();
    test_e2e5_illegal();
    test_think_applies_legal_black_move();
    test_undo_restores();
    return test_report();
}
