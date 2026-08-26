#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

void hal_display_init(void);

/** Current programmed panel size (controller coords). */
void hal_display_get_size(int *w, int *h);

/** Blit full panel RGB565, row-major, top-left origin. */
void hal_display_blit_panel_rgb565(const uint16_t *fb);

/**
 * Generate and blit one frame row-by-row (DRAM-friendly).
 * Callback fills `row` with `width` RGB565 pixels for scanline `y`.
 */
typedef void (*hal_display_row_fn)(int y, uint16_t *row, int width, void *ctx);
void hal_display_blit_rows(hal_display_row_fn fn, void *ctx);

/** Fill a rectangle (clipped). rgb565 is host endian; HAL byteswaps for SPI. */
void hal_display_fill_rect(int x, int y, int w, int h, uint16_t rgb565);

/**
 * Blit a w×h RGB565 image (host endian, row-major) at (x,y). Clipped.
 * Prefer this over many fill_rect calls when the region is already composed.
 */
void hal_display_blit_rgb565(int x, int y, int w, int h, const uint16_t *rgb565);

#ifdef __cplusplus
}
#endif
