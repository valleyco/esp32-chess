#include "chess_api.h"

#include "chess_engine_internal.h"

static bool s_engine_ready = false;

static void ensure_engine_ready(void)
{
    if (!s_engine_ready) {
        chess_engine_init();
        s_engine_ready = true;
    }
}

static int promo_to_step_type(int promo)
{
    if (promo == 0) {
        return 7;
    }
    if (promo < 0) {
        promo = -promo;
    }
    return promo + 2;
}

static bool apply_step(const step_t &step)
{
    movestep(0, const_cast<step_t &>(step));
    movepos(0, const_cast<step_t &>(step));
    game_steps[game_ply] = step;
    pos[1].w = !pos[0].w;
    pos[0] = pos[1];
    game_ply++;
    return true;
}

static bool find_and_apply_move(int c1, int c2, int want_type, bool require_type)
{
    generate_steps(0);
    for (int i = 0; i < pos[0].n_steps; i++) {
        const step_t &s = pos[0].steps[i];
        if (s.c1 != c1 || s.c2 != c2) {
            continue;
        }
        if (require_type && s.type != want_type) {
            continue;
        }
        if (!require_type && s.type >= 4 && want_type != s.type) {
            continue;
        }
        pos[0].cur_step = i;
        return apply_step(s);
    }
    return false;
}

extern "C" void chess_new_game(void)
{
    ensure_engine_ready();
    fen(String("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"));
    game_ply = 0;
    game_pos = pos[0];
    for (int i = 0; i < 64; i++) {
        game_pole[i] = pole[i];
    }
    game_w = 1;
}

extern "C" bool chess_try_move(int c1, int c2, int promo)
{
    if (!(game_ply % 2 == 0 && game_w)) {
        return false;
    }

    generate_steps(0);

    int promotion_candidates = 0;
    for (int i = 0; i < pos[0].n_steps; i++) {
        const step_t &s = pos[0].steps[i];
        if (s.c1 != c1 || s.c2 != c2) {
            continue;
        }
        if (s.type >= 4) {
            promotion_candidates++;
        }
    }

    if (promotion_candidates == 0) {
        return find_and_apply_move(c1, c2, 0, false);
    }

    const int want_type = promo_to_step_type(promo);
    return find_and_apply_move(c1, c2, want_type, true);
}

extern "C" bool chess_is_promotion_move(int c1, int c2)
{
    if (!(game_ply % 2 == 0 && game_w)) {
        return false;
    }
    generate_steps(0);
    for (int i = 0; i < pos[0].n_steps; i++) {
        const step_t &s = pos[0].steps[i];
        if (s.c1 == c1 && s.c2 == c2 && s.type >= 4) {
            return true;
        }
    }
    return false;
}

extern "C" chess_status_t chess_status(void)
{
    generate_steps(0);
    int legal = 0;
    int in_check = 0;
    for (int i = 0; i < pos[0].n_steps; i++) {
        movestep(0, pos[0].steps[i]);
        const int check = pos[0].w ? check_w() : check_b();
        if (!check) {
            legal++;
        }
        backstep(0, pos[0].steps[i]);
    }
    in_check = pos[0].w ? check_w() : check_b();
    if (legal > 0) {
        return CHESS_STATUS_OK;
    }
    return in_check ? CHESS_STATUS_CHECKMATE : CHESS_STATUS_STALEMATE;
}

extern "C" bool chess_think(unsigned timeout_ms)
{
    timelimith = timeout_ms;
    halt = 0;
    pos[0].best.c1 = -1;
    solve_step();
    if (pos[0].best.c1 == -1) {
        return false;
    }

    generate_steps(0);
    for (int i = 0; i < pos[0].n_steps; i++) {
        const step_t &s = pos[0].steps[i];
        if (s.c1 == pos[0].best.c1 && s.c2 == pos[0].best.c2 && s.type == pos[0].best.type) {
            pos[0].cur_step = i;
            movestep(0, pos[0].steps[i]);
            movepos(0, pos[0].steps[i]);
            game_steps[game_ply] = pos[0].steps[i];
            pos[0] = pos[1];
            game_ply++;
            return true;
        }
    }
    return false;
}

extern "C" bool chess_undo(void)
{
    if (game_ply <= 1) {
        return false;
    }

    pos[0] = game_pos;
    for (int i = 0; i < 64; i++) {
        pole[i] = game_pole[i];
    }
    game_ply -= 2;
    for (int i = 0; i < game_ply; i++) {
        movestep(0, game_steps[i]);
        movepos(0, game_steps[i]);
        generate_steps(1);
        pos[1].w = !pos[0].w;
        pos[0] = pos[1];
    }
    return true;
}

extern "C" int chess_get_square(int i)
{
    if (i < 0 || i > 63) {
        return 0;
    }
    return pole[i];
}

extern "C" int chess_side_to_move(void)
{
    return pos[0].w ? 1 : 0;
}

extern "C" int chess_ply(void)
{
    return game_ply;
}
