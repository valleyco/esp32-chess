#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal_display.h"
#include "lcd_test_panel_rgb565.h"

static const char *TAG = "lcdtest";

void app_main(void)
{
    ESP_LOGI(TAG, "ST7789 blit exact BMP %dx%d", LCD_TEST_PANEL_W,
             LCD_TEST_PANEL_H);

    hal_display_init();

    while (1)
    {
        hal_display_blit_panel_rgb565(lcd_test_panel_rgb565);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
