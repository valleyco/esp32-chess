#include "touch_calib.h"

static int iabs(int v)
{
    return v < 0 ? -v : v;
}

static int clamp_int(int v, int lo, int hi)
{
    if (v < lo)
    {
        return lo;
    }
    if (v > hi)
    {
        return hi;
    }
    return v;
}

/* in_a → out_min, in_b → out_max (in_a may be greater than in_b when mirrored). */
static int map_range(int v, int in_a, int in_b, int out_min, int out_max)
{
    if (in_a == in_b)
    {
        return out_min;
    }
    const int in_lo = in_a < in_b ? in_a : in_b;
    const int in_hi = in_a < in_b ? in_b : in_a;
    if (v < in_lo)
    {
        v = in_lo;
    }
    if (v > in_hi)
    {
        v = in_hi;
    }
    return out_min + (v - in_a) * (out_max - out_min) / (in_b - in_a);
}

/** Extrapolate raw at panel edge 0 from raw_at_inset (panel=inset). */
static int extrapolate_edge(int raw_near, int raw_far, int inset, int span)
{
    /* raw_near maps to inset, raw_far to span-1-inset. */
    const int usable = span - 1 - 2 * inset;
    if (usable <= 0)
    {
        return raw_near;
    }
    return raw_near + (raw_near - raw_far) * inset / usable;
}

void touch_calib_set_defaults(touch_calib_t *c)
{
    if (!c)
    {
        return;
    }
    c->x_min = 200;
    c->x_max = 3800;
    c->y_min = 200;
    c->y_max = 3800;
    c->swap_xy = 0;
    c->z_threshold = TOUCH_CALIB_DEFAULT_Z;
}

bool touch_calib_valid(const touch_calib_t *c)
{
    if (!c)
    {
        return false;
    }
    if (iabs((int)c->x_max - (int)c->x_min) < TOUCH_CALIB_MIN_SPAN)
    {
        return false;
    }
    if (iabs((int)c->y_max - (int)c->y_min) < TOUCH_CALIB_MIN_SPAN)
    {
        return false;
    }
    return true;
}

bool touch_calib_from_corners(touch_raw_pt_t tl, touch_raw_pt_t tr,
                              touch_raw_pt_t bl, touch_raw_pt_t br, int panel_w,
                              int panel_h, int inset, touch_calib_t *out)
{
    if (!out || panel_w < 8 || panel_h < 8 || inset < 0 ||
        inset * 2 >= panel_w - 1 || inset * 2 >= panel_h - 1)
    {
        return false;
    }

    const int dtr_x = (int)tr.x - (int)tl.x;
    const int dtr_y = (int)tr.y - (int)tl.y;
    const uint8_t swap = (uint8_t)(iabs(dtr_y) > iabs(dtr_x));

    /* Axis A follows panel X; axis B follows panel Y (after optional swap). */
    int tl_a = swap ? (int)tl.y : (int)tl.x;
    int tl_b = swap ? (int)tl.x : (int)tl.y;
    int tr_a = swap ? (int)tr.y : (int)tr.x;
    int tr_b = swap ? (int)tr.x : (int)tr.y;
    int bl_a = swap ? (int)bl.y : (int)bl.x;
    int bl_b = swap ? (int)bl.x : (int)bl.y;
    int br_a = swap ? (int)br.y : (int)br.x;
    int br_b = swap ? (int)br.x : (int)br.y;

    const int left = (tl_a + bl_a) / 2;
    const int right = (tr_a + br_a) / 2;
    const int top = (tl_b + tr_b) / 2;
    const int bottom = (bl_b + br_b) / 2;

    const int x_min = extrapolate_edge(left, right, inset, panel_w);
    const int x_max = extrapolate_edge(right, left, inset, panel_w);
    const int y_min = extrapolate_edge(top, bottom, inset, panel_h);
    const int y_max = extrapolate_edge(bottom, top, inset, panel_h);

    out->x_min = (int16_t)x_min;
    out->x_max = (int16_t)x_max;
    out->y_min = (int16_t)y_min;
    out->y_max = (int16_t)y_max;
    out->swap_xy = swap;
    out->z_threshold = TOUCH_CALIB_DEFAULT_Z;

    return touch_calib_valid(out);
}

void touch_calib_map(const touch_calib_t *c, int raw_x, int raw_y, int panel_w,
                     int panel_h, int *px, int *py)
{
    if (!c || panel_w <= 0 || panel_h <= 0)
    {
        if (px)
        {
            *px = 0;
        }
        if (py)
        {
            *py = 0;
        }
        return;
    }

    int a = c->swap_xy ? raw_y : raw_x;
    int b = c->swap_xy ? raw_x : raw_y;

    int lx = map_range(a, c->x_min, c->x_max, 0, panel_w - 1);
    int ly = map_range(b, c->y_min, c->y_max, 0, panel_h - 1);
    lx = clamp_int(lx, 0, panel_w - 1);
    ly = clamp_int(ly, 0, panel_h - 1);

    if (px)
    {
        *px = lx;
    }
    if (py)
    {
        *py = ly;
    }
}
