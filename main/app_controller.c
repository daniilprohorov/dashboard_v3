#include "app_controller.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "logger.h"
#include "nvs.h"
#include "obd.h"
#include "sdkconfig.h"

typedef struct {
    app_action_t action;
    obd_profile_t draft;
} request_t;

static QueueHandle_t requests;
static SemaphoreHandle_t state_lock;
static app_state_t state = {.busy = true};

void app_controller_get_state(app_state_t *out)
{
    if (!state_lock) {
        *out = (app_state_t){.busy = true, .active = profile_default(), .saved = profile_default()};
        return;
    }
    xSemaphoreTake(state_lock, portMAX_DELAY);
    *out = state;
    xSemaphoreGive(state_lock);
}

static void publish(app_state_t *next)
{
    xSemaphoreTake(state_lock, portMAX_DELAY);
    next->revision = state.revision + 1;
    state = *next;
    xSemaphoreGive(state_lock);
}

static void failure(app_state_t *s, const char *operation, esp_err_t err)
{
    s->operation_error = err;
    snprintf(s->message, sizeof s->message, "%s: %s. Not applied/saved.",
             operation, esp_err_to_name(err));
}

static esp_err_t stop_obd(app_state_t *s)
{
#if CONFIG_DASH_DEMO
    esp_err_t err = ESP_OK; // Demo owns no CAN node and creates no OBD task.
#else
    esp_err_t err = obd_stop();
#endif
    // A stop error retains ownership but does not prove polling is still live.
    s->obd_running = false;
    s->active_known = false;
    return err;
}

static esp_err_t start_obd(app_state_t *s, obd_profile_t profile)
{
#if CONFIG_DASH_DEMO
    esp_err_t err = ESP_OK;
#else
    esp_err_t err = obd_start(&profile);
#endif
    if (err == ESP_OK) {
        s->active = profile;
        s->active_known = true;
        s->obd_running = true;
        s->generation++;
    }
    return err;
}

static void apply(app_state_t *s, obd_profile_t draft)
{
    esp_err_t err = stop_obd(s);
    if (err != ESP_OK) {
        failure(s, "OBD stop failed; ownership retained", err);
        return;
    }
    err = start_obd(s, draft);
    if (err != ESP_OK) {
        // A failed start can retain cleanup resources. The next operation MUST
        // stop successfully before starting anything else; no implicit rollback.
        failure(s, "OBD start failed; polling stopped", err);
        return;
    }
    err = profile_save(draft);
    if (err != ESP_OK) {
        s->persistence_uncertain = true;
        s->operation_error = err;
        snprintf(s->message, sizeof s->message,
                 "Active, NOT confirmed saved: %s. Persistence uncertain; Logger exit restores last confirmed saved profile.",
                 esp_err_to_name(err));
        return;
    }
    s->saved = draft;
    s->saved_known = true;
    s->saved_present = true;
    s->persistence_uncertain = false;
    s->load_error = ESP_OK;
    snprintf(s->message, sizeof s->message, "Applied and saved. Discovering ECU; No ECU does not roll back the profile.");
}

static void enter_logger(app_state_t *s)
{
    if (s->logger_init_error != ESP_OK) {
        failure(s, "Logger unavailable; OBD unchanged", s->logger_init_error);
        return;
    }
    esp_err_t err = stop_obd(s);
    if (err != ESP_OK) {
        failure(s, "OBD stop failed; Logger NOT entered", err);
        return;
    }
    err = logger_enter();
    if (err != ESP_OK) {
        // Enter can time out after its request was accepted. Quiesce Logger
        // explicitly before any recovery OBD start, never assume it owns no CAN.
        esp_err_t leave_err = logger_leave();
        if (leave_err != ESP_OK) {
            s->logger_open = true;
            failure(s, "Logger entry/cleanup failed; Exit retries release", leave_err);
            return;
        }
        failure(s, "Logger entry failed; polling stopped; Apply to resume", err);
        return;
    }
    s->logger_open = true;
    snprintf(s->message, sizeof s->message, "Logger idle; waiting for an explicit CLI capture.");
}

static void exit_logger(app_state_t *s)
{
    esp_err_t err = logger_leave();
    if (err != ESP_OK) {
        failure(s, "Logger release failed; OBD NOT restarted; retry Exit", err);
        return;
    }
    // ESP_OK is the CAN-release guarantee, even if UART END failed. Preserve
    // that separate UART error in the message instead of hiding it on return.
    logger_status_t status;
    logger_get_status(&status);
    s->logger_open = false;
    if (!s->saved_known) {
        obd_profile_t saved;
        err = profile_load(&saved);
        if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
            s->load_error = err;
            failure(s, "Cannot restore unknown saved profile; Apply to resume", err);
            return;
        }
        s->saved = saved;
        s->saved_known = true;
        s->saved_present = err == ESP_OK;
        s->load_error = ESP_OK;
    }
    // Never resume a merely active/unsaved profile or the dropdown draft.
    err = start_obd(s, s->saved);
    if (err != ESP_OK) {
        failure(s, "Saved OBD profile restart failed; Apply to retry", err);
        return;
    }
    if (status.error != ESP_OK) {
        s->operation_error = status.error;
        snprintf(s->message, sizeof s->message,
                 "CAN released; saved OBD profile restored. Logger/UART error: %s.",
                 esp_err_to_name(status.error));
    } else {
        snprintf(s->message, sizeof s->message, "Logger exited; saved dashboard profile restored.");
    }
}

static void controller_task(void *arg)
{
    app_state_t s;
    app_controller_get_state(&s);
    esp_err_t load = profile_load(&s.saved);
    s.saved_known = load == ESP_OK || load == ESP_ERR_NVS_NOT_FOUND;
    s.saved_present = load == ESP_OK;
    s.load_error = s.saved_known ? ESP_OK : load;
    s.logger_init_error = logger_init();
    esp_err_t err = start_obd(&s, s.saved);
    if (err != ESP_OK) {
        failure(&s, "Boot OBD start failed; Apply to retry", err);
    } else if (!s.saved_known) {
        snprintf(s.message, sizeof s.message,
                 "Profile load failed: %s. Running default, NOT a restored saved profile. NVS was not erased.",
                 esp_err_to_name(load));
    } else {
        snprintf(s.message, sizeof s.message, "%s; discovering ECU.",
                 s.saved_present ? "Saved dashboard profile restored" : "No stored profile; using default");
    }
    s.busy = false;
    publish(&s);

    request_t request;
    for (;;) {
        xQueueReceive(requests, &request, portMAX_DELAY);
        s.operation_error = ESP_OK;
        switch (request.action) {
        case APP_APPLY:        apply(&s, request.draft); break;
        case APP_ENTER_LOGGER: enter_logger(&s); break;
        case APP_EXIT_LOGGER:  exit_logger(&s); break;
        }
        s.busy = false;
        publish(&s);
    }
}

esp_err_t app_controller_start(void)
{
    state_lock = xSemaphoreCreateMutex();
    requests = xQueueCreate(1, sizeof(request_t));
    if (!state_lock || !requests) {
        if (state_lock) { vSemaphoreDelete(state_lock); state_lock = NULL; }
        if (requests) { vQueueDelete(requests); requests = NULL; }
        return ESP_ERR_NO_MEM;
    }
    state = (app_state_t){.busy = true, .active = profile_default(), .saved = profile_default()};
    snprintf(state.message, sizeof state.message, "Starting dashboard...");
    if (xTaskCreate(controller_task, "app_controller", 6144, NULL, 4, NULL) != pdPASS) {
        vQueueDelete(requests);
        vSemaphoreDelete(state_lock);
        requests = NULL;
        state_lock = NULL;
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

bool app_controller_request(app_action_t action, obd_profile_t draft)
{
    if (!state_lock) { return false; }
    if (action < APP_APPLY || action > APP_EXIT_LOGGER || !profile_valid(draft)) {
        return false;
    }
    xSemaphoreTake(state_lock, portMAX_DELAY);
    if (state.busy || (action == APP_EXIT_LOGGER) != state.logger_open) {
        xSemaphoreGive(state_lock);
        return false;
    }
    state.busy = true;
    state.revision++;
    const request_t request = {.action = action, .draft = draft};
    if (xQueueSend(requests, &request, 0) != pdTRUE) {
        state.busy = false;
        xSemaphoreGive(state_lock);
        return false;
    }
    xSemaphoreGive(state_lock);
    return true;
}
