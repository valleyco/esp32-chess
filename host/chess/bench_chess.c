/**
 * Fixed-depth chess engine bench (host).
 * Metrics are depth + nodes (CPU-independent). Wall-clock nps is secondary.
 *
 *   make -C host/chess bench          # print table
 *   make -C host/chess test           # unit tests + golden bench
 */
#include "../../components/chess/include/chess_api.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static void sq_name(int sq, char out[3])
{
    if (sq < 0 || sq > 63) {
        out[0] = '?';
        out[1] = '?';
        out[2] = '\0';
        return;
    }
    out[0] = (char)('a' + (sq % 8));
    out[1] = (char)('0' + (8 - sq / 8));
    out[2] = '\0';
}

typedef struct {
    const char *name;
    const char *fen;
    int depth;
    int expect_c1;
    int expect_c2;
    unsigned long expect_nodes;
    int expect_depth;
} bench_case_t;

/*
 * Goldens: host -O0 (host/chess/Makefile). Update after intentional search changes.
 * Squares: a8=0 … h1=63.
 */
static const bench_case_t k_cases[] = {
    {
        .name = "start_d3",
        .fen = CHESS_START_FEN,
        .depth = 3,
        .expect_c1 = 57, /* b1 */
        .expect_c2 = 42, /* c3 */
        .expect_nodes = 626,
        .expect_depth = 3,
    },
    {
        .name = "after_e2e4_d3",
        .fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3",
        .depth = 3,
        .expect_c1 = 6,  /* g8 */
        .expect_c2 = 21, /* f6 */
        .expect_nodes = 759,
        .expect_depth = 3,
    },
    {
        .name = "kiwipete_d3",
        .fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
        .depth = 3,
        .expect_c1 = 52, /* e2 */
        .expect_c2 = 16, /* a6 */
        .expect_nodes = 13540,
        .expect_depth = 3,
    },
};

static int run_case(const bench_case_t *c, int check_goldens)
{
    chess_search_result_t r;
    memset(&r, 0, sizeof(r));

    if (!chess_set_fen(c->fen)) {
        fprintf(stderr, "FAIL %s: set_fen\n", c->name);
        return 1;
    }

    const clock_t t0 = clock();
    if (!chess_think_depth(c->depth, &r)) {
        fprintf(stderr, "FAIL %s: think_depth\n", c->name);
        return 1;
    }
    const clock_t t1 = clock();
    const double ms = 1000.0 * (double)(t1 - t0) / (double)CLOCKS_PER_SEC;
    const double nps = (ms > 0.0) ? (1000.0 * (double)r.nodes / ms) : 0.0;

    char from[3], to[3];
    sq_name(r.c1, from);
    sq_name(r.c2, to);

    printf("%-14s depth=%d nodes=%lu move=%s%s score=%d  (%.1f ms, %.0f nps)\n",
           c->name, r.depth, r.nodes, from, to, r.score, ms, nps);

    if (!check_goldens) {
        return 0;
    }

    int failed = 0;
    if (r.depth != c->expect_depth) {
        fprintf(stderr, "FAIL %s: depth expected %d got %d\n", c->name,
                c->expect_depth, r.depth);
        failed = 1;
    }
    if (r.nodes != c->expect_nodes) {
        fprintf(stderr, "FAIL %s: nodes expected %lu got %lu\n", c->name,
                c->expect_nodes, r.nodes);
        failed = 1;
    }
    if (r.c1 != c->expect_c1 || r.c2 != c->expect_c2) {
        fprintf(stderr, "FAIL %s: move expected %d→%d got %d→%d\n", c->name,
                c->expect_c1, c->expect_c2, r.c1, r.c2);
        failed = 1;
    }
    return failed;
}

int main(int argc, char **argv)
{
    int check_goldens = 1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--print-only") == 0) {
            check_goldens = 0;
        }
    }

    printf("chess bench — fixed depth (nodes primary)\n");
    int fails = 0;
    const int n = (int)(sizeof(k_cases) / sizeof(k_cases[0]));
    for (int i = 0; i < n; i++) {
        fails += run_case(&k_cases[i], check_goldens);
    }

    if (fails) {
        fprintf(stderr, "%d bench case(s) failed\n", fails);
        return 1;
    }
    if (check_goldens) {
        printf("bench ok (%d cases)\n", n);
    } else {
        printf("(print-only; no golden asserts)\n");
    }
    return 0;
}
