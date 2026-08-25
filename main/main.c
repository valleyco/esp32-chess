#include "chess_api.h"
#include "chess_fsm.h"
#include "chess_geom.h"
#include "chess_ui.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal_display.h"
#include "hal_touch.h"

static const char *TAG = "chess";

enum
{
    THINK_MS = 3000,
    THINK_STACK = 24576,
};

static TaskHandle_t s_think_task;
static volatile bool s_think_done;
static volatile bool s_think_ok;
static bool s_thinking;
static chess_fsm_t s_fsm;
static bool s_was_pressed;

static bool from_selectable(int sq)
{
    if (s_thinking || !chess_side_to_move())
    {
        return false;
    }
    return chess_get_square(sq) > 0; /* white piece */
}

static void request_think(void)
{
    s_thinking = true;
    s_think_done = false;
    chess_ui_set_busy(true);
    chess_ui_paint();
    xTaskNotifyGive(s_think_task);
}

static void think_task(void *arg)
{
    (void)arg;
    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        ESP_LOGI(TAG, "engine thinking (%d ms)...", THINK_MS);
        s_think_ok = chess_think(THINK_MS);
        s_think_done = true;
    }
}

static void on_board_tap(int sq)
{
    const chess_fsm_event_t ev =
        chess_fsm_tap_square(&s_fsm, sq, from_selectable(sq));

    switch (ev)
    {
    case CHESS_FSM_SELECT:
        chess_ui_sync_from_game(s_fsm.selected);
        chess_ui_paint();
        break;
    case CHESS_FSM_CANCEL:
        chess_ui_sync_from_game(-1);
        chess_ui_paint();
        break;
    case CHESS_FSM_MOVE:
        if (chess_try_move(s_fsm.c1, s_fsm.c2, 0))
        {
            ESP_LOGI(TAG, "human %d→%d", s_fsm.c1, s_fsm.c2);
            chess_ui_sync_from_game(-1);
            chess_ui_paint();
            request_think();
        }
        else
        {
            ESP_LOGI(TAG, "illegal %d→%d", s_fsm.c1, s_fsm.c2);
            chess_fsm_init(&s_fsm);
            chess_ui_sync_from_game(-1);
            chess_ui_paint();
        }
        break;
    default:
        break;
    }
}

static void handle_touch_edge(void)
{
    int px = 0;
    int py = 0;
    bool pressed = false;
    hal_touch_sample(NULL, NULL, NULL, &px, &py, &pressed);

    if (pressed && !s_was_pressed && !s_thinking)
    {
        const int sq = chess_geom_panel_to_square(px, py);
        if (sq >= 0)
        {
            on_board_tap(sq);
        }
    }
    s_was_pressed = pressed;
}

static void handle_think_done(void)
{
    if (!s_think_done)
    {
        return;
    }
    s_think_done = false;
    s_thinking = false;
    chess_ui_set_busy(false);
    if (s_think_ok)
    {
        ESP_LOGI(TAG, "engine moved, ply=%d", chess_ply());
    }
    else
    {
        ESP_LOGW(TAG, "engine found no move");
    }
    chess_ui_sync_from_game(-1);
    chess_ui_paint();
}

void app_main(void)
{
    ESP_LOGI(TAG, "esp32-chess — touch play vs engine");

    chess_new_game();
    chess_fsm_init(&s_fsm);

    hal_display_init();
    hal_touch_init();

    chess_ui_init();
    chess_ui_sync_from_game(-1);
    chess_ui_paint();

    BaseType_t ok = xTaskCreatePinnedToCore(think_task, "think", THINK_STACK,
                                            NULL, 5, &s_think_task, 1);
    if (ok != pdPASS)
    {
        ESP_LOGE(TAG, "think task create failed");
        return;
    }

    ESP_LOGI(TAG, "ready — play White; strip yellow while thinking");

    while (1)
    {
        handle_think_done();
        handle_touch_edge();
        vTaskDelay(pdMS_TO_TICKS(15));
    }
}
