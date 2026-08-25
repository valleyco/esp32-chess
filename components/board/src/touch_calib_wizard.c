#include "touch_calib_wizard.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal_display.h"
#include "hal_touch.h"
#include "touch_calib.h"

static const char *TAG = "calib";

#define C_BLACK 0x0000
#define C_WHITE 0xFFFF
#define C_CYAN 0x07FF
#define C_YELLOW 0xFFE0
#define C_RED 0xF800
#define C_GREEN 0x07E0
#define C_MAGENTA 0xF81F

#define INSET 16
#define AVG_SAMPLES 8
#define DEBOUNCE_MS 40

static void fill_screen(uint16_t c)
{
    int w = 0;
    int h = 0;
    hal_display_get_size(&w, &h);
    hal_display_fill_rect(0, 0, w, h, c);
}

static void draw_target(int x, int y, uint16_t color)
{
    hal_display_fill_rect(x - 6, y - 1, 13, 3, color);
    hal_display_fill_rect(x - 1, y - 6, 3, 13, color);
    hal_display_fill_rect(x - 2, y - 2, 5, 5, C_WHITE);
}

static void wait_release(void)
{
    for (;;)
    {
        bool pressed = false;
        hal_touch_sample(NULL, NULL, NULL, NULL, NULL, &pressed);
        if (!pressed)
        {
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(15));
    }
}

static bool capture_corner(int tx, int ty, touch_raw_pt_t *out)
{
    fill_screen(C_BLACK);
    hal_display_fill_rect(0, 0, 320, 16, C_YELLOW);
    draw_target(tx, ty, C_RED);
    ESP_LOGI(TAG, "touch target (%d,%d)", tx, ty);
    wait_release();

    for (;;)
    {
        uint16_t rx = 0;
        uint16_t ry = 0;
        uint16_t rz = 0;
        int px = 0;
        int py = 0;
        bool pressed = false;
        hal_touch_sample(&rx, &ry, &rz, &px, &py, &pressed);
        if (!pressed)
        {
            vTaskDelay(pdMS_TO_TICKS(15));
            continue;
        }

        uint32_t sx = 0;
        uint32_t sy = 0;
        int n = 0;
        while (n < AVG_SAMPLES)
        {
            pressed = false;
            hal_touch_sample(&rx, &ry, &rz, &px, &py, &pressed);
            if (!pressed)
            {
                break;
            }
            sx += rx;
            sy += ry;
            n++;
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        if (n < AVG_SAMPLES / 2)
        {
            continue;
        }
        out->x = (uint16_t)(sx / (uint32_t)n);
        out->y = (uint16_t)(sy / (uint32_t)n);
        wait_release();
        return true;
    }
}

bool touch_calib_run_wizard(void)
{
    const int w = TOUCH_CALIB_PANEL_W;
    const int h = TOUCH_CALIB_PANEL_H;
    touch_raw_pt_t tl, tr, bl, br;

    ESP_LOGI(TAG, "4-corner calibration wizard");
    capture_corner(INSET, INSET, &tl);
    capture_corner(w - 1 - INSET, INSET, &tr);
    capture_corner(INSET, h - 1 - INSET, &bl);
    capture_corner(w - 1 - INSET, h - 1 - INSET, &br);

    touch_calib_t c;
    if (!touch_calib_from_corners(tl, tr, bl, br, w, h, INSET, &c))
    {
        fill_screen(C_RED);
        ESP_LOGE(TAG, "calib rejected");
        vTaskDelay(pdMS_TO_TICKS(800));
        return false;
    }

    hal_touch_set_calib(&c);
    const bool saved = hal_touch_save_nvs();
    fill_screen(saved ? C_GREEN : C_YELLOW);
    ESP_LOGI(TAG, "calib %s x=[%d,%d] y=[%d,%d] swap=%u",
             saved ? "saved" : "active (NVS fail)", (int)c.x_min, (int)c.x_max,
             (int)c.y_min, (int)c.y_max, (unsigned)c.swap_xy);
    vTaskDelay(pdMS_TO_TICKS(600));
    return true;
}
