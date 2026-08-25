#include "../test_assert.h"
#include "ui_pieces.h"

static int s_fill_px;
static int s_outline_px;
static uint16_t s_last_fill;
static uint16_t s_last_outline;

static void count_span(int x, int y, int w, uint16_t color, void *user)
{
    (void)x;
    (void)y;
    (void)user;
    if (color == 0xFFFF)
    {
        s_fill_px += w;
        s_last_fill = color;
    }
    else
    {
        s_outline_px += w;
        s_last_outline = color;
    }
}

static void test_fill_counts(void)
{
    for (int t = 1; t <= 6; t++)
    {
        const int n = ui_piece_fill_count(t);
        ASSERT_TRUE(n > 80);
        ASSERT_TRUE(n < 400);
    }
    ASSERT_EQ_INT(0, ui_piece_fill_count(0));
    ASSERT_EQ_INT(0, ui_piece_fill_count(7));
}

static void test_draw_spans(void)
{
    s_fill_px = 0;
    s_outline_px = 0;
    ui_piece_draw(1, true, 0, 0, 0xFFFF, 0x0000, count_span, NULL);
    ASSERT_EQ_INT(ui_piece_fill_count(1), s_fill_px);
    ASSERT_TRUE(s_outline_px > 20);
    ASSERT_EQ_INT(0xFFFF, (int)s_last_fill);
    ASSERT_EQ_INT(0x0000, (int)s_last_outline);
}

static void test_types_distinct(void)
{
    /* Knight denser than pawn in these silhouettes. */
    ASSERT_TRUE(ui_piece_fill_count(2) > ui_piece_fill_count(1));
    ASSERT_TRUE(ui_piece_fill_count(5) > ui_piece_fill_count(3));
}

int main(void)
{
    test_fill_counts();
    test_draw_spans();
    test_types_distinct();
    return test_report();
}
