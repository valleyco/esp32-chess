#include "ui_font.h"

#include <ctype.h>
#include <string.h>

/* 5×7, bit4 = left column. Enough for strip labels. */
static const uint8_t G_SPACE[7] = {0, 0, 0, 0, 0, 0, 0};
static const uint8_t G_0[7] = {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E};
static const uint8_t G_1[7] = {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
static const uint8_t G_2[7] = {0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F};
static const uint8_t G_3[7] = {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E};
static const uint8_t G_4[7] = {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02};
static const uint8_t G_5[7] = {0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E};
static const uint8_t G_A[7] = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
static const uint8_t G_B[7] = {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
static const uint8_t G_C[7] = {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E};
static const uint8_t G_D[7] = {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E};
static const uint8_t G_E[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
static const uint8_t G_I[7] = {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E};
static const uint8_t G_L[7] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
static const uint8_t G_M[7] = {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11};
static const uint8_t G_N[7] = {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11};
static const uint8_t G_O[7] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
static const uint8_t G_Q[7] = {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D};
static const uint8_t G_R[7] = {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11};
static const uint8_t G_S[7] = {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
static const uint8_t G_T[7] = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
static const uint8_t G_U[7] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
static const uint8_t G_W[7] = {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11};

const uint8_t *ui_font_glyph(char c)
{
    c = (char)toupper((unsigned char)c);
    switch (c)
    {
    case ' ':
        return G_SPACE;
    case '0':
        return G_0;
    case '1':
        return G_1;
    case '2':
        return G_2;
    case '3':
        return G_3;
    case '4':
        return G_4;
    case '5':
        return G_5;
    case 'A':
        return G_A;
    case 'B':
        return G_B;
    case 'C':
        return G_C;
    case 'D':
        return G_D;
    case 'E':
        return G_E;
    case 'I':
        return G_I;
    case 'L':
        return G_L;
    case 'M':
        return G_M;
    case 'N':
        return G_N;
    case 'O':
        return G_O;
    case 'Q':
        return G_Q;
    case 'R':
        return G_R;
    case 'S':
        return G_S;
    case 'T':
        return G_T;
    case 'U':
        return G_U;
    case 'W':
        return G_W;
    default:
        return G_SPACE;
    }
}

int ui_font_text_width(const char *s, int scale)
{
    if (!s || scale < 1)
    {
        return 0;
    }
    const int n = (int)strlen(s);
    if (n == 0)
    {
        return 0;
    }
    return n * (UI_FONT_W * scale) + (n - 1) * scale;
}

void ui_font_draw_spans(const char *s, int x, int y, int scale,
                        ui_font_span_fn span, void *user)
{
    if (!s || !span || scale < 1)
    {
        return;
    }
    int cx = x;
    for (; *s; s++)
    {
        const uint8_t *g = ui_font_glyph(*s);
        for (int row = 0; row < UI_FONT_H; row++)
        {
            const uint8_t bits = g[row];
            /* Build a scale-wide bitmask of on/off columns, then emit runs. */
            for (int dy = 0; dy < scale; dy++)
            {
                const int py = y + row * scale + dy;
                int run = -1;
                for (int col = 0; col <= UI_FONT_W; col++)
                {
                    const int on =
                        (col < UI_FONT_W) && (bits & (1u << (4 - col)));
                    if (on)
                    {
                        if (run < 0)
                        {
                            run = col;
                        }
                    }
                    else if (run >= 0)
                    {
                        const int px = cx + run * scale;
                        const int pw = (col - run) * scale;
                        span(px, py, pw, user);
                        run = -1;
                    }
                }
            }
        }
        cx += UI_FONT_W * scale + scale;
    }
}

typedef struct
{
    ui_font_plot_fn plot;
    void *user;
} plot_via_span_t;

static void span_to_plots(int x, int y, int w, void *user)
{
    const plot_via_span_t *ctx = (const plot_via_span_t *)user;
    for (int i = 0; i < w; i++)
    {
        ctx->plot(x + i, y, ctx->user);
    }
}

void ui_font_draw(const char *s, int x, int y, int scale, ui_font_plot_fn plot,
                  void *user)
{
    if (!s || !plot || scale < 1)
    {
        return;
    }
    plot_via_span_t ctx = {.plot = plot, .user = user};
    ui_font_draw_spans(s, x, y, scale, span_to_plots, &ctx);
}
