#include "chess_api.h"
#include "chess_ui.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal_display.h"

static const char *TAG = "chess";

void app_main(void)
{
    ESP_LOGI(TAG, "esp32-chess — board UI paint smoke");
    chess_new_game();
    hal_display_init();
    chess_ui_init();
    chess_ui_sync_from_game(-1);
    chess_ui_paint();
    ESP_LOGI(TAG, "initial board painted, ply=%d", chess_ply());

    /* Demo dirty path: e2e4 then paint only changed squares. */
    if (chess_try_move(52, 36, 0))
    {
        chess_ui_sync_from_game(-1);
        chess_ui_paint();
        ESP_LOGI(TAG, "e2e4 dirty paint done");
    }

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
