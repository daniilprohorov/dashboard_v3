#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "profile.h"

typedef enum {
    APP_APPLY,
    APP_ENTER_LOGGER,
    APP_EXIT_LOGGER,
} app_action_t;

typedef struct {
    bool busy;
    bool logger_open;
    bool obd_running;
    bool saved_known;
    bool saved_present;
    bool active_known;
    bool persistence_uncertain;
    obd_profile_t active;
    obd_profile_t saved;
    esp_err_t load_error;
    esp_err_t logger_init_error;
    esp_err_t operation_error;
    uint32_t revision;
    uint32_t generation;
    char message[192];
} app_state_t;

// Initialize after widgets exist, OUTSIDE the LVGL lock. Boot always starts the
// dashboard, loads only its saved profile and initializes discovery (not Logger).
esp_err_t app_controller_start(void);

// Short, nonblocking UI submission. One outstanding operation; never joins a
// worker or accesses CAN/NVS while LVGL owns its lock.
bool app_controller_request(app_action_t action, obd_profile_t draft);
void app_controller_get_state(app_state_t *out);
