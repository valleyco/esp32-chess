#include <string.h>
#include "hal_display.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "hal_display";

#define CYD_LCD_HOST SPI2_HOST
#define CYD_PIN_SCLK 14
#define CYD_PIN_MOSI 13
#define CYD_PIN_CS 15
#define CYD_PIN_DC 2
#define CYD_PIN_RST -1
#define CYD_PIN_BL 21
#define CYD_BL_ON 1

/*
 * Dual-USB CYD variants often use ST7789, not ILI9341. Wrong driver →
 * stripe/hail wrap. ST7789 needs color invert; landscape via swap_xy.
 * Verified config matches esp32-invaders on this unit.
 */
#define PANEL_W 320
#define PANEL_H 240
#define CYD_PCLK_HZ (20 * 1000 * 1000)

#define BLIT_ROWS 8
#define STRIP_PIXELS (PANEL_W * BLIT_ROWS)

static esp_lcd_panel_handle_t s_panel;
static SemaphoreHandle_t s_lcd_done;
static uint16_t s_strip[2][STRIP_PIXELS];
static int s_strip_idx;

static bool lcd_on_color_done(esp_lcd_panel_io_handle_t panel_io,
                              esp_lcd_panel_io_event_data_t *edata,
                              void *user_ctx)
{
    (void)panel_io;
    (void)edata;
    (void)user_ctx;
    BaseType_t hpw = pdFALSE;
    xSemaphoreGiveFromISR(s_lcd_done, &hpw);
    return hpw == pdTRUE;
}

static void lcd_wait_color_done(void)
{
    xSemaphoreTake(s_lcd_done, portMAX_DELAY);
}

static void rgb565_byteswap(uint16_t *buf, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        buf[i] = __builtin_bswap16(buf[i]);
    }
}

static void panel_fill_black(void)
{
    static uint16_t line[PANEL_W];
    memset(line, 0, sizeof(line));
    for (int y = 0; y < PANEL_H; y++)
    {
        ESP_ERROR_CHECK(
            esp_lcd_panel_draw_bitmap(s_panel, 0, y, PANEL_W, y + 1, line));
        lcd_wait_color_done();
    }
}

void hal_display_init(void)
{
    ESP_LOGI(TAG, "ST7789 landscape %dx%d swap_xy+mirror_x RGB invert-off",
             PANEL_W, PANEL_H);

    s_lcd_done = xSemaphoreCreateBinary();
    ESP_ERROR_CHECK(s_lcd_done ? ESP_OK : ESP_ERR_NO_MEM);

    gpio_config_t bk = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << CYD_PIN_BL,
    };
    ESP_ERROR_CHECK(gpio_config(&bk));
    gpio_set_level(CYD_PIN_BL, !CYD_BL_ON);

    spi_bus_config_t buscfg = {
        .sclk_io_num = CYD_PIN_SCLK,
        .mosi_io_num = CYD_PIN_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = STRIP_PIXELS * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(CYD_LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = CYD_PIN_CS,
        .dc_gpio_num = CYD_PIN_DC,
        .spi_mode = 0,
        .pclk_hz = CYD_PCLK_HZ,
        .trans_queue_depth = 4,
        .on_color_trans_done = lcd_on_color_done,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)CYD_LCD_HOST,
                                             &io_config, &io));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = CYD_PIN_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io, &panel_config, &s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(s_panel, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(s_panel, true, false));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(s_panel, false));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_panel, true));

    panel_fill_black();

    gpio_set_level(CYD_PIN_BL, CYD_BL_ON);
    ESP_LOGI(TAG, "display ready (ST7789, RGB, invert off)");
}

void hal_display_get_size(int *w, int *h)
{
    if (w)
    {
        *w = PANEL_W;
    }
    if (h)
    {
        *h = PANEL_H;
    }
}

void hal_display_blit_panel_rgb565(const uint16_t *fb)
{
    if (!s_panel || !fb)
    {
        return;
    }

    for (int y0 = 0; y0 < PANEL_H; y0 += BLIT_ROWS)
    {
        int nrows = BLIT_ROWS;
        if (y0 + nrows > PANEL_H)
        {
            nrows = PANEL_H - y0;
        }

        uint16_t *strip = s_strip[s_strip_idx];
        memcpy(strip, fb + y0 * PANEL_W,
               (size_t)nrows * PANEL_W * sizeof(uint16_t));
        rgb565_byteswap(strip, (size_t)nrows * PANEL_W);

        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(s_panel, 0, y0, PANEL_W,
                                                  y0 + nrows, strip));
        lcd_wait_color_done();
        s_strip_idx ^= 1;
    }
}

void hal_display_blit_rows(hal_display_row_fn fn, void *ctx)
{
    if (!s_panel || !fn)
    {
        return;
    }

    for (int y0 = 0; y0 < PANEL_H; y0 += BLIT_ROWS)
    {
        int nrows = BLIT_ROWS;
        if (y0 + nrows > PANEL_H)
        {
            nrows = PANEL_H - y0;
        }

        uint16_t *strip = s_strip[s_strip_idx];
        for (int r = 0; r < nrows; r++)
        {
            fn(y0 + r, strip + r * PANEL_W, PANEL_W, ctx);
        }
        rgb565_byteswap(strip, (size_t)nrows * PANEL_W);

        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(s_panel, 0, y0, PANEL_W,
                                                  y0 + nrows, strip));
        lcd_wait_color_done();
        s_strip_idx ^= 1;
    }
}

void hal_display_fill_rect(int x, int y, int w, int h, uint16_t rgb565)
{
    if (!s_panel || w <= 0 || h <= 0)
    {
        return;
    }

    if (x < 0)
    {
        w += x;
        x = 0;
    }
    if (y < 0)
    {
        h += y;
        y = 0;
    }
    if (x + w > PANEL_W)
    {
        w = PANEL_W - x;
    }
    if (y + h > PANEL_H)
    {
        h = PANEL_H - y;
    }
    if (w <= 0 || h <= 0)
    {
        return;
    }

    uint16_t pix = rgb565;
    rgb565_byteswap(&pix, 1);

    /* Reuse one strip row as a solid color line (max PANEL_W). */
    uint16_t *line = s_strip[s_strip_idx];
    for (int i = 0; i < w; i++)
    {
        line[i] = pix;
    }

    for (int row = y; row < y + h; row++)
    {
        ESP_ERROR_CHECK(
            esp_lcd_panel_draw_bitmap(s_panel, x, row, x + w, row + 1, line));
        lcd_wait_color_done();
    }
    s_strip_idx ^= 1;
}
