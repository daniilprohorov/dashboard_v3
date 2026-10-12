// OBD-II (SAE J1979, ISO 15765-4 11-bit) poller: requests mode 01 PIDs on CAN
// and feeds the dashboard. Only single-frame replies are needed for mode 01.
#include "obd.h"

#include <stdio.h>
#include <string.h>

#include "board.h"
#include "dashboard.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_lv_adapter.h"
#include "esp_twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char *TAG = "obd";

#define OBD_REQ_ID      0x7DF // functional broadcast request
#define OBD_RESP_ID     0x7E8 // ECU replies 0x7E8..0x7EF
#define OBD_RESP_MASK   0x7F8
#define OBD_TIMEOUT_MS  50    // J1979 P2 max
#define OBD_CYCLE_MS    50    // min period of one poll cycle / screen refresh
#define OBD_MAX_MISSES  5     // consecutive timeouts before the ECU counts as lost
#define OBD_RETRY_MS    1000  // pause between discovery attempts

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
    uint32_t id;
    uint8_t len;
    uint8_t data[8];
} rx_msg_t;

static twai_node_handle_t s_node;
static QueueHandle_t s_rx;
static uint32_t s_supported[8]; // mode 01 PIDs 0x01..0xFF, MSB-first per 32-PID block
static uint32_t s_maf_x100;     // g/s × 100
static char s_errors[24];

static bool on_rx(twai_node_handle_t node, const twai_rx_done_event_data_t *edata, void *ctx)
{
    rx_msg_t m;
    twai_frame_t f = {.buffer = m.data, .buffer_len = sizeof m.data};
    if (twai_node_receive_from_isr(node, &f) != ESP_OK) {
        return false;
    }
    m.id = f.header.id;
    m.len = f.header.dlc > sizeof m.data ? sizeof m.data : f.header.dlc;
    BaseType_t woken = pdFALSE;
    xQueueSendFromISR(s_rx, &m, &woken);
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
    xQueueReset(s_rx); // drop late replies to earlier requests

    uint8_t req[8] = {0x02, 0x01, pid};
    twai_frame_t f = {.header = {.id = OBD_REQ_ID, .dlc = 8}, .buffer = req, .buffer_len = sizeof req};
    if (twai_node_transmit(s_node, &f, OBD_TIMEOUT_MS) != ESP_OK) {
        recover_if_bus_off();
        return -1;
    }

    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(OBD_TIMEOUT_MS);
    rx_msg_t m;
    for (;;) {
        TickType_t now = xTaskGetTickCount();
        if (now >= deadline || xQueueReceive(s_rx, &m, deadline - now) != pdTRUE) {
            return -1;
        }
        // Single frame: [len, 0x41, pid, data...]
        uint8_t n = m.data[0];
        if (m.len >= 3 && n >= 2 && n <= 7 && n < m.len && m.data[1] == 0x41 && m.data[2] == pid) {
            memcpy(out, &m.data[3], n - 2 > 4 ? 4 : n - 2);
            return n - 2;
        }
    }
}

// Reads the supported-PID bitmaps 0x00, 0x20, ... as far as the chain goes.
static bool discover(void)
{
    memset(s_supported, 0, sizeof s_supported);
    for (unsigned base = 0; base <= 0xE0; base += 0x20) {
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
    if (!pid_supported(PID_MAF) || !(d->valid & DASH_BIT(DASH_SPEED)) || d->speed == 0) {
        return;
    }
    // l/100km × 10 = maf_x100/100 [g/s] × 3600 / (14.7 × 745) [l/h] × 100 / speed × 10
    uint64_t v = (uint64_t)s_maf_x100 * 3600 * 100 / (147ULL * 745 * d->speed);
    d->fcns_x10 = v > 999 ? 999 : (uint16_t)v;
    d->valid |= DASH_BIT(DASH_FCNS);
}

static void show(const dash_data_t *d)
{
    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        dashboard_update(d);
        esp_lv_adapter_unlock();
    }
}

static void obd_task(void *arg)
{
    dash_data_t d = {.errors = s_errors};
    size_t slow_idx = 0;
    unsigned misses = 0;
    bool online = false;

    for (;;) {
        if (!online) {
            d.valid = 0;
            strcpy(s_errors, "No ECU");
            show(&d);
            if (!discover()) {
                vTaskDelay(pdMS_TO_TICKS(OBD_RETRY_MS));
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
        vTaskDelayUntil(&start, pdMS_TO_TICKS(OBD_CYCLE_MS));
    }
}

esp_err_t obd_start(void)
{
    s_rx = xQueueCreate(8, sizeof(rx_msg_t));
    ESP_RETURN_ON_FALSE(s_rx, ESP_ERR_NO_MEM, TAG, "queue");
    ESP_RETURN_ON_ERROR(board_can_new(CONFIG_DASH_CAN_BITRATE, &s_node), TAG, "can node");

    const twai_mask_filter_config_t filter = {.id = OBD_RESP_ID, .mask = OBD_RESP_MASK};
    ESP_RETURN_ON_ERROR(twai_node_config_mask_filter(s_node, 0, &filter), TAG, "filter");
    const twai_event_callbacks_t cbs = {.on_rx_done = on_rx};
    ESP_RETURN_ON_ERROR(twai_node_register_event_callbacks(s_node, &cbs, NULL), TAG, "callbacks");
    ESP_RETURN_ON_ERROR(twai_node_enable(s_node), TAG, "enable");

    BaseType_t ok = xTaskCreate(obd_task, "obd", 4096, NULL, 5, NULL);
    ESP_RETURN_ON_FALSE(ok == pdPASS, ESP_ERR_NO_MEM, TAG, "task");
    return ESP_OK;
}
