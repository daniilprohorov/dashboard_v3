#include "app_ui.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "app_controller.h"
#include "board.h"
#include "dashboard.h"
#include "logger.h"
#include "lvgl.h"
#include "sdkconfig.h"

LV_FONT_DECLARE(font_b612_40);
LV_FONT_DECLARE(font_b612_24);

typedef enum { PAGE_DASHBOARD, PAGE_SETTINGS, PAGE_LOGGER } page_t;
static page_t page;
static lv_obj_t *dashboard_screen, *settings_screen, *logger_screen;
static lv_obj_t *profiles, *settings_info, *settings_message, *logger_info;
static lv_obj_t *apply_button, *logger_button, *back_button, *exit_button;
static uint32_t shown_revision = UINT32_MAX;
static char last_logger_text[1536];

static void text(lv_obj_t *label, const char *value)
{
    if (strcmp(lv_label_get_text(label), value) != 0) {
        lv_label_set_text(label, value);
    }
}

static lv_obj_t *label(lv_obj_t *parent, int x, int y, int w, const lv_font_t *font)
{
    lv_obj_t *obj = lv_label_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_width(obj, w);
    lv_obj_set_style_text_font(obj, font, 0);
    lv_label_set_long_mode(obj, LV_LABEL_LONG_WRAP);
    lv_label_set_text_static(obj, "");
    return obj;
}

static lv_obj_t *screen(const char *title)
{
    lv_obj_t *obj = lv_obj_create(NULL);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, BOARD_LCD_H_RES, BOARD_LCD_V_RES);
    lv_obj_set_scrollable(obj, false);
    lv_obj_set_style_bg_color(obj, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(obj, lv_color_white(), 0);
    lv_obj_set_style_text_font(obj, &font_b612_24, 0);
    text(label(obj, 24, 16, 740, &font_b612_40), title);
    return obj;
}

static lv_obj_t *button(lv_obj_t *parent, const char *caption, int x, int y,
                        int w, lv_event_cb_t callback)
{
    lv_obj_t *obj = lv_button_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, 64);
    lv_obj_set_style_border_color(obj, lv_color_white(), 0);
    lv_obj_set_style_border_width(obj, 3, 0);
    lv_obj_set_style_bg_color(obj, lv_color_white(), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_STATE_PRESSED);
    lv_obj_set_style_text_color(obj, lv_color_black(), LV_STATE_PRESSED);
    lv_obj_set_style_text_opa(obj, LV_OPA_50, LV_STATE_DISABLED);
    lv_obj_t *caption_label = lv_label_create(obj);
    lv_label_set_text(caption_label, caption);
    lv_obj_center(caption_label);
    lv_obj_add_event_cb(obj, callback, LV_EVENT_CLICKED, NULL);
    return obj;
}

static void enabled(lv_obj_t *obj, bool value)
{
    if (value) { lv_obj_remove_state(obj, LV_STATE_DISABLED); }
    else { lv_obj_add_state(obj, LV_STATE_DISABLED); }
}

static void show(page_t next)
{
    page = next;
    lv_screen_load(next == PAGE_DASHBOARD ? dashboard_screen :
                   next == PAGE_SETTINGS ? settings_screen : logger_screen);
}

static void open_settings(lv_event_t *event)
{
    app_state_t s;
    app_controller_get_state(&s);
    if (s.busy || s.logger_open) { return; }
    // Reopening discards every unapplied dropdown selection.
    lv_dropdown_set_selected(profiles, s.active_known ? s.active : s.saved);
    show(PAGE_SETTINGS);
}

static void back(lv_event_t *event)
{
    app_state_t s;
    app_controller_get_state(&s);
    if (!s.busy) { show(PAGE_DASHBOARD); }
}

static void apply(lv_event_t *event)
{
    app_controller_request(APP_APPLY, (obd_profile_t)lv_dropdown_get_selected(profiles));
}

static void enter_logger(lv_event_t *event)
{
    app_controller_request(APP_ENTER_LOGGER, (obd_profile_t)lv_dropdown_get_selected(profiles));
}

static void exit_logger(lv_event_t *event)
{
    app_controller_request(APP_EXIT_LOGGER, profile_default());
}

static void refresh_logger(const app_state_t *s)
{
    logger_status_t status;
    logger_get_status(&status);
    const char *phase = status.mode == LOGGER_MODE_CAPTURING ? "Capturing (listen-only)" :
                        status.error != ESP_OK ? "Error; capture stopped / awaiting CLI" :
                        status.reason != LOGGER_REASON_NONE ? "Capture ended; awaiting new CLI START" :
                        "Idle; waiting for CLI";
    char config[160];
    if (status.bitrate) {
        snprintf(config, sizeof config, "Effective CAN: %" PRIu32 " bit/s; IDs: %s; listen-only\nRequested CAN: %" PRIu32 " bit/s",
                 status.bitrate, logger_ids_name(status.ids), status.requested_bitrate);
    } else {
        snprintf(config, sizeof config, "Effective CAN: none (no applied capture configuration)");
    }
    char output[sizeof last_logger_text];
    snprintf(output, sizeof output,
             "%s\n%s\nEffective filter (inclusive OR, then ID type):\n%s\n"
             "UART TX: %s (NOT proof of a reader)\n"
             "CLI ACK/heartbeat: %s; acknowledged seq: %" PRIu64 "\n"
             "Assigned seq: %" PRIu64 "; UART TX seq: %" PRIu64 "\n"
             "Software dropped frames: %" PRIu64 "\n"
             "Hardware overflow events: %" PRIu64 "; lost frames: unknown\n"
             "Last reason: %s; error: %s\n"
             "ACK means CLI receipt, not disk/fsync or absence of upstream loss.\n%s%s",
             phase, config, status.filter, status.tx ? "enabled/healthy" : "off/unavailable",
             status.client ? "confirmed for this session" : "not confirmed",
             status.ack, status.seq, status.tx_seq, status.sw_drops, status.hw_overruns,
             logger_reason_name(status.reason), esp_err_to_name(status.error),
             s->busy ? "Exiting; waiting for CAN release...\n" : "",
             s->operation_error != ESP_OK ? s->message : "Exit stops capture with user_exit and restores saved OBD.");
    if (strcmp(output, last_logger_text) != 0) {
        snprintf(last_logger_text, sizeof last_logger_text, "%s", output);
        text(logger_info, output);
    }
}

static void refresh(lv_timer_t *timer)
{
    app_state_t s;
    app_controller_get_state(&s);
    if (s.logger_open && page != PAGE_LOGGER) {
        show(PAGE_LOGGER);
    } else if (!s.logger_open && page == PAGE_LOGGER) {
        show(PAGE_DASHBOARD);
    }
#if CONFIG_DASH_DEMO
    static uint32_t generation;
    if (generation != s.generation) {
        generation = s.generation;
        const dash_data_t empty = {0};
        dashboard_update(&empty);
    }
    dashboard_demo_set_enabled(s.obd_running && !s.logger_open && !s.busy);
#endif
    if (shown_revision != s.revision) {
        shown_revision = s.revision;
        enabled(profiles, !s.busy);
        enabled(apply_button, !s.busy);
        enabled(logger_button, !s.busy);
        enabled(back_button, !s.busy);
        enabled(exit_button, !s.busy);
        char info[512];
        snprintf(info, sizeof info,
                 "%sActive: %s\nSaved: %s%s%s\nProfile load: %s; Logger startup: %s",
#if CONFIG_DASH_DEMO
                 "DEMO: simulated dashboard; no OBD CAN requests.\n",
#else
                 "",
#endif
                 s.active_known ? profile_name(s.active) : "none confirmed (polling stopped/cleanup pending)",
                 s.saved_known ? profile_name(s.saved) : "unknown (read failed)",
                 s.saved_known && !s.saved_present ? " (default; not yet stored)" : "",
                 s.persistence_uncertain ? " (last confirmed; persistence uncertain)" : "",
                 esp_err_to_name(s.load_error), esp_err_to_name(s.logger_init_error));
        text(settings_info, info);
        text(settings_message, s.busy ? "Working; waiting for safe ownership transfer..." : s.message);
        // Preserve the ECU warning, adding a separate app error line. Full
        // failure details stay in Settings; boot errors survive navigation.
        const char *error = s.load_error != ESP_OK ? "Profile load error" :
                            s.logger_init_error != ESP_OK ? "Logger init error" :
                            s.operation_error != ESP_OK ? "App error: Settings" :
                            s.persistence_uncertain ? "Save uncertain" : NULL;
        dashboard_set_app_error(error);
    }
    if (page == PAGE_LOGGER) { refresh_logger(&s); }
}

void app_ui_create(void)
{
    dashboard_create(open_settings);
    dashboard_screen = lv_screen_active();
    settings_screen = screen("Settings");
    text(label(settings_screen, 24, 80, 976, &font_b612_24),
         "Dashboard ISO 15765-4 profile (draft until Apply)");
    profiles = lv_dropdown_create(settings_screen);
    lv_obj_set_pos(profiles, 24, 120);
    lv_obj_set_size(profiles, 976, 64);
    lv_obj_set_style_text_font(profiles, &font_b612_24, 0);
    lv_obj_set_style_bg_color(profiles, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(profiles, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(profiles, lv_color_white(), 0);
    lv_obj_set_style_border_color(profiles, lv_color_white(), 0);
    lv_obj_set_style_border_width(profiles, 3, 0);
    lv_obj_set_style_radius(profiles, 0, 0);
    lv_dropdown_set_symbol(profiles, NULL); // no built-in icon/font or color
    char options[192];
    snprintf(options, sizeof options, "%s\n%s\n%s\n%s",
             profile_name(OBD_PROFILE_11BIT_250K), profile_name(OBD_PROFILE_11BIT_500K),
             profile_name(OBD_PROFILE_29BIT_250K), profile_name(OBD_PROFILE_29BIT_500K));
    lv_dropdown_set_options(profiles, options);
    lv_obj_t *list = lv_dropdown_get_list(profiles);
    lv_obj_set_style_text_font(list, &font_b612_24, 0);
    lv_obj_set_style_text_color(list, lv_color_white(), 0);
    lv_obj_set_style_bg_color(list, lv_color_black(), 0);
    lv_obj_set_style_border_color(list, lv_color_white(), 0);
    lv_obj_set_style_bg_color(list, lv_color_white(), LV_PART_SELECTED);
    lv_obj_set_style_text_color(list, lv_color_black(), LV_PART_SELECTED);
    lv_obj_set_style_radius(list, 0, 0);

    settings_info = label(settings_screen, 24, 200, 976, &font_b612_24);
    settings_message = label(settings_screen, 24, 370, 976, &font_b612_24);
    apply_button = button(settings_screen, "Apply", 24, 512, 300, apply);
    logger_button = button(settings_screen, "Logger", 362, 512, 300, enter_logger);
    back_button = button(settings_screen, "Back", 700, 512, 300, back);

    logger_screen = screen("Logger");
    exit_button = button(logger_screen, "Exit", 800, 16, 200, exit_logger);
    lv_obj_t *body = lv_obj_create(logger_screen);
    lv_obj_remove_style_all(body);
    lv_obj_set_pos(body, 24, 100);
    lv_obj_set_size(body, 976, 476);
    lv_obj_set_scroll_dir(body, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(body, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_bg_color(body, lv_color_white(), LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(body, LV_OPA_COVER, LV_PART_SCROLLBAR);
    logger_info = label(body, 0, 0, 948, &font_b612_24);
    // All three screens live for the application lifetime. OBD, blink and demo
    // timers therefore never retain deleted widgets during navigation.
    lv_timer_create(refresh, 100, NULL);
}
