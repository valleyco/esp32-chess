idf_component_register(
    SRCS "src/cyd_display.c" "src/cyd_touch.c" "src/touch_calib_math.c"
         "src/touch_calib_wizard.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_lcd driver nvs_flash
)
