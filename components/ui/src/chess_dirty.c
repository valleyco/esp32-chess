#include "chess_dirty.h"

void chess_dirty_clear(chess_dirty_mask_t *m)
{
    if (m)
    {
        *m = 0;
    }
}

void chess_dirty_add(chess_dirty_mask_t *m, int sq)
{
    if (!m || sq < 0 || sq > 63)
    {
        return;
    }
    *m |= (chess_dirty_mask_t)1 << sq;
}

bool chess_dirty_test(chess_dirty_mask_t m, int sq)
{
    if (sq < 0 || sq > 63)
    {
        return false;
    }
    return (m & ((chess_dirty_mask_t)1 << sq)) != 0;
}

int chess_dirty_count(chess_dirty_mask_t m)
{
    int n = 0;
    while (m)
    {
        n += (int)(m & 1);
        m >>= 1;
    }
    return n;
}

void chess_dirty_all(chess_dirty_mask_t *m)
{
    if (m)
    {
        *m = ~(chess_dirty_mask_t)0;
    }
}

void chess_dirty_from_boards(const int8_t prev[64], const int8_t curr[64],
                             chess_dirty_mask_t *m)
{
    if (!prev || !curr || !m)
    {
        return;
    }
    for (int i = 0; i < 64; i++)
    {
        if (prev[i] != curr[i])
        {
            chess_dirty_add(m, i);
        }
    }
}
