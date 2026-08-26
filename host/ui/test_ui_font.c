#include "../test_assert.h"
#include "ui_font.h"

static int s_plots;

static void count_plot(int x, int y, void *user)
{
    (void)x;
    (void)y;
    (void)user;
    s_plots++;
}

static void test_glyph_known(void)
{
    ASSERT_TRUE(ui_font_glyph('N') != NULL);
    ASSERT_TRUE(ui_font_glyph('n') != NULL); /* case-insensitive */
    ASSERT_TRUE(ui_font_glyph('Q') != NULL);
    ASSERT_TRUE(ui_font_glyph('1') != NULL);
}

static void test_text_width(void)
{
    /* scale 2: 3 glyphs * 10 + 2 gaps * 2 = 34 */
    ASSERT_EQ_INT(34, ui_font_text_width("NEW", 2));
    ASSERT_EQ_INT(0, ui_font_text_width("", 2));
    ASSERT_EQ_INT(10, ui_font_text_width("W", 2));
}

static void test_draw_plots(void)
{
    s_plots = 0;
    ui_font_draw("A", 0, 0, 1, count_plot, NULL);
    ASSERT_TRUE(s_plots > 10); /* letter A has multiple pixels */
    const int a_plots = s_plots;
    s_plots = 0;
    ui_font_draw("A", 0, 0, 2, count_plot, NULL);
    ASSERT_EQ_INT(a_plots * 4, s_plots); /* scale 2 → 2x2 per pixel */
}

static int s_span_pixels;
static int s_spans;

static void count_span(int x, int y, int w, void *user)
{
    (void)x;
    (void)y;
    (void)user;
    s_spans++;
    s_span_pixels += w;
}

static void test_draw_spans(void)
{
    s_plots = 0;
    ui_font_draw("NEW", 0, 0, 2, count_plot, NULL);
    const int via_plot = s_plots;

    s_span_pixels = 0;
    s_spans = 0;
    ui_font_draw_spans("NEW", 0, 0, 2, count_span, NULL);
    ASSERT_EQ_INT(via_plot, s_span_pixels);
    ASSERT_TRUE(s_spans > 0);
    ASSERT_TRUE(s_spans < via_plot); /* spans coalesce vs 1×1 plots */
}

int main(void)
{
    test_glyph_known();
    test_text_width();
    test_draw_plots();
    test_draw_spans();
    return test_report();
}
