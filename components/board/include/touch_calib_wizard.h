#pragma once
#include <stdbool.h>
#include "touch_calib.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Blocking 4-corner calibration wizard (uses display + touch).
 * Saves to NVS on success. Returns true if a valid calib was applied.
 */
bool touch_calib_run_wizard(void);

#ifdef __cplusplus
}
#endif
