#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <nvs.h>
#include <nvs_flash.h>
#include "hal_touch.h"
#include "touch_calib.h"

static const char *TAG = "hal_touch";

/* CYD XPT2046 on a bus separate from the LCD (LCD = SPI2). */
#define TOUCH_HOST SPI3_HOST
#define TOUCH_PIN_SCLK 25
#define TOUCH_PIN_MOSI 32
#define TOUCH_PIN_MISO 39
#define TOUCH_PIN_CS 33
#define TOUCH_PIN_IRQ 36
#define TOUCH_HZ (1 * 1000 * 1000)

#define PANEL_W TOUCH_CALIB_PANEL_W
#define PANEL_H TOUCH_CALIB_PANEL_H

#define CMD_X 0x90
#define CMD_Y 0xD0
#define CMD_Z1 0xB0

#define NVS_NS "touch"
#define NVS_KEY "calib"

static spi_device_handle_t s_touch;
static bool s_irq_ok;
static touch_calib_t s_calib;
static bool s_nvs_ready;
static bool s_calib_from_nvs;

static uint16_t xpt_read12(uint8_t cmd)
{
    spi_transaction_t t = {
        .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
        .length = 24,
    };
    t.tx_data[0] = cmd;
    t.tx_data[1] = 0;
    t.tx_data[2] = 0;
    if (spi_device_polling_transmit(s_touch, &t) != ESP_OK)
    {
        return 0;
    }
    return (uint16_t)(((t.rx_data[1] << 8) | t.rx_data[2]) >> 3);
}

static void ensure_nvs(void)
{
    if (s_nvs_ready)
    {
        return;
    }
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "nvs_flash_init failed: %s", esp_err_to_name(err));
        return;
    }
    s_nvs_ready = true;
}

void hal_touch_set_calib(const touch_calib_t *c)
{
    if (!c || !touch_calib_valid(c))
    {
        return;
    }
    s_calib = *c;
}

void hal_touch_get_calib(touch_calib_t *c)
{
    if (c)
    {
        *c = s_calib;
    }
}

bool hal_touch_load_nvs(void)
{
    ensure_nvs();
    if (!s_nvs_ready)
    {
        return false;
    }

    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NS, NVS_READONLY, &h);
    if (err != ESP_OK)
    {
        return false;
    }

    touch_calib_t c;
    size_t len = sizeof(c);
    err = nvs_get_blob(h, NVS_KEY, &c, &len);
    nvs_close(h);
    if (err != ESP_OK || len != sizeof(c) || !touch_calib_valid(&c))
    {
        return false;
    }

    s_calib = c;
    s_calib_from_nvs = true;
    ESP_LOGI(TAG, "calib from NVS x=[%d,%d] y=[%d,%d] swap=%u z=%u",
             (int)s_calib.x_min, (int)s_calib.x_max, (int)s_calib.y_min,
             (int)s_calib.y_max, (unsigned)s_calib.swap_xy,
             (unsigned)s_calib.z_threshold);
    return true;
}

bool hal_touch_calib_from_nvs(void)
{
    return s_calib_from_nvs;
}

bool hal_touch_save_nvs(void)
{
    if (!touch_calib_valid(&s_calib))
    {
        return false;
    }
    ensure_nvs();
    if (!s_nvs_ready)
    {
        return false;
    }

    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NS, NVS_READWRITE, &h);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "nvs_open: %s", esp_err_to_name(err));
        return false;
    }
    err = nvs_set_blob(h, NVS_KEY, &s_calib, sizeof(s_calib));
    if (err == ESP_OK)
    {
        err = nvs_commit(h);
    }
    nvs_close(h);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "nvs save: %s", esp_err_to_name(err));
        return false;
    }
    s_calib_from_nvs = true;
    ESP_LOGI(TAG, "calib saved to NVS");
    return true;
}

void hal_touch_init(void)
{
    touch_calib_set_defaults(&s_calib);
    s_calib_from_nvs = false;

    /*
     * GPIO36 is input-only and has NO internal pulls on classic ESP32.
     * Configuring pull-up here aborts with ESP_ERR_INVALID_ARG.
     */
    gpio_config_t irq = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1ULL << TOUCH_PIN_IRQ,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    s_irq_ok = (gpio_config(&irq) == ESP_OK);

    spi_bus_config_t buscfg = {
        .mosi_io_num = TOUCH_PIN_MOSI,
        .miso_io_num = TOUCH_PIN_MISO,
        .sclk_io_num = TOUCH_PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(TOUCH_HOST, &buscfg, SPI_DMA_DISABLED));

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = TOUCH_HZ,
        .mode = 0,
        .spics_io_num = TOUCH_PIN_CS,
        .queue_size = 1,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(TOUCH_HOST, &devcfg, &s_touch));

    if (!hal_touch_load_nvs())
    {
        ESP_LOGI(TAG, "using factory calib defaults");
    }

    ESP_LOGI(TAG, "XPT2046 SPI3 CS=%d IRQ=%d ok=%d @ %d Hz", TOUCH_PIN_CS,
             TOUCH_PIN_IRQ, (int)s_irq_ok, TOUCH_HZ);
}

int hal_touch_irq_level(void)
{
    if (!s_irq_ok)
    {
        return 1;
    }
    return gpio_get_level(TOUCH_PIN_IRQ);
}

bool hal_touch_sample(uint16_t *raw_x, uint16_t *raw_y, uint16_t *raw_z,
                      int *px, int *py, bool *pressed)
{
    if (!s_touch)
    {
        return false;
    }

    const uint16_t z1 = xpt_read12(CMD_Z1);
    uint32_t sx = 0;
    uint32_t sy = 0;
    for (int i = 0; i < 4; i++)
    {
        sx += xpt_read12(CMD_X);
        sy += xpt_read12(CMD_Y);
    }
    sx /= 4;
    sy /= 4;

    if (raw_x)
    {
        *raw_x = (uint16_t)sx;
    }
    if (raw_y)
    {
        *raw_y = (uint16_t)sy;
    }
    if (raw_z)
    {
        *raw_z = z1;
    }

    const int irq = hal_touch_irq_level();
    const bool down =
        (z1 >= s_calib.z_threshold) || (s_irq_ok && irq == 0);

    if (pressed)
    {
        *pressed = down;
    }

    int lx = 0;
    int ly = 0;
    touch_calib_map(&s_calib, (int)sx, (int)sy, PANEL_W, PANEL_H, &lx, &ly);

    if (px)
    {
        *px = lx;
    }
    if (py)
    {
        *py = ly;
    }
    return true;
}

bool hal_touch_poll(int *x, int *y)
{
    uint16_t rx = 0;
    uint16_t ry = 0;
    uint16_t rz = 0;
    int px = 0;
    int py = 0;
    bool pressed = false;
    if (!hal_touch_sample(&rx, &ry, &rz, &px, &py, &pressed) || !pressed)
    {
        return false;
    }
    if (x)
    {
        *x = px;
    }
    if (y)
    {
        *y = py;
    }
    return true;
}
