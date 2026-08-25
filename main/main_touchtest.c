#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal_display.h"
#include "hal_touch.h"

static const char *TAG = "touchtest";

#define C_BLACK 0x0000
#define C_WHITE 0xFFFF
#define C_CYAN 0x07FF
#define C_YELLOW 0xFFE0
#define C_RED 0xF800
#define C_GREEN 0x07E0
#define C_BLUE 0x001F

void app_main(void)
{
    int w = 0;
    int h = 0;

    ESP_LOGI(TAG, "touch paint probe");

    /* Prove LCD alive before touch init (bright field). */
    hal_display_init();
    hal_display_get_size(&w, &h);
    hal_display_fill_rect(0, 0, w, h, C_BLUE);
    hal_display_fill_rect(0, 0, w, 24, C_YELLOW);
    vTaskDelay(pdMS_TO_TICKS(200));

    hal_touch_init();
    ESP_LOGI(TAG, "touch ready — drag finger; UART logs raw ADC");

    /* Corner targets: touch TL should paint near yellow-left, etc. */
    hal_display_fill_rect(0, 0, w, h, C_BLACK);
    hal_display_fill_rect(0, 0, w, 16, C_YELLOW);
    hal_display_fill_rect(0, 20, 24, 24, C_RED);
    hal_display_fill_rect(0, 0, 12, 12, C_WHITE);           /* TL */
    hal_display_fill_rect(w - 12, 0, 12, 12, C_CYAN);       /* TR */
    hal_display_fill_rect(0, h - 12, 12, 12, C_GREEN);      /* BL */
    hal_display_fill_rect(w - 12, h - 12, 12, 12, C_YELLOW); /* BR */

    int64_t last_log_ms = 0;

    while (1)
    {
        uint16_t rx = 0;
        uint16_t ry = 0;
        uint16_t rz = 0;
        int px = 0;
        int py = 0;
        bool pressed = false;
        hal_touch_sample(&rx, &ry, &rz, &px, &py, &pressed);

        const int irq = hal_touch_irq_level();
        hal_display_fill_rect(0, 20, 24, 24, irq == 0 ? C_GREEN : C_RED);

        if (pressed)
        {
            hal_display_fill_rect(px - 2, py - 2, 5, 5, C_CYAN);
        }

        const int64_t now = (int64_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
        if (now - last_log_ms >= 250)
        {
            last_log_ms = now;
            ESP_LOGI(TAG, "irq=%d z=%u raw=(%u,%u) map=(%d,%d) pressed=%d", irq,
                     (unsigned)rz, (unsigned)rx, (unsigned)ry, px, py,
                     (int)pressed);
        }

        vTaskDelay(pdMS_TO_TICKS(15));
    }
}
