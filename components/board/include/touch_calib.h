#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** Raw ADC sample at one panel corner (12-bit class values). */
typedef struct
{
    uint16_t x;
    uint16_t y;
} touch_raw_pt_t;

/**
 * Axis-aligned raw→panel map.
 * After optional swap_xy, raw (x_min,y_min) maps to panel (0,0) and
 * (x_max,y_max) to (panel_w-1, panel_h-1). Mirroring is baked into min/max.
 */
typedef struct
{
    int16_t x_min;
    int16_t x_max;
    int16_t y_min;
    int16_t y_max;
    uint8_t swap_xy;
    uint16_t z_threshold;
} touch_calib_t;

enum
{
    TOUCH_CALIB_PANEL_W = 320,
    TOUCH_CALIB_PANEL_H = 240,
    TOUCH_CALIB_DEFAULT_Z = 80,
    TOUCH_CALIB_MIN_SPAN = 100,
};

void touch_calib_set_defaults(touch_calib_t *c);

/** False if spans too small or pointers null. */
bool touch_calib_valid(const touch_calib_t *c);

/**
 * Build calib from four corner raw samples in order TL, TR, BL, BR.
 * Samples were taken at inset pixels from each panel edge
 * (e.g. inset=16 → TL target is (16,16)). Extrapolates to full 0..(w-1).
 * Detects axis swap from TL→TR raw delta. Returns false if invalid.
 */
bool touch_calib_from_corners(touch_raw_pt_t tl, touch_raw_pt_t tr,
                              touch_raw_pt_t bl, touch_raw_pt_t br, int panel_w,
                              int panel_h, int inset, touch_calib_t *out);

/** Map raw ADC to panel coords using calib (clamped). */
void touch_calib_map(const touch_calib_t *c, int raw_x, int raw_y, int panel_w,
                     int panel_h, int *px, int *py);

#ifdef __cplusplus
}
#endif
