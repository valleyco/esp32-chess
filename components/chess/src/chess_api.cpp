#include "chess_api.h"

#include "chess_engine_internal.h"

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#endif

static bool s_engine_ready = false;

#ifdef ESP_PLATFORM
static SemaphoreHandle_t s_mu;

static void engine_lock(void)
{
    if (!s_mu)
    {
        s_mu = xSemaphoreCreateMutex();
    }
    if (s_mu)
    {
        xSemaphoreTake(s_mu, portMAX_DELAY);
    }
}

static void engine_unlock(void)
{
    if (s_mu)
    {
        xSemaphoreGive(s_mu);
    }
}
#else
static void engine_lock(void)
{
}
static void engine_unlock(void)
{
}
#endif

struct EngineGuard
{
    EngineGuard()
    {
        engine_lock();
    }
    ~EngineGuard()
    {
        engine_unlock();
    }
};

static void ensure_engine_ready(void)
{
    if (!s_engine_ready)
    {
        chess_engine_init();
        s_engine_ready = true;
    }
}

static void reset_game_history(void)
{
    game_ply = 0;
    game_pos = pos[0];
    for (int i = 0; i < 64; i++)
    {
        game_pole[i] = pole[i];
    }
    game_w = pos[0].w ? 1 : 0;
}

static int promo_to_step_type(int promo)
{
    if (promo == 0 || promo == CHESS_PIECE_QUEEN)
    {
        return 7;
    }
    if (promo < 0)
    {
        promo = -promo;
    }
    return promo + 2;
}

static int step_type_to_promo(int type)
{
    if (type < 4)
    {
        return 0;
    }
    /* type 4..7 → NBRQ → 2..5 */
    return type - 2;
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
    for (int i = 0; i < pos[0].n_steps; i++)
    {
        const step_t &s = pos[0].steps[i];
        if (s.c1 != c1 || s.c2 != c2)
        {
            continue;
        }
        if (require_type && s.type != want_type)
        {
            continue;
        }
        if (!require_type && s.type >= 4 && want_type != s.type)
        {
            continue;
        }
        pos[0].cur_step = i;
        return apply_step(s);
    }
    return false;
}

static bool step_leaves_own_king_safe(step_t &s)
{
    movestep(0, s);
    const int check = pos[0].w ? check_w() : check_b();
    backstep(0, s);
    return !check;
}

static bool run_search_and_apply(chess_search_result_t *out)
{
    halt = 0;
    pos[0].best.c1 = -1;
    solve_step();
    if (pos[0].best.c1 == -1)
    {
        return false;
    }

    chess_search_result_t local = {};
    local.c1 = pos[0].best.c1;
    local.c2 = pos[0].best.c2;
    local.promo = step_type_to_promo(pos[0].best.type);
    local.depth = lastbestdepth > 0 ? lastbestdepth : level;
    local.nodes = count;
    local.score = pos[0].best.weight;

    generate_steps(0);
    for (int i = 0; i < pos[0].n_steps; i++)
    {
        const step_t &s = pos[0].steps[i];
        if (s.c1 == pos[0].best.c1 && s.c2 == pos[0].best.c2 &&
            s.type == pos[0].best.type)
        {
            pos[0].cur_step = i;
            movestep(0, pos[0].steps[i]);
            movepos(0, pos[0].steps[i]);
            game_steps[game_ply] = pos[0].steps[i];
            pos[0] = pos[1];
            game_ply++;
            if (out)
            {
                *out = local;
            }
            return true;
        }
    }
    return false;
}

extern "C" void chess_new_game(void)
{
    EngineGuard g;
    ensure_engine_ready();
    fen(CHESS_START_FEN);
    reset_game_history();
}

extern "C" bool chess_set_fen(const char *fen_str)
{
    EngineGuard g;
    ensure_engine_ready();
    if (!fen_str || !fen_str[0])
    {
        return false;
    }
    if (!fen(fen_str))
    {
        return false;
    }
    reset_game_history();
    return true;
}

extern "C" int chess_get_fen(char *buf, size_t buflen)
{
    EngineGuard g;
    ensure_engine_ready();
    return fenout(0, buf, buflen);
}

extern "C" bool chess_try_move(int c1, int c2, int promo)
{
    EngineGuard g;
    ensure_engine_ready();
    generate_steps(0);

    int promotion_candidates = 0;
    for (int i = 0; i < pos[0].n_steps; i++)
    {
        const step_t &s = pos[0].steps[i];
        if (s.c1 != c1 || s.c2 != c2)
        {
            continue;
        }
        if (s.type >= 4)
        {
            promotion_candidates++;
        }
    }

    if (promotion_candidates == 0)
    {
        return find_and_apply_move(c1, c2, 0, false);
    }

    const int want_type = promo_to_step_type(promo);
    return find_and_apply_move(c1, c2, want_type, true);
}

extern "C" bool chess_is_promotion_move(int c1, int c2)
{
    EngineGuard g;
    ensure_engine_ready();
    generate_steps(0);
    for (int i = 0; i < pos[0].n_steps; i++)
    {
        const step_t &s = pos[0].steps[i];
        if (s.c1 == c1 && s.c2 == c2 && s.type >= 4)
        {
            return true;
        }
    }
    return false;
}

extern "C" int chess_legal_moves(chess_move_t *out, int max_out)
{
    EngineGuard g;
    ensure_engine_ready();
    if (max_out < 0)
    {
        max_out = 0;
    }
    generate_steps(0);
    int n = 0;
    for (int i = 0; i < pos[0].n_steps; i++)
    {
        step_t s = pos[0].steps[i];
        if (!step_leaves_own_king_safe(s))
        {
            continue;
        }
        if (out && n < max_out)
        {
            out[n].c1 = s.c1;
            out[n].c2 = s.c2;
            out[n].promo = step_type_to_promo(s.type);
        }
        n++;
    }
    return n;
}

extern "C" chess_status_t chess_status(void)
{
    EngineGuard g;
    ensure_engine_ready();
    generate_steps(0);
    int legal = 0;
    for (int i = 0; i < pos[0].n_steps; i++)
    {
        step_t s = pos[0].steps[i];
        if (step_leaves_own_king_safe(s))
        {
            legal++;
        }
    }
    const int in_check = pos[0].w ? check_w() : check_b();
    if (legal > 0)
    {
        return CHESS_STATUS_OK;
    }
    return in_check ? CHESS_STATUS_CHECKMATE : CHESS_STATUS_STALEMATE;
}

extern "C" bool chess_think_time(unsigned timeout_ms, chess_search_result_t *out)
{
    EngineGuard g;
    ensure_engine_ready();
    timelimith = timeout_ms ? timeout_ms : 1;
    search_max_level = 20;
    return run_search_and_apply(out);
}

extern "C" bool chess_think_depth(int max_depth, chess_search_result_t *out)
{
    EngineGuard g;
    ensure_engine_ready();
    if (max_depth < 2)
    {
        max_depth = 2;
    }
    if (max_depth > 20)
    {
        max_depth = 20;
    }
    search_max_level = max_depth;
    /* Large limit so depth, not wall clock, stops the search. */
    timelimith = 24UL * 60UL * 60UL * 1000UL;
    return run_search_and_apply(out);
}

extern "C" bool chess_think(unsigned timeout_ms)
{
    return chess_think_time(timeout_ms, nullptr);
}

static bool replay_to_ply(int target_ply)
{
    if (target_ply < 0 || target_ply > game_ply)
    {
        return false;
    }
    pos[0] = game_pos;
    for (int i = 0; i < 64; i++)
    {
        pole[i] = game_pole[i];
    }
    game_ply = target_ply;
    for (int i = 0; i < game_ply; i++)
    {
        movestep(0, game_steps[i]);
        movepos(0, game_steps[i]);
        generate_steps(1);
        pos[1].w = !pos[0].w;
        pos[0] = pos[1];
    }
    return true;
}

extern "C" bool chess_undo_ply(void)
{
    EngineGuard g;
    ensure_engine_ready();
    if (game_ply <= 0)
    {
        return false;
    }
    return replay_to_ply(game_ply - 1);
}

extern "C" bool chess_undo(void)
{
    EngineGuard g;
    ensure_engine_ready();
    if (game_ply <= 1)
    {
        return false;
    }
    return replay_to_ply(game_ply - 2);
}

extern "C" int chess_get_square(int i)
{
    EngineGuard g;
    ensure_engine_ready();
    if (i < 0 || i > 63)
    {
        return 0;
    }
    return pole[i];
}

extern "C" int chess_side_to_move(void)
{
    EngineGuard g;
    ensure_engine_ready();
    return pos[0].w ? 1 : 0;
}

extern "C" int chess_ply(void)
{
    EngineGuard g;
    ensure_engine_ready();
    return game_ply;
}

extern "C" bool chess_last_move(int *c1, int *c2)
{
    EngineGuard g;
    ensure_engine_ready();
    if (game_ply <= 0)
    {
        return false;
    }
    const step_t &s = game_steps[game_ply - 1];
    if (c1)
    {
        *c1 = s.c1;
    }
    if (c2)
    {
        *c2 = s.c2;
    }
    return true;
}
