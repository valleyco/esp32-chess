#include "../test_assert.h"
#include "../../components/chess/include/chess_api.h"

#include <string.h>

enum {
    SQ_E2 = 52,
    SQ_E4 = 36,
    SQ_E5 = 28,
    SQ_E7 = 12,
    SQ_A2 = 48,
    SQ_A7 = 8,
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
    ASSERT_TRUE(chess_try_move(SQ_E2, SQ_E4, CHESS_PROMO_QUEEN_DEFAULT));
    ASSERT_EQ_INT(0, chess_get_square(SQ_E2));
    ASSERT_EQ_INT(1, chess_get_square(SQ_E4));
    ASSERT_EQ_INT(0, chess_side_to_move());
    ASSERT_EQ_INT(1, chess_ply());
}

static void test_e2e5_illegal(void)
{
    chess_new_game();
    ASSERT_TRUE(!chess_try_move(SQ_E2, SQ_E5, CHESS_PROMO_QUEEN_DEFAULT));
    ASSERT_EQ_INT(1, chess_get_square(SQ_E2));
    ASSERT_EQ_INT(1, chess_side_to_move());
}

static void test_think_applies_legal_black_move(void)
{
    chess_new_game();
    ASSERT_TRUE(chess_try_move(SQ_E2, SQ_E4, CHESS_PROMO_QUEEN_DEFAULT));

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
    ASSERT_TRUE(chess_try_move(SQ_E2, SQ_E4, CHESS_PROMO_QUEEN_DEFAULT));
    ASSERT_TRUE(chess_think(500));
    ASSERT_TRUE(chess_undo());

    ASSERT_EQ_INT(1, chess_get_square(SQ_E2));
    ASSERT_EQ_INT(0, chess_get_square(SQ_E4));
    ASSERT_EQ_INT(1, chess_side_to_move());
    ASSERT_EQ_INT(0, chess_ply());
}

static void test_undo_ply(void)
{
    chess_new_game();
    ASSERT_TRUE(!chess_undo_ply());
    ASSERT_TRUE(chess_try_move(SQ_E2, SQ_E4, CHESS_PROMO_QUEEN_DEFAULT));
    ASSERT_TRUE(chess_try_move(SQ_E7, SQ_E5, CHESS_PROMO_QUEEN_DEFAULT));
    ASSERT_EQ_INT(2, chess_ply());
    ASSERT_TRUE(chess_undo_ply());
    ASSERT_EQ_INT(1, chess_ply());
    ASSERT_EQ_INT(0, chess_side_to_move());
    ASSERT_EQ_INT(-1, chess_get_square(SQ_E7)); /* black pawn back */
    ASSERT_TRUE(chess_undo_ply());
    ASSERT_EQ_INT(0, chess_ply());
    ASSERT_EQ_INT(1, chess_get_square(SQ_E2));
    ASSERT_TRUE(!chess_undo_ply());
}

static void test_last_move(void)
{
    int c1 = -1;
    int c2 = -1;
    chess_new_game();
    ASSERT_TRUE(!chess_last_move(&c1, &c2));

    ASSERT_TRUE(chess_try_move(SQ_E2, SQ_E4, CHESS_PROMO_QUEEN_DEFAULT));
    ASSERT_TRUE(chess_last_move(&c1, &c2));
    ASSERT_EQ_INT(SQ_E2, c1);
    ASSERT_EQ_INT(SQ_E4, c2);

    ASSERT_TRUE(chess_think(500));
    ASSERT_TRUE(chess_last_move(&c1, &c2));
    ASSERT_TRUE(c1 >= 0 && c1 < 64);
    ASSERT_TRUE(c2 >= 0 && c2 < 64);
    ASSERT_TRUE(c1 != c2);

    ASSERT_TRUE(chess_undo());
    ASSERT_TRUE(!chess_last_move(&c1, &c2));
}

static void test_try_move_side_to_move(void)
{
    chess_new_game();
    ASSERT_TRUE(chess_try_move(SQ_E2, SQ_E4, CHESS_PROMO_QUEEN_DEFAULT));
    ASSERT_EQ_INT(0, chess_side_to_move());
    ASSERT_TRUE(chess_try_move(SQ_E7, SQ_E5, CHESS_PROMO_QUEEN_DEFAULT));
    ASSERT_EQ_INT(1, chess_side_to_move());
    ASSERT_EQ_INT(-1, chess_get_square(SQ_E5));
    ASSERT_EQ_INT(0, chess_get_square(SQ_E7));
}

static void test_set_get_fen(void)
{
    char buf[128];
    chess_new_game();
    ASSERT_TRUE(chess_set_fen(CHESS_START_FEN));
    ASSERT_EQ_INT(1, chess_get_square(SQ_E2));

    const int n = chess_get_fen(buf, sizeof(buf));
    ASSERT_TRUE(n > 20);
    ASSERT_TRUE(strstr(buf, "rnbqkbnr/pppppppp/") != NULL);
    ASSERT_TRUE(strstr(buf, " w ") != NULL);

    ASSERT_TRUE(chess_try_move(SQ_E2, SQ_E4, CHESS_PROMO_QUEEN_DEFAULT));
    ASSERT_TRUE(chess_set_fen(CHESS_START_FEN));
    ASSERT_EQ_INT(0, chess_ply());
    ASSERT_EQ_INT(1, chess_get_square(SQ_E2));
    ASSERT_EQ_INT(1, chess_side_to_move());

    ASSERT_TRUE(!chess_set_fen(""));
    ASSERT_TRUE(!chess_set_fen(NULL));
}

static void test_legal_moves_start(void)
{
    chess_move_t moves[64];
    chess_new_game();
    const int n = chess_legal_moves(moves, 64);
    /* Standard start: 20 legal moves. */
    ASSERT_EQ_INT(20, n);

    int found_e2e4 = 0;
    for (int i = 0; i < n; i++) {
        if (moves[i].c1 == SQ_E2 && moves[i].c2 == SQ_E4 && moves[i].promo == 0) {
            found_e2e4 = 1;
        }
    }
    ASSERT_TRUE(found_e2e4);

    ASSERT_EQ_INT(20, chess_legal_moves(NULL, 0));
}

static void test_think_depth_result(void)
{
    chess_search_result_t r;
    memset(&r, 0, sizeof(r));
    chess_new_game();
    ASSERT_TRUE(chess_try_move(SQ_E2, SQ_E4, CHESS_PROMO_QUEEN_DEFAULT));
    ASSERT_TRUE(chess_think_depth(3, &r));
    ASSERT_EQ_INT(1, chess_side_to_move());
    ASSERT_EQ_INT(2, chess_ply());
    ASSERT_TRUE(r.c1 >= 0 && r.c1 < 64);
    ASSERT_TRUE(r.c2 >= 0 && r.c2 < 64);
    ASSERT_EQ_INT(3, r.depth);
    ASSERT_TRUE(r.nodes > 0);
}

static void test_think_depth_stable_nodes(void)
{
    chess_search_result_t a, b;
    chess_new_game();
    chess_try_move(SQ_E2, SQ_E4, CHESS_PROMO_QUEEN_DEFAULT);
    ASSERT_TRUE(chess_think_depth(3, &a));

    chess_new_game();
    chess_try_move(SQ_E2, SQ_E4, CHESS_PROMO_QUEEN_DEFAULT);
    ASSERT_TRUE(chess_think_depth(3, &b));

    ASSERT_EQ_INT(a.c1, b.c1);
    ASSERT_EQ_INT(a.c2, b.c2);
    ASSERT_EQ_INT((int)a.nodes, (int)b.nodes);
    ASSERT_EQ_INT(a.depth, b.depth);
}

int main(void)
{
    test_start_position();
    test_e2e4_legal();
    test_e2e5_illegal();
    test_think_applies_legal_black_move();
    test_undo_restores();
    test_undo_ply();
    test_last_move();
    test_try_move_side_to_move();
    test_set_get_fen();
    test_legal_moves_start();
    test_think_depth_result();
    test_think_depth_stable_nodes();
    return test_report();
}
