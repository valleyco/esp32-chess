/* 24x24 1bpp chess silhouettes; bit23 = leftmost pixel. */
#include "ui_pieces.h"

#include <stdbool.h>

static const uint32_t SPR_PAWN[UI_PIECE_PX] = {
    0x000000u,
    0x000000u,
    0x003C00u,
    0x007E00u,
    0x007E00u,
    0x003C00u,
    0x001800u,
    0x003C00u,
    0x007E00u,
    0x00FF00u,
    0x01FF80u,
    0x01FF80u,
    0x00FF00u,
    0x007E00u,
    0x00FF00u,
    0x01FF80u,
    0x03FFC0u,
    0x03FFC0u,
    0x03FFC0u,
    0x07FFE0u,
    0x0FFFF0u,
    0x0FFFF0u,
    0x000000u,
    0x000000u,
};

static const uint32_t SPR_KNIGHT[UI_PIECE_PX] = {
    0x000000u,
    0x00F800u,
    0x01FE00u,
    0x03FF00u,
    0x03CF80u,
    0x07C7C0u,
    0x0783C0u,
    0x0F81C0u,
    0x0FC1C0u,
    0x0FF1C0u,
    0x07FDC0u,
    0x03FFE0u,
    0x01FFF0u,
    0x00FFF0u,
    0x007FF0u,
    0x003FF0u,
    0x007FF0u,
    0x00FFF0u,
    0x01FFF8u,
    0x03FFFCu,
    0x07FFFCu,
    0x0FFFFCu,
    0x000000u,
    0x000000u,
};

static const uint32_t SPR_BISHOP[UI_PIECE_PX] = {
    0x000000u,
    0x001800u,
    0x003C00u,
    0x007E00u,
    0x00E700u,
    0x01C380u,
    0x0381C0u,
    0x0381C0u,
    0x01C380u,
    0x00E700u,
    0x007E00u,
    0x003C00u,
    0x007E00u,
    0x00FF00u,
    0x01FF80u,
    0x03FFC0u,
    0x03FFC0u,
    0x01FF80u,
    0x00FF00u,
    0x01FF80u,
    0x03FFC0u,
    0x07FFE0u,
    0x000000u,
    0x000000u,
};

static const uint32_t SPR_ROOK[UI_PIECE_PX] = {
    0x000000u,
    0x066600u,
    0x066600u,
    0x0FFFC0u,
    0x0FFFC0u,
    0x07FF80u,
    0x07FF80u,
    0x07FF80u,
    0x07FF80u,
    0x07FF80u,
    0x07FF80u,
    0x07FF80u,
    0x07FF80u,
    0x07FF80u,
    0x0FFFC0u,
    0x0FFFC0u,
    0x1FFFE0u,
    0x1FFFE0u,
    0x1FFFE0u,
    0x1FFFE0u,
    0x1FFFE0u,
    0x1FFFE0u,
    0x000000u,
    0x000000u,
};

static const uint32_t SPR_QUEEN[UI_PIECE_PX] = {
    0x000000u,
    0x086100u,
    0x1CF380u,
    0x1FFFE0u,
    0x0FFFC0u,
    0x07FF80u,
    0x03FF00u,
    0x07FF80u,
    0x0FFFC0u,
    0x1FFFE0u,
    0x1FFFE0u,
    0x0FFFC0u,
    0x07FF80u,
    0x03FF00u,
    0x07FF80u,
    0x0FFFC0u,
    0x1FFFE0u,
    0x1FFFE0u,
    0x3FFFF0u,
    0x3FFFF0u,
    0x3FFFF0u,
    0x7FFFF8u,
    0x000000u,
    0x000000u,
};

static const uint32_t SPR_KING[UI_PIECE_PX] = {
    0x000000u,
    0x001800u,
    0x001800u,
    0x007E00u,
    0x001800u,
    0x001800u,
    0x007E00u,
    0x00FF00u,
    0x01C380u,
    0x0381C0u,
    0x0381C0u,
    0x0381C0u,
    0x01C380u,
    0x00FF00u,
    0x007E00u,
    0x00FF00u,
    0x01FF80u,
    0x03FFC0u,
    0x03FFC0u,
    0x07FFE0u,
    0x0FFFF0u,
    0x0FFFF0u,
    0x000000u,
    0x000000u,
};

static const uint32_t *sprite_for(int type)
{
    switch (type) {
    case 1:
        return SPR_PAWN;
    case 2:
        return SPR_KNIGHT;
    case 3:
        return SPR_BISHOP;
    case 4:
        return SPR_ROOK;
    case 5:
        return SPR_QUEEN;
    case 6:
        return SPR_KING;
    default:
        return 0;
    }
}

static int mask_get(const uint32_t *spr, int x, int y)
{
    if (x < 0 || y < 0 || x >= UI_PIECE_PX || y >= UI_PIECE_PX)
    {
        return 0;
    }
    return (spr[y] >> (23 - x)) & 1;
}

static int is_outline(const uint32_t *spr, int px, int py)
{
    if (mask_get(spr, px, py))
    {
        return 0;
    }
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
            {
                continue;
            }
            if (mask_get(spr, px + dx, py + dy))
            {
                return 1;
            }
        }
    }
    return 0;
}

static void emit_spans(const uint32_t *spr, int x, int y, uint16_t color,
                       int outline_pass, ui_piece_span_fn span, void *user)
{
    for (int py = 0; py < UI_PIECE_PX; py++)
    {
        int run = -1;
        for (int px = 0; px <= UI_PIECE_PX; px++)
        {
            const int on = (px < UI_PIECE_PX) &&
                           (outline_pass ? is_outline(spr, px, py)
                                         : mask_get(spr, px, py));
            if (on)
            {
                if (run < 0)
                {
                    run = px;
                }
            }
            else if (run >= 0)
            {
                span(x + run, y + py, px - run, color, user);
                run = -1;
            }
        }
    }
}

int ui_piece_fill_count(int type)
{
    const uint32_t *spr = sprite_for(type);
    if (!spr)
    {
        return 0;
    }
    int n = 0;
    for (int y = 0; y < UI_PIECE_PX; y++)
    {
        uint32_t row = spr[y];
        while (row)
        {
            n += (int)(row & 1u);
            row >>= 1;
        }
    }
    return n;
}

void ui_piece_draw(int type, bool white, int x, int y, uint16_t fill,
                   uint16_t outline, ui_piece_span_fn span, void *user)
{
    (void)white;
    const uint32_t *spr = sprite_for(type);
    if (!spr || !span)
    {
        return;
    }
    emit_spans(spr, x, y, outline, 1, span, user);
    emit_spans(spr, x, y, fill, 0, span, user);
}

