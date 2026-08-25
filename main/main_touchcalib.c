#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal_display.h"
#include "hal_touch.h"
#include "touch_calib_wizard.h"

static const char *TAG = "touchcalib";

#define C_BLACK 0x0000
#define C_YELLOW 0xFFE0
#define C_GREEN 0x07E0
#define C_MAGENTA 0xF81F

static void paint_test_loop(void)
{
    int w = 0;
    int h = 0;
    hal_display_get_size(&w, &h);
    hal_display_fill_rect(0, 0, w, h, C_BLACK);
    hal_display_fill_rect(0, 0, w, 16, C_GREEN);
    ESP_LOGI(TAG, "draw test — drag to verify");

    while (1)
    {
        int px = 0;
        int py = 0;
        if (hal_touch_poll(&px, &py))
        {
            hal_display_fill_rect(px - 2, py - 2, 5, 5, C_MAGENTA);
        }
        vTaskDelay(pdMS_TO_TICKS(15));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "standalone touch calibration");
    hal_display_init();
    hal_touch_init();
    (void)touch_calib_run_wizard();
    paint_test_loop();
}
