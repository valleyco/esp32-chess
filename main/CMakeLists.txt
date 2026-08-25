set(MAIN_SRCS "main.c")
set(MAIN_INCLUDE_DIRS ".")

if(LCD_TEST AND TOUCH_TEST)
    message(FATAL_ERROR "Set only one of LCD_TEST, TOUCH_TEST, or TOUCH_CALIB")
elseif(LCD_TEST AND TOUCH_CALIB)
    message(FATAL_ERROR "Set only one of LCD_TEST, TOUCH_TEST, or TOUCH_CALIB")
elseif(TOUCH_TEST AND TOUCH_CALIB)
    message(FATAL_ERROR "Set only one of LCD_TEST, TOUCH_TEST, or TOUCH_CALIB")
elseif(LCD_TEST)
    set(MAIN_SRCS "main_lcdtest.c")
    list(APPEND MAIN_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/host/lcdtest/generated")
    message(STATUS "LCD_TEST=1 — building orientation test card firmware")
elseif(TOUCH_TEST)
    set(MAIN_SRCS "main_touchtest.c")
    message(STATUS "TOUCH_TEST=1 — building touch finger-paint probe firmware")
elseif(TOUCH_CALIB)
    set(MAIN_SRCS "main_touchcalib.c")
    message(STATUS "TOUCH_CALIB=1 — building 4-corner calibration wizard")
endif()

idf_component_register(
    SRCS ${MAIN_SRCS}
    INCLUDE_DIRS ${MAIN_INCLUDE_DIRS}
    PRIV_REQUIRES board chess ui esp_timer
)
