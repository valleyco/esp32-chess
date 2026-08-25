#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "touch_calib.h"

#ifdef __cplusplus
extern "C"
{
#endif

void hal_touch_init(void);

/**
 * Poll XPT2046. On touch, writes panel coords in the same space as the LCD
 * (landscape 320×240, top-left origin) and returns true.
 */
bool hal_touch_poll(int *x, int *y);

/**
 * Always samples ADC. Sets *pressed from Z (and optional IRQ).
 * raw_* are 12-bit ADC; px/py are mapped panel coords (valid if pressed).
 */
bool hal_touch_sample(uint16_t *raw_x, uint16_t *raw_y, uint16_t *raw_z,
                      int *px, int *py, bool *pressed);

/** PENIRQ GPIO36: 0 often means pen down (active low). */
int hal_touch_irq_level(void);

void hal_touch_set_calib(const touch_calib_t *c);
void hal_touch_get_calib(touch_calib_t *c);

/** Load from NVS namespace "touch". False if missing/corrupt (defaults kept). */
bool hal_touch_load_nvs(void);

/** Persist current calib to NVS. False on error. */
bool hal_touch_save_nvs(void);

#ifdef __cplusplus
}
#endif
