/* SPDX-License-Identifier: GPL-3.0-only */
#include "chess_api.h"
#include "chess_fsm.h"
#include "chess_geom.h"
#include "chess_ui.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal_display.h"
#include "hal_touch.h"
#include "touch_calib_wizard.h"

static const char *TAG = "chess";

enum
{
    THINK_STACK = 24576,
};

static const unsigned k_think_opts[] = {1000, 3000, 5000};
static unsigned s_think_ms = 3000;
static int s_think_opt = 1;

static TaskHandle_t s_think_task;
static volatile bool s_think_done;
static volatile bool s_think_ok;
static bool s_thinking;
static bool s_game_over;
static bool s_promo;
static int s_promo_c1 = -1;
static int s_promo_c2 = -1;
static chess_fsm_t s_fsm;
static bool s_was_pressed;
static bool s_new_armed;

static void clear_new_armed(void)
{
    if (!s_new_armed)
    {
        return;
    }
    s_new_armed = false;
    chess_ui_set_new_armed(false);
    chess_ui_paint();
}

static bool from_selectable(int sq)
{
    if (s_thinking || s_game_over || s_promo || !chess_side_to_move())
    {
        return false;
    }
    return chess_get_square(sq) > 0;
}

static void refresh_board(int highlight)
{
    chess_ui_sync_from_game(highlight);
    chess_ui_paint();
}

static void apply_status_after_move(void)
{
    const chess_status_t st = chess_status();
    if (st == CHESS_STATUS_OK)
    {
        return;
    }
    s_game_over = true;
    chess_ui_set_status(st);
    chess_ui_set_mode(UI_MODE_OVER);
    ESP_LOGI(TAG, "%s",
             st == CHESS_STATUS_CHECKMATE ? "checkmate" : "stalemate");
    refresh_board(-1);
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
        ESP_LOGI(TAG, "engine thinking (%u ms)...", s_think_ms);
        chess_search_result_t r = {0};
        const int64_t t0 = esp_timer_get_time();
        s_think_ok = chess_think_time(s_think_ms, &r);
        const int64_t us = esp_timer_get_time() - t0;
        const double ms = (double)us / 1000.0;
        const double nps = (ms > 0.0) ? (1000.0 * (double)r.nodes / ms) : 0.0;
        if (s_think_ok)
        {
            ESP_LOGI(TAG,
                     "engine done depth=%d nodes=%lu score=%d (%.0f ms, %.0f nps)",
                     r.depth, (unsigned long)r.nodes, r.score, ms, nps);
        }
        else
        {
            ESP_LOGW(TAG, "engine think failed (%.0f ms)", ms);
        }
        s_think_done = true;
    }
}

static void do_new_game(void)
{
    s_new_armed = false;
    chess_ui_set_new_armed(false);
    chess_new_game();
    chess_fsm_init(&s_fsm);
    s_promo = false;
    s_game_over = false;
    s_thinking = false;
    chess_ui_set_busy(false);
    chess_ui_set_status(CHESS_STATUS_OK);
    chess_ui_set_mode(UI_MODE_PLAY);
    chess_ui_invalidate_all();
    refresh_board(-1);
    ESP_LOGI(TAG, "new game");
}

static void do_undo(void)
{
    clear_new_armed();
    if (s_thinking || s_promo)
    {
        return;
    }
    if (!chess_undo())
    {
        ESP_LOGI(TAG, "undo unavailable");
        return;
    }
    chess_fsm_init(&s_fsm);
    s_game_over = false;
    chess_ui_set_status(CHESS_STATUS_OK);
    chess_ui_set_mode(UI_MODE_PLAY);
    chess_ui_invalidate_all();
    refresh_board(-1);
    ESP_LOGI(TAG, "undo, ply=%d", chess_ply());
}

static void do_calib(void)
{
    clear_new_armed();
    if (s_thinking)
    {
        return;
    }
    ESP_LOGI(TAG, "in-game calibration");
    (void)touch_calib_run_wizard();
    chess_ui_invalidate_all();
    chess_ui_set_mode(s_game_over ? UI_MODE_OVER
                                  : (s_promo ? UI_MODE_PROMO : UI_MODE_PLAY));
    refresh_board(s_fsm.selected);
}

static void cycle_think_time(void)
{
    clear_new_armed();
    s_think_opt = (s_think_opt + 1) % 3;
    s_think_ms = k_think_opts[s_think_opt];
    chess_ui_set_think_ms(s_think_ms);
    chess_ui_paint();
    ESP_LOGI(TAG, "think time = %u ms", s_think_ms);
}

static void finish_human_move(void)
{
    refresh_board(-1);
    apply_status_after_move();
    if (!s_game_over)
    {
        request_think();
    }
}

static void enter_promo(int c1, int c2)
{
    s_promo = true;
    s_promo_c1 = c1;
    s_promo_c2 = c2;
    chess_fsm_init(&s_fsm);
    chess_ui_set_mode(UI_MODE_PROMO);
    refresh_board(-1);
    ESP_LOGI(TAG, "choose promotion piece");
}

static void complete_promo(int piece)
{
    if (!chess_try_move(s_promo_c1, s_promo_c2, piece))
    {
        ESP_LOGW(TAG, "promo failed");
        s_promo = false;
        chess_ui_set_mode(UI_MODE_PLAY);
        refresh_board(-1);
        return;
    }
    s_promo = false;
    chess_ui_set_mode(UI_MODE_PLAY);
    ESP_LOGI(TAG, "promoted to %d", piece);
    finish_human_move();
}

static void on_board_tap(int sq)
{
    clear_new_armed();
    if (s_promo || s_game_over)
    {
        return;
    }

    const chess_fsm_event_t ev =
        chess_fsm_tap_square(&s_fsm, sq, from_selectable(sq));

    switch (ev)
    {
    case CHESS_FSM_SELECT:
        refresh_board(s_fsm.selected);
        break;
    case CHESS_FSM_CANCEL:
        refresh_board(-1);
        break;
    case CHESS_FSM_MOVE:
        if (chess_is_promotion_move(s_fsm.c1, s_fsm.c2))
        {
            enter_promo(s_fsm.c1, s_fsm.c2);
        }
        else if (chess_try_move(s_fsm.c1, s_fsm.c2, 0))
        {
            ESP_LOGI(TAG, "human %d→%d", s_fsm.c1, s_fsm.c2);
            finish_human_move();
        }
        else
        {
            ESP_LOGI(TAG, "illegal %d→%d", s_fsm.c1, s_fsm.c2);
            chess_fsm_init(&s_fsm);
            refresh_board(-1);
        }
        break;
    default:
        break;
    }
}

static void on_strip_tap(ui_strip_hit_t hit)
{
    if (s_promo)
    {
        switch (hit)
        {
        case UI_STRIP_PROMO_Q:
            complete_promo(CHESS_PROMO_QUEEN);
            break;
        case UI_STRIP_PROMO_R:
            complete_promo(CHESS_PROMO_ROOK);
            break;
        case UI_STRIP_PROMO_B:
            complete_promo(CHESS_PROMO_BISHOP);
            break;
        case UI_STRIP_PROMO_N:
            complete_promo(CHESS_PROMO_KNIGHT);
            break;
        default:
            break;
        }
        return;
    }

    if (s_thinking)
    {
        return;
    }

    switch (hit)
    {
    case UI_STRIP_NEW:
        /* Already at start — no need to confirm. */
        if (chess_ply() == 0 && !s_game_over)
        {
            do_new_game();
            break;
        }
        if (!s_new_armed)
        {
            s_new_armed = true;
            chess_ui_set_new_armed(true);
            chess_ui_paint();
            ESP_LOGI(TAG, "NEW armed — tap OK to confirm");
            break;
        }
        do_new_game();
        break;
    case UI_STRIP_UNDO:
        do_undo();
        break;
    case UI_STRIP_CALIB:
        do_calib();
        break;
    case UI_STRIP_TIME:
        cycle_think_time();
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

    if (pressed && !s_was_pressed)
    {
        const int sq = chess_geom_panel_to_square(px, py);
        if (sq >= 0 && !s_thinking)
        {
            on_board_tap(sq);
        }
        else
        {
            const ui_strip_hit_t hit =
                chess_geom_strip_hit(px, py, s_promo);
            if (hit != UI_STRIP_NONE && hit != UI_STRIP_SIDE)
            {
                on_strip_tap(hit);
            }
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
        refresh_board(-1);
        apply_status_after_move();
    }
    else
    {
        ESP_LOGW(TAG, "engine found no move");
        apply_status_after_move();
        if (!s_game_over)
        {
            refresh_board(-1);
        }
    }
}

static void run_paint_bench(void)
{
    /* Step 26: one-shot scenes matching PLAN targets (full / move / strip). */
    chess_ui_set_paint_log(true);
    ESP_LOGI(TAG, "paint_bench begin");

    chess_ui_invalidate_all();
    chess_ui_sync_from_game(-1);
    chess_ui_paint(); /* full 64 + strip */

    (void)chess_try_move(52, 36, CHESS_PROMO_QUEEN_DEFAULT); /* e2e4 */
    chess_ui_sync_from_game(-1);
    chess_ui_paint(); /* move dirty + last-move + side */

    chess_ui_set_think_ms(5000);
    chess_ui_paint(); /* TIME button only */

    chess_ui_set_busy(true);
    chess_ui_paint(); /* side badge only */
    chess_ui_set_busy(false);

    chess_ui_set_paint_log(false);
    chess_new_game();
    chess_ui_set_think_ms(s_think_ms);
    chess_ui_invalidate_all();
    chess_ui_sync_from_game(-1);
    chess_ui_paint();
    ESP_LOGI(TAG, "paint_bench end");
}

void app_main(void)
{
    ESP_LOGI(TAG, "esp32-chess — play + strip controls");

    chess_new_game();
    chess_fsm_init(&s_fsm);

    hal_display_init();
    hal_touch_init();

    if (!hal_touch_calib_from_nvs())
    {
        ESP_LOGI(TAG, "no NVS calib — running wizard");
        (void)touch_calib_run_wizard();
    }

    chess_ui_init();
    chess_ui_set_think_ms(s_think_ms);
    run_paint_bench();

    BaseType_t ok = xTaskCreatePinnedToCore(think_task, "think", THINK_STACK,
                                            NULL, 5, &s_think_task, 1);
    if (ok != pdPASS)
    {
        ESP_LOGE(TAG, "think task create failed");
        return;
    }

    /* Device baseline (step 19): one start-position think for ESP32 nps. */
    {
        chess_search_result_t r = {0};
        const int64_t t0 = esp_timer_get_time();
        const bool think_ok = chess_think_time(1000, &r);
        const double ms = (double)(esp_timer_get_time() - t0) / 1000.0;
        const double nps = (ms > 0.0) ? (1000.0 * (double)r.nodes / ms) : 0.0;
        ESP_LOGI(TAG,
                 "device_bench 1s start: ok=%d depth=%d nodes=%lu score=%d "
                 "(%.0f ms, %.0f nps)",
                 (int)think_ok, r.depth, (unsigned long)r.nodes, r.score, ms,
                 nps);
        chess_new_game();
        refresh_board(-1);
    }

    ESP_LOGI(TAG, "strip: NEW / UNDO / CAL / TIME — play White");

    while (1)
    {
        handle_think_done();
        handle_touch_edge();
        vTaskDelay(pdMS_TO_TICKS(15));
    }
}
