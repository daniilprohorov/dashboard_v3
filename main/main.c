#include <assert.h>

#include "board.h"
#include "esp_log.h"
#include "esp_lv_adapter.h"

static const char *TAG = "main";

static void battery_timer_cb(lv_timer_t *t)
{
    lv_obj_t *label = lv_timer_get_user_data(t);
    uint32_t mv;
    if (board_battery_mv(&mv) == ESP_OK) {
        lv_label_set_text_fmt(label, "BAT %lu.%02lu V", mv / 1000, (mv % 1000) / 10);
    }
}

static void build_ui(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    lv_obj_t *title = lv_label_create(scr);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_label_set_text(title, "dashboard");
    lv_obj_center(title);

    lv_obj_t *bat = lv_label_create(scr);
    lv_obj_set_style_text_color(bat, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_label_set_text(bat, "BAT --");
    lv_obj_align(bat, LV_ALIGN_BOTTOM_RIGHT, -16, -16);
    lv_timer_create(battery_timer_cb, 1000, bat);
}

void app_main(void)
{
    const esp_lv_adapter_rotation_t rotation = ESP_LV_ADAPTER_ROTATE_0;
    const esp_lv_adapter_tear_avoid_mode_t tear = ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT_RGB;

    board_t board;
    ESP_ERROR_CHECK(board_init(esp_lv_adapter_get_required_frame_buffer_count(tear, rotation), &board));

    esp_lv_adapter_config_t adapter_cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    adapter_cfg.task_stack_size = 12 * 1024;
    adapter_cfg.stack_in_psram = true;
    ESP_ERROR_CHECK(esp_lv_adapter_init(&adapter_cfg));

    esp_lv_adapter_display_config_t disp_cfg =
        ESP_LV_ADAPTER_DISPLAY_RGB_DEFAULT_CONFIG(board.panel, NULL, BOARD_LCD_H_RES, BOARD_LCD_V_RES, rotation);
    disp_cfg.profile.use_psram = true;
    lv_display_t *disp = esp_lv_adapter_register_display(&disp_cfg);
    assert(disp);

    if (board.touch) {
        esp_lv_adapter_touch_config_t touch_cfg = ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(disp, board.touch);
        lv_indev_t *touch = esp_lv_adapter_register_touch(&touch_cfg);
        assert(touch);
    }

    ESP_ERROR_CHECK(esp_lv_adapter_start());

    ESP_ERROR_CHECK(esp_lv_adapter_lock(-1));
    build_ui();
    esp_lv_adapter_unlock();

    ESP_ERROR_CHECK(board_backlight_set(100));
    ESP_LOGI(TAG, "up");
}
