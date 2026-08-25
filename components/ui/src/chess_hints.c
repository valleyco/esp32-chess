#include "chess_hints.h"

uint64_t chess_hints_from_moves(int from, const chess_move_t *moves, int n)
{
    uint64_t m = 0;
    if (from < 0 || from > 63 || !moves || n <= 0)
    {
        return 0;
    }
    for (int i = 0; i < n; i++)
    {
        if (moves[i].c1 != from)
        {
            continue;
        }
        const int to = moves[i].c2;
        if (to >= 0 && to < 64)
        {
            m |= (uint64_t)1 << to;
        }
    }
    return m;
}

uint64_t chess_hints_collect(int from)
{
    if (from < 0 || from > 63)
    {
        return 0;
    }
    chess_move_t moves[128];
    const int n = chess_legal_moves(moves, 128);
    return chess_hints_from_moves(from, moves, n);
}
