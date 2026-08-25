#include "../test_assert.h"
#include "touch_calib.h"

static void test_defaults_valid(void)
{
    touch_calib_t c;
    touch_calib_set_defaults(&c);
    ASSERT_TRUE(touch_calib_valid(&c));
    ASSERT_EQ_INT(200, c.x_min);
    ASSERT_EQ_INT(3800, c.x_max);
    ASSERT_EQ_INT(TOUCH_CALIB_DEFAULT_Z, (int)c.z_threshold);
    ASSERT_EQ_INT(0, (int)c.swap_xy);
}

static void test_defaults_map_midpoint(void)
{
    touch_calib_t c;
    touch_calib_set_defaults(&c);
    int px = -1;
    int py = -1;
    /* Mid of 200..3800 ≈ 2000 → near panel center. */
    touch_calib_map(&c, 2000, 2000, TOUCH_CALIB_PANEL_W, TOUCH_CALIB_PANEL_H,
                    &px, &py);
    ASSERT_TRUE(px > 140 && px < 180);
    ASSERT_TRUE(py > 100 && py < 140);
}

static void test_from_corners_ideal_no_swap(void)
{
    /*
     * Ideal: raw grows with panel x and y. Targets at inset=16.
     * TL(16,16)→(400,500), TR(303,16)→(3600,500),
     * BL(16,223)→(400,3500), BR(303,223)→(3600,3500).
     */
    const int inset = 16;
    touch_raw_pt_t tl = {.x = 400, .y = 500};
    touch_raw_pt_t tr = {.x = 3600, .y = 500};
    touch_raw_pt_t bl = {.x = 400, .y = 3500};
    touch_raw_pt_t br = {.x = 3600, .y = 3500};
    touch_calib_t c;
    ASSERT_TRUE(touch_calib_from_corners(tl, tr, bl, br, TOUCH_CALIB_PANEL_W,
                                         TOUCH_CALIB_PANEL_H, inset, &c));
    ASSERT_EQ_INT(0, (int)c.swap_xy);
    ASSERT_TRUE(touch_calib_valid(&c));

    int px = 0;
    int py = 0;
    touch_calib_map(&c, 400, 500, TOUCH_CALIB_PANEL_W, TOUCH_CALIB_PANEL_H, &px,
                    &py);
    ASSERT_TRUE(px >= inset - 2 && px <= inset + 2);
    ASSERT_TRUE(py >= inset - 2 && py <= inset + 2);

    touch_calib_map(&c, 3600, 3500, TOUCH_CALIB_PANEL_W, TOUCH_CALIB_PANEL_H,
                    &px, &py);
    ASSERT_TRUE(px >= TOUCH_CALIB_PANEL_W - 1 - inset - 2 &&
                px <= TOUCH_CALIB_PANEL_W - 1 - inset + 2);
    ASSERT_TRUE(py >= TOUCH_CALIB_PANEL_H - 1 - inset - 2 &&
                py <= TOUCH_CALIB_PANEL_H - 1 - inset + 2);
}

static void test_from_corners_detects_swap(void)
{
    /* TL→TR changes raw_y more than raw_x → swap_xy. */
    const int inset = 16;
    touch_raw_pt_t tl = {.x = 500, .y = 400};
    touch_raw_pt_t tr = {.x = 500, .y = 3600};
    touch_raw_pt_t bl = {.x = 3500, .y = 400};
    touch_raw_pt_t br = {.x = 3500, .y = 3600};
    touch_calib_t c;
    ASSERT_TRUE(touch_calib_from_corners(tl, tr, bl, br, TOUCH_CALIB_PANEL_W,
                                         TOUCH_CALIB_PANEL_H, inset, &c));
    ASSERT_EQ_INT(1, (int)c.swap_xy);

    int px = 0;
    int py = 0;
    touch_calib_map(&c, 500, 400, TOUCH_CALIB_PANEL_W, TOUCH_CALIB_PANEL_H, &px,
                    &py);
    ASSERT_TRUE(px >= inset - 2 && px <= inset + 2);
    ASSERT_TRUE(py >= inset - 2 && py <= inset + 2);
}

static void test_from_corners_mirror_x(void)
{
    /* Raw X decreases left→right (mirrored). */
    const int inset = 16;
    touch_raw_pt_t tl = {.x = 3600, .y = 500};
    touch_raw_pt_t tr = {.x = 400, .y = 500};
    touch_raw_pt_t bl = {.x = 3600, .y = 3500};
    touch_raw_pt_t br = {.x = 400, .y = 3500};
    touch_calib_t c;
    ASSERT_TRUE(touch_calib_from_corners(tl, tr, bl, br, TOUCH_CALIB_PANEL_W,
                                         TOUCH_CALIB_PANEL_H, inset, &c));
    ASSERT_EQ_INT(0, (int)c.swap_xy);

    int px = 0;
    int py = 0;
    touch_calib_map(&c, 3600, 500, TOUCH_CALIB_PANEL_W, TOUCH_CALIB_PANEL_H,
                    &px, &py);
    ASSERT_TRUE(px >= inset - 2 && px <= inset + 2);
    ASSERT_TRUE(py >= inset - 2 && py <= inset + 2);

    touch_calib_map(&c, 400, 500, TOUCH_CALIB_PANEL_W, TOUCH_CALIB_PANEL_H, &px,
                    &py);
    ASSERT_TRUE(px >= TOUCH_CALIB_PANEL_W - 1 - inset - 2 &&
                px <= TOUCH_CALIB_PANEL_W - 1 - inset + 2);
}

static void test_rejects_degenerate(void)
{
    touch_raw_pt_t p = {.x = 1000, .y = 1000};
    touch_calib_t c;
    ASSERT_TRUE(!touch_calib_from_corners(p, p, p, p, TOUCH_CALIB_PANEL_W,
                                          TOUCH_CALIB_PANEL_H, 16, &c));
}

int main(void)
{
    test_defaults_valid();
    test_defaults_map_midpoint();
    test_from_corners_ideal_no_swap();
    test_from_corners_detects_swap();
    test_from_corners_mirror_x();
    test_rejects_degenerate();
    return test_report();
}
