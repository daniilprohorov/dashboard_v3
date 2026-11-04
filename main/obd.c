// Restartable ISO 15765-4 OBD-II mode 01 poller (11/29-bit addressing).
// Only single-frame replies are needed for the dashboard's mode 01 PIDs.
#include "obd.h"

#include <stdatomic.h>
#include <stdio.h>
#include <string.h>

#include "board.h"
#include "dashboard.h"
#include "esp_attr.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_lv_adapter.h"
#include "esp_twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/idf_additions.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char *TAG = "obd";

#define OBD_REQ_STD     0x7DFu
#define OBD_RESP_STD    0x7E8u
#define OBD_MASK_STD    0x7F8u
#define OBD_REQ_EXT     0x18DB33F1u // functional request, external tester F1
#define OBD_RESP_EXT    0x18DAF100u // physical replies addressed to tester F1
#define OBD_MASK_EXT    0x1FFFFF00u // ECU source address is the low byte
#define OBD_TIMEOUT_MS  50    // J1979 P2 max
#define OBD_CYCLE_MS    50    // min period of one poll cycle / screen refresh
#define OBD_MAX_MISSES  5     // consecutive timeouts before the ECU counts as lost
#define OBD_RETRY_MS    1000  // pause between discovery attempts
#define OBD_UI_WAIT_MS  50
#define OBD_STOP_MS     500

#define PID_STATUS   0x01
#define PID_ECT      0x05
#define PID_RPM      0x0C
#define PID_SPEED    0x0D
#define PID_MAF      0x10
#define PID_FUEL_LVL 0x2F
#define PID_AMBIENT  0x46
#define PID_OIL_TEMP 0x5C
#define PID_ODOMETER 0xA6

// Polled every cycle (if supported).
static const uint8_t fast_pids[] = {PID_RPM, PID_SPEED, PID_MAF};
// One of these per cycle, round robin.
static const uint8_t slow_pids[] = {PID_ECT, PID_OIL_TEMP, PID_AMBIENT, PID_FUEL_LVL, PID_ODOMETER, PID_STATUS};

typedef struct {
    // The ISR validates address, format and RTR before enqueueing.
    uint8_t len;
    uint8_t data[8];
} rx_msg_t;

static twai_node_handle_t s_node;
static QueueHandle_t s_rx;
static SemaphoreHandle_t s_done;
static TaskHandle_t s_worker;
static atomic_bool s_stop_requested;
static bool s_worker_joined;
static bool s_node_enabled;
static bool s_extended;
// IDF queues frame POINTERS, not copies. Keep this frame and payload alive and
// unchanged until wait_all_done succeeds (or the node has been deleted).
static uint8_t s_tx_data[8];
static twai_frame_t s_tx_frame;
static uint32_t s_supported[8]; // mode 01 PIDs 0x01..0xFF, MSB-first per 32-PID block
static uint32_t s_maf_x100;     // g/s × 100
static bool s_maf_valid;
static char s_errors[24];

static bool IRAM_ATTR on_rx(twai_node_handle_t node, const twai_rx_done_event_data_t *edata, void *ctx)
{
    rx_msg_t m;
    twai_frame_t f = {.buffer = m.data, .buffer_len = sizeof m.data};
    if (twai_node_receive_from_isr(node, &f) != ESP_OK) {
        return false;
    }
    // The S3 hardware acceptance filter alone cannot guarantee IDE/RTR.
    uint32_t mask = s_extended ? OBD_MASK_EXT : OBD_MASK_STD;
    uint32_t reply = s_extended ? OBD_RESP_EXT : OBD_RESP_STD;
    if (f.header.ide != s_extended || f.header.rtr || f.header.fdf ||
        f.header.dlc > sizeof m.data || (f.header.id & mask) != reply) {
        return false;
    }
    m.len = f.header.dlc;
    BaseType_t woken = pdFALSE;
    xQueueSendFromISR((QueueHandle_t)ctx, &m, &woken);
    return woken == pdTRUE;
}

static bool pid_supported(uint8_t pid)
{
    return pid && (s_supported[(pid - 1) / 32] & (0x80000000u >> ((pid - 1) % 32)));
}

static void recover_if_bus_off(void)
{
    twai_node_status_t st;
    if (twai_node_get_info(s_node, &st, NULL) == ESP_OK && st.state == TWAI_ERROR_BUS_OFF) {
        ESP_LOGW(TAG, "bus off, recovering");
        twai_node_recover(s_node);
    }
}

// Mode 01 request. Copies reply payload (after the PID byte) to out[4]; returns its length or -1.
static int request(uint8_t pid, uint8_t out[4])
{
    if (atomic_load(&s_stop_requested)) {
        return -1;
    }
    // Never overwrite the driver's outstanding frame on a missing ACK/bus-off.
    if (twai_node_transmit_wait_all_done(s_node, 0) != ESP_OK) {
        recover_if_bus_off();
        return -1;
    }
    xQueueReset(s_rx); // drop late replies to earlier requests
    memset(s_tx_data, 0, sizeof s_tx_data);
    s_tx_data[0] = 0x02;
    s_tx_data[1] = 0x01;
    s_tx_data[2] = pid;
    s_tx_frame = (twai_frame_t) {
        .header = {.id = s_extended ? OBD_REQ_EXT : OBD_REQ_STD, .dlc = 8, .ide = s_extended},
        .buffer = s_tx_data,
        .buffer_len = sizeof s_tx_data,
    };
    if (twai_node_transmit(s_node, &s_tx_frame, 0) != ESP_OK) {
        recover_if_bus_off();
        return -1;
    }

    TickType_t start = xTaskGetTickCount();
    TickType_t timeout = pdMS_TO_TICKS(OBD_TIMEOUT_MS);
    rx_msg_t m;
    while (!atomic_load(&s_stop_requested)) {
        TickType_t elapsed = xTaskGetTickCount() - start; // wrap-safe
        if (elapsed >= timeout || xQueueReceive(s_rx, &m, timeout - elapsed) != pdTRUE) {
            return -1;
        }
        if (m.len < 3) {
            continue;
        }
        // ISO-TP single frame: [len, 0x41, pid, data...].
        uint8_t n = m.data[0];
        if (n >= 2 && n <= 7 && n < m.len && m.data[1] == 0x41 && m.data[2] == pid) {
            size_t count = n - 2 > 4 ? 4 : n - 2;
            memcpy(out, &m.data[3], count);
            return count;
        }
    }
    return -1;
}

// Reads the supported-PID bitmaps 0x00, 0x20, ... as far as the chain goes.
static bool discover(void)
{
    memset(s_supported, 0, sizeof s_supported);
    for (unsigned base = 0; base <= 0xE0; base += 0x20) {
        if (atomic_load(&s_stop_requested)) {
            return false;
        }
        uint8_t b[4];
        if (request(base, b) != 4) {
            return base > 0;
        }
        s_supported[base / 32] = (uint32_t)b[0] << 24 | (uint32_t)b[1] << 16 | (uint32_t)b[2] << 8 | b[3];
        if (base == 0xE0 || !pid_supported(base + 0x20)) {
            break;
        }
    }
    ESP_LOGI(TAG, "supported PIDs: %08lx %08lx %08lx %08lx %08lx %08lx",
             s_supported[0], s_supported[1], s_supported[2], s_supported[3], s_supported[4], s_supported[5]);
    return true;
}

static void decode(uint8_t pid, const uint8_t *b, int n, dash_data_t *d)
{
    if (n < 1) {
        return;
    }
    switch (pid) {
    case PID_RPM:
        if (n >= 2) {
            d->rpm = ((uint32_t)b[0] << 8 | b[1]) / 4;
            d->valid |= DASH_BIT(DASH_RPM);
        }
        break;
    case PID_SPEED:
        d->speed = b[0];
        d->valid |= DASH_BIT(DASH_SPEED);
        break;
    case PID_MAF:
        if (n >= 2) {
            s_maf_x100 = (uint32_t)b[0] << 8 | b[1];
            s_maf_valid = true;
        }
        break;
    case PID_ECT:
        d->wtmp = (int16_t)b[0] - 40;
        d->valid |= DASH_BIT(DASH_WTMP);
        break;
    case PID_OIL_TEMP:
        d->otmp = (int16_t)b[0] - 40;
        d->valid |= DASH_BIT(DASH_OTMP);
        break;
    case PID_AMBIENT:
        d->etmp = (int16_t)b[0] - 40;
        d->valid |= DASH_BIT(DASH_ETMP);
        break;
    case PID_FUEL_LVL:
        d->flvl_x10 = (uint16_t)((uint32_t)b[0] * CONFIG_DASH_FUEL_TANK_L * 10 / 255);
        d->valid |= DASH_BIT(DASH_FLVL);
        break;
    case PID_ODOMETER:
        if (n >= 4) {
            d->odometer = ((uint32_t)b[0] << 24 | (uint32_t)b[1] << 16 | (uint32_t)b[2] << 8 | b[3]) / 10;
            d->valid |= DASH_BIT(DASH_ODO);
        }
        break;
    case PID_STATUS:
        if (b[0] & 0x80) {
            snprintf(s_errors, sizeof s_errors, "MIL %u DTC", b[0] & 0x7F);
        } else {
            s_errors[0] = '\0';
        }
        break;
    }
}

// Instantaneous l/100 km from MAF at stoichiometric petrol (AFR 14.7, 745 g/l).
// Undefined at standstill, so the field goes unknown below 1 km/h.
static void update_consumption(dash_data_t *d)
{
    d->valid &= ~DASH_BIT(DASH_FCNS);
    if (!s_maf_valid || !pid_supported(PID_MAF) || !(d->valid & DASH_BIT(DASH_SPEED)) || d->speed == 0) {
        return;
    }
    // l/100km × 10 = maf_x100/100 [g/s] × 3600 / (14.7 × 745) [l/h] × 100 / speed × 10
    uint64_t v = (uint64_t)s_maf_x100 * 3600 * 100 / (147ULL * 745 * d->speed);
    d->fcns_x10 = v > 999 ? 999 : (uint16_t)v;
    d->valid |= DASH_BIT(DASH_FCNS);
}

static esp_err_t show(const dash_data_t *d)
{
    esp_err_t err = esp_lv_adapter_lock(OBD_UI_WAIT_MS);
    if (err != ESP_OK) {
        return err;
    }
    if (!atomic_load(&s_stop_requested)) {
        dashboard_update(d);
    }
    esp_lv_adapter_unlock();
    return ESP_OK;
}

static void reset_measurements(dash_data_t *d)
{
    *d = (dash_data_t) {.errors = s_errors};
    memset(s_supported, 0, sizeof s_supported);
    s_maf_x100 = 0;
    s_maf_valid = false;
    strcpy(s_errors, "No ECU");
}

// Task notifications interrupt retry/cycle delays; RX and LVGL waits are finite.
static void pause_ms(uint32_t ms)
{
    if (!atomic_load(&s_stop_requested)) {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(ms));
    }
}

static void obd_task(void *arg)
{
    dash_data_t d = {.errors = s_errors};
    size_t slow_idx = 0;
    unsigned misses = 0;
    bool online = false;

    while (!atomic_load(&s_stop_requested)) {
        if (!online) {
            reset_measurements(&d);
            show(&d);
            if (!discover()) {
                pause_ms(OBD_RETRY_MS);
                continue;
            }
            ESP_LOGI(TAG, "ECU online");
            s_errors[0] = '\0';
            online = true;
            misses = 0;
        }

        TickType_t start = xTaskGetTickCount();
        uint8_t polled[sizeof fast_pids + 1];
        size_t n = 0;
        for (size_t i = 0; i < sizeof fast_pids; i++) {
            polled[n++] = fast_pids[i];
        }
        polled[n++] = slow_pids[slow_idx];
        slow_idx = (slow_idx + 1) % sizeof slow_pids;

        for (size_t i = 0; i < n; i++) {
            if (atomic_load(&s_stop_requested)) {
                break;
            }
            if (!pid_supported(polled[i])) {
                continue;
            }
            uint8_t b[4];
            int len = request(polled[i], b);
            if (len < 1) {
                misses++;
                continue;
            }
            misses = 0;
            decode(polled[i], b, len, &d);
        }

        if (misses >= OBD_MAX_MISSES) {
            ESP_LOGW(TAG, "ECU lost");
            online = false;
            continue;
        }

        update_consumption(&d);
        show(&d);
        TickType_t elapsed = xTaskGetTickCount() - start;
        TickType_t cycle = pdMS_TO_TICKS(OBD_CYCLE_MS);
        if (elapsed < cycle && !atomic_load(&s_stop_requested)) {
            ulTaskNotifyTake(pdTRUE, cycle - elapsed);
        }
    }
    // No shared resources are touched after signaling completion. The owner
    // deletes this now-quiescent task before deleting the node/RX queue.
    xSemaphoreGive(s_done);
    for (;;) {
        vTaskSuspend(NULL);
    }
}

static esp_err_t release_resources(void)
{
    esp_err_t err = board_can_release(&s_node, &s_node_enabled);
    if (err != ESP_OK) {
        return err;
    }
    if (s_rx) {
        vQueueDeleteWithCaps(s_rx);
        s_rx = NULL;
    }
    if (s_done) {
        vSemaphoreDeleteWithCaps(s_done);
        s_done = NULL;
    }
    return ESP_OK;
}

esp_err_t obd_stop(void)
{
    atomic_store(&s_stop_requested, true);
    if (s_worker) {
        if (!s_worker_joined) {
            xTaskNotifyGive(s_worker);
            if (xSemaphoreTake(s_done, pdMS_TO_TICKS(OBD_STOP_MS)) != pdTRUE) {
                return ESP_ERR_TIMEOUT;
            }
            s_worker_joined = true;
        }
        vTaskDelete(s_worker);
        s_worker = NULL;
    }
    return release_resources();
}

esp_err_t obd_start(const obd_profile_t *profile)
{
    if (!profile || !profile_valid(*profile)) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_node || s_worker || s_rx || s_done) {
        return ESP_ERR_INVALID_STATE;
    }
    s_extended = profile_id_bits(*profile) == 29;
    s_worker_joined = false;
    atomic_store(&s_stop_requested, false);
    s_rx = xQueueCreateWithCaps(8, sizeof(rx_msg_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    s_done = xSemaphoreCreateBinaryWithCaps(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    esp_err_t err = ESP_ERR_NO_MEM;
    if (!s_rx || !s_done) {
        goto fail;
    }
    err = board_can_create(profile_bitrate(*profile), false, &s_node);
    if (err != ESP_OK) {
        goto fail;
    }
    const twai_mask_filter_config_t filter = {
        .id = s_extended ? OBD_RESP_EXT : OBD_RESP_STD,
        .mask = s_extended ? OBD_MASK_EXT : OBD_MASK_STD,
        .is_ext = s_extended,
    };
    err = twai_node_config_mask_filter(s_node, 0, &filter);
    if (err != ESP_OK) {
        goto fail;
    }
    const twai_event_callbacks_t cbs = {.on_rx_done = on_rx};
    err = twai_node_register_event_callbacks(s_node, &cbs, s_rx);
    if (err != ESP_OK) {
        goto fail;
    }
    s_node_enabled = true; // include partially failed enable in cleanup
    err = twai_node_enable(s_node);
    if (err != ESP_OK) {
        goto fail;
    }
    dash_data_t empty;
    reset_measurements(&empty);
    err = show(&empty);
    if (err != ESP_OK) {
        goto fail;
    }
    if (xTaskCreate(obd_task, "obd", 4096, NULL, 5, &s_worker) != pdPASS) {
        err = ESP_ERR_NO_MEM;
        goto fail;
    }
    return ESP_OK;

fail:
    atomic_store(&s_stop_requested, true);
    esp_err_t cleanup_err = release_resources();
    return cleanup_err == ESP_OK ? err : cleanup_err;
}
