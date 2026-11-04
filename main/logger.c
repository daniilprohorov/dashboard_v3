#include "logger.h"
#include "logger_protocol.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "board.h"
#include "esp_app_desc.h"
#include "esp_attr.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define FRAME_QUEUE_LENGTH 128
#define LOGGER_TASK_STACK_BYTES 8192
#define UART_WRITE_TIMEOUT_MS 20
#define CONTROL_TIMEOUT_MS 5000
#define HEARTBEAT_US UINT64_C(3000000)
#define STATUS_INTERVAL_US UINT64_C(1000000)

typedef struct {
    uint64_t seq;
    uint64_t us;
    uint32_t id;
    uint8_t bits;
    uint8_t rtr;
    uint8_t dlc;
    uint8_t data[8];
} logger_frame_t;

typedef enum { CONTROL_ENTER, CONTROL_LEAVE } control_t;

// Queue storage, callbacks' data and the task stack stay in internal RAM.
static DRAM_ATTR StaticQueue_t s_frame_queue_storage;
static DRAM_ATTR uint8_t s_frame_queue_bytes[FRAME_QUEUE_LENGTH * sizeof(logger_frame_t)];
static DRAM_ATTR StaticQueue_t s_control_queue_storage;
static DRAM_ATTR uint8_t s_control_queue_bytes[sizeof(control_t)];
static DRAM_ATTR StackType_t s_task_stack[LOGGER_TASK_STACK_BYTES / sizeof(StackType_t)];
static DRAM_ATTR StaticTask_t s_task_storage;
static StaticSemaphore_t s_api_mutex_storage;
static StaticSemaphore_t s_completion_storage;
static QueueHandle_t s_frames;
static QueueHandle_t s_controls;
static SemaphoreHandle_t s_api_mutex;
static SemaphoreHandle_t s_completion;
static TaskHandle_t s_task;

static portMUX_TYPE s_snapshot_lock = portMUX_INITIALIZER_UNLOCKED;
static portMUX_TYPE s_rx_lock = portMUX_INITIALIZER_UNLOCKED;
static portMUX_TYPE s_control_lock = portMUX_INITIALIZER_UNLOCKED;
static logger_status_t s_snapshot = {
    .mode = LOGGER_MODE_DASHBOARD, .sid = "none", .ids = LOGGER_IDS_NONE,
    .filter = "none", .listen_only = true,
};
// All non-ISR state below is owned exclusively by logger_task.
static logger_status_t s_status = {
    .mode = LOGGER_MODE_DASHBOARD, .sid = "none", .ids = LOGGER_IDS_NONE,
    .filter = "none", .listen_only = true,
};
static logger_config_t s_config;
static twai_node_handle_t s_node;
static bool s_node_enabled;
static bool s_active;
static bool s_uart_broken;
static bool s_have_overrun_baseline;
static uint64_t s_overrun_baseline;
static uint64_t s_started_us;
static uint64_t s_deadline_us;
static uint64_t s_last_service_us;
static uint64_t s_last_status_us;
static uint32_t s_boot_nonce;
static uint32_t s_session_counter;
static char s_firmware[64];
static char s_tx_line[LOGGER_LINE_MAX + 2];
static char s_rx_line[LOGGER_LINE_MAX + 1];
static size_t s_rx_used;
static bool s_rx_invalid;
static bool s_rx_cr;
// Protected by s_rx_lock. Config stays immutable until node deletion joins ISR.
static DRAM_ATTR bool s_accepting;
static DRAM_ATTR bool s_rx_fault;
static DRAM_ATTR uint64_t s_assigned;
static DRAM_ATTR uint64_t s_sw_drops;
// Protected by s_control_lock. Pending stays set until the API consumes the
// completion token, including after a caller times out; no caller pointers.
static bool s_control_pending;
static esp_err_t s_control_result;

const char *logger_mode_name(logger_mode_t mode)
{
    switch (mode) {
    case LOGGER_MODE_DASHBOARD: return "dashboard";
    case LOGGER_MODE_IDLE: return "logger_idle";
    case LOGGER_MODE_CAPTURING: return "capturing";
    }
    return "dashboard";
}

const char *logger_ids_name(logger_ids_t ids)
{
    switch (ids) {
    case LOGGER_IDS_NONE: return "none";
    case LOGGER_IDS_11: return "11";
    case LOGGER_IDS_29: return "29";
    case LOGGER_IDS_BOTH: return "both";
    }
    return "none";
}

const char *logger_reason_name(logger_reason_t reason)
{
    switch (reason) {
    case LOGGER_REASON_NONE: return "none";
    case LOGGER_REASON_DURATION: return "duration";
    case LOGGER_REASON_CTRL_C: return "ctrl_c";
    case LOGGER_REASON_HOST_STOP: return "host_stop";
    case LOGGER_REASON_USER_EXIT: return "user_exit";
    case LOGGER_REASON_LINK_LOSS: return "link_loss";
    case LOGGER_REASON_HARDWARE_ERROR: return "hardware_error";
    }
    return "hardware_error";
}

static void record_error(esp_err_t error)
{
    if (error != ESP_OK) {
        s_status.error = error;
    }
}

static void update_counts(void)
{
    portENTER_CRITICAL(&s_rx_lock);
    s_status.seq = s_assigned;
    s_status.sw_drops = s_sw_drops;
    portEXIT_CRITICAL(&s_rx_lock);
    if (s_have_overrun_baseline) {
        uint64_t current;
        esp_err_t error = board_can_get_overruns(&current);
        if (error == ESP_OK) {
            s_status.hw_overruns = current - s_overrun_baseline;
        } else {
            record_error(error);
            portENTER_CRITICAL(&s_rx_lock);
            s_rx_fault = true;
            portEXIT_CRITICAL(&s_rx_lock);
        }
    }
}

static void publish_status(void)
{
    update_counts();
    s_status.device_us = (uint64_t)esp_timer_get_time();
    portENTER_CRITICAL(&s_snapshot_lock);
    s_snapshot = s_status;
    portEXIT_CRITICAL(&s_snapshot_lock);
}

void logger_get_status(logger_status_t *out)
{
    if (!out) {
        return;
    }
    portENTER_CRITICAL(&s_snapshot_lock);
    *out = s_snapshot;
    portEXIT_CRITICAL(&s_snapshot_lock);
    out->device_us = (uint64_t)esp_timer_get_time();
}

static bool write_bytes(const void *bytes, size_t length)
{
    size_t written = 0;
    esp_err_t error = board_uart_write(bytes, length, &written, UART_WRITE_TIMEOUT_MS);
    if (error != ESP_OK || written != length) {
        record_error(error == ESP_OK ? ESP_FAIL : error);
        s_uart_broken = true;
        s_status.tx = false;
        return false;
    }
    return true;
}

static bool write_line(const char *format, ...)
{
    if (s_uart_broken) {
        return false;
    }
    va_list args;
    va_start(args, format);
    int length = vsnprintf(s_tx_line, sizeof s_tx_line, format, args);
    va_end(args);
    if (length < 0 || length > LOGGER_LINE_MAX) {
        record_error(ESP_ERR_INVALID_SIZE);
        s_uart_broken = true;
        s_status.tx = false;
        return false;
    }
    s_tx_line[length++] = '\n';
    // A partial line is never retried. Its session will terminate with an
    // honest transport error, not a fabricated successful END.
    return write_bytes(s_tx_line, (size_t)length);
}

static bool emit_common(const char *prefix)
{
    publish_status();
    return write_line("%s mode=%s sid=%s device_us=%" PRIu64
                      " bitrate=%" PRIu32 " requested_bitrate=%" PRIu32
                      " ids=%s filter=%s listen_only=1 seq=%" PRIu64
                      " tx_seq=%" PRIu64 " ack=%" PRIu64 " sw_drops=%" PRIu64
                      " hw_overruns=%" PRIu64 " hw_lost=unknown tx=%u client=%u reason=%s",
                      prefix, logger_mode_name(s_status.mode), s_status.sid, s_status.device_us,
                      s_status.bitrate, s_status.requested_bitrate, logger_ids_name(s_status.ids),
                      s_status.filter, s_status.seq, s_status.tx_seq, s_status.ack,
                      s_status.sw_drops, s_status.hw_overruns, (unsigned)s_status.tx,
                      (unsigned)s_status.client, logger_reason_name(s_status.reason));
}

static bool reply_error(uint32_t rid, const char *code)
{
    return write_line("@D1 R %" PRIu32 " ERR code=%s", rid, code);
}

static bool reply_ok(uint32_t rid)
{
    return write_line("@D1 R %" PRIu32 " OK", rid);
}

static bool IRAM_ATTR receive_frame(twai_node_handle_t node,
                                   const twai_rx_done_event_data_t *event, void *context)
{
    (void)event;
    (void)context;
    logger_frame_t message;
    twai_frame_t frame = {.buffer = message.data, .buffer_len = sizeof message.data};
    if (twai_node_receive_from_isr(node, &frame) != ESP_OK) {
        portENTER_CRITICAL_ISR(&s_rx_lock);
        s_rx_fault = true;
        portEXIT_CRITICAL_ISR(&s_rx_lock);
        return false;
    }
    // Software timestamp at ISR dequeue, in boot-monotonic microseconds. This
    // is NOT the wire SOF timestamp; interrupt/FIFO latency is unmeasured.
    message.us = (uint64_t)esp_timer_get_time();
    message.id = frame.header.id;
    message.bits = frame.header.ide ? 29 : 11;
    message.rtr = frame.header.rtr;
    // Classic CAN DLC codes 9..15 also encode eight payload bytes.
    message.dlc = frame.header.dlc > 8 ? 8 : (uint8_t)frame.header.dlc;

    portENTER_CRITICAL_ISR(&s_rx_lock);
    bool accepting = s_accepting;
    uint64_t deadline_us = s_deadline_us;
    portEXIT_CRITICAL_ISR(&s_rx_lock);
    if (!accepting || frame.header.fdf ||
        (deadline_us && message.us >= deadline_us) ||
        (s_config.ids == LOGGER_IDS_11 && message.bits != 11) ||
        (s_config.ids == LOGGER_IDS_29 && message.bits != 29)) {
        return false;
    }
    bool allowed = s_config.range_count == 0;
    for (size_t i = 0; !allowed && i < s_config.range_count; ++i) {
        allowed = message.id >= s_config.ranges[i].low && message.id <= s_config.ranges[i].high;
    }
    if (!allowed) {
        return false;
    }
    portENTER_CRITICAL_ISR(&s_rx_lock);
    accepting = s_accepting; // Exit can close acceptance during filtering.
    if (accepting) {
        message.seq = ++s_assigned;
    }
    portEXIT_CRITICAL_ISR(&s_rx_lock);
    if (!accepting) {
        return false;
    }
    // One node ISR serializes producers. Shutdown joins it before draining
    // the queue/resetting config, so no frame or drop can arrive after END.
    BaseType_t woken = pdFALSE;
    if (xQueueSendFromISR(s_frames, &message, &woken) != pdTRUE) {
        portENTER_CRITICAL_ISR(&s_rx_lock);
        ++s_sw_drops; // Drop NEW. Existing FIFO order is untouched.
        portEXIT_CRITICAL_ISR(&s_rx_lock);
    }
    return woken == pdTRUE;
}

static bool IRAM_ATTR state_changed(twai_node_handle_t node,
                                   const twai_state_change_event_data_t *event, void *context)
{
    (void)node;
    (void)context;
    if (event->new_sta == TWAI_ERROR_BUS_OFF) {
        portENTER_CRITICAL_ISR(&s_rx_lock);
        s_rx_fault = true;
        portEXIT_CRITICAL_ISR(&s_rx_lock);
    }
    return false;
}

static esp_err_t release_node(void)
{
    portENTER_CRITICAL(&s_rx_lock);
    s_accepting = false;
    portEXIT_CRITICAL(&s_rx_lock);
    esp_err_t error = board_can_release(&s_node, &s_node_enabled);
    // The successful delete joins callbacks before final software/hardware
    // snapshots. On failure keep ownership/baseline for a later retry.
    update_counts();
    if (error != ESP_OK) {
        record_error(error);
        return error;
    }
    s_have_overrun_baseline = false;
    return ESP_OK;
}

static bool emit_frame(const logger_frame_t *frame)
{
    char data[17];
    static const char hex[] = "0123456789ABCDEF";
    if (frame->rtr || !frame->dlc) {
        strcpy(data, "-");
    } else {
        for (unsigned i = 0; i < frame->dlc; ++i) {
            data[i * 2] = hex[frame->data[i] >> 4];
            data[i * 2 + 1] = hex[frame->data[i] & 15];
        }
        data[frame->dlc * 2] = '\0';
    }
    if (!write_line("@D1 F sid=%s seq=%" PRIu64 " us=%" PRIu64
                    " bits=%u id=%0*" PRIX32 " rtr=%u dlc=%u data=%s",
                    s_status.sid, frame->seq, frame->us, frame->bits,
                    frame->bits == 11 ? 3 : 8, frame->id, frame->rtr, frame->dlc, data)) {
        return false;
    }
    s_status.tx_seq = frame->seq;
    return true;
}

static esp_err_t stop_capture(logger_reason_t reason, bool leaving)
{
    // Physical capture/callback ownership stops BEFORE potentially slow UART.
    esp_err_t ownership = release_node();
    bool was_active = s_active;
    s_active = false;
    if (was_active) {
        logger_frame_t frame;
        while (xQueueReceive(s_frames, &frame, 0) == pdTRUE) {
            if (!s_uart_broken) {
                emit_frame(&frame);
            }
        }
        s_status.reason = ownership != ESP_OK || s_uart_broken || s_status.error != ESP_OK
                        ? LOGGER_REASON_HARDWARE_ERROR : reason;
        s_status.tx = false;
        s_status.client = false;
    }
    // Retain Logger ownership on a release error; the caller MUST NOT start OBD.
    if (ownership == ESP_OK) {
        s_status.mode = leaving ? LOGGER_MODE_DASHBOARD : LOGGER_MODE_IDLE;
    } else {
        s_status.mode = LOGGER_MODE_IDLE;
    }
    if (was_active && !s_uart_broken && !emit_common("@D1 E")) {
        s_status.reason = LOGGER_REASON_HARDWARE_ERROR;
    }
    publish_status();
    return ownership;
}

static bool current_session(const logger_request_t *request)
{
    return s_active && strcmp(request->sid, s_status.sid) == 0;
}

static void start_capture(const logger_request_t *request)
{
    if (s_status.mode == LOGGER_MODE_DASHBOARD) {
        reply_error(request->rid, "logger_required");
        return;
    }
    if (s_active || s_node) {
        reply_error(request->rid, "busy");
        return;
    }
    esp_err_t error = board_can_validate_bitrate(request->config.bitrate);
    if (error != ESP_OK) {
        if (error != ESP_ERR_INVALID_ARG) {
            record_error(error);
        }
        reply_error(request->rid, error == ESP_ERR_INVALID_ARG ? "invalid_bitrate" : "hardware_error");
        return;
    }
    uint64_t now = (uint64_t)esp_timer_get_time();
    if (request->config.duration_ms > ((uint64_t)INT64_MAX - now) / 1000) {
        reply_error(request->rid, "invalid_argument");
        return;
    }
    if (s_session_counter == UINT32_MAX) {
        record_error(ESP_ERR_INVALID_STATE); // Never reuse a sid within this boot.
        reply_error(request->rid, "hardware_error");
        return;
    }
    uint64_t overrun_baseline = 0;
    error = board_can_create(request->config.bitrate, true, &s_node);
    if (error == ESP_OK) {
        error = board_can_get_overruns(&overrun_baseline);
    }
    if (error == ESP_OK) {
        twai_event_callbacks_t callbacks = {.on_rx_done = receive_frame, .on_state_change = state_changed};
        error = twai_node_register_event_callbacks(s_node, &callbacks, NULL);
    }
    // No hardware filter is configured: the driver's default accepts both ID
    // formats. A configured mask, even zero, would enable IDF's format filter.
    if (error == ESP_OK) {
        xQueueReset(s_frames);
        portENTER_CRITICAL(&s_rx_lock);
        s_config = request->config;
        s_deadline_us = 0;
        s_rx_fault = false;
        s_accepting = false;
        portEXIT_CRITICAL(&s_rx_lock);
        // Mark possibly enabled BEFORE calling IDF: enable can fail part-way.
        s_node_enabled = true;
        error = twai_node_enable(s_node);
    }
    if (error != ESP_OK) {
        record_error(error);
        release_node();
        // Last successful config/counters remain authoritative on failure.
        reply_error(request->rid, "hardware_error");
        publish_status();
        return;
    }

    // Publish the new session only after every hardware step has succeeded.
    memset(&s_status, 0, sizeof s_status);
    s_status.mode = LOGGER_MODE_CAPTURING;
    snprintf(s_status.sid, sizeof s_status.sid, "%08" PRIX32 "%08" PRIX32,
             s_boot_nonce, ++s_session_counter);
    s_status.bitrate = request->config.bitrate;
    s_status.requested_bitrate = request->config.bitrate;
    s_status.ids = request->config.ids;
    strcpy(s_status.filter, request->config.filter);
    s_status.listen_only = true;
    s_status.duration_ms = request->config.duration_ms;
    s_status.tx = true;
    s_overrun_baseline = overrun_baseline;
    s_have_overrun_baseline = true;
    s_active = true;
    s_started_us = (uint64_t)esp_timer_get_time();
    s_last_service_us = s_started_us;
    s_last_status_us = s_started_us;
    portENTER_CRITICAL(&s_rx_lock);
    s_assigned = 0;
    s_sw_drops = 0;
    s_deadline_us = s_config.duration_ms ? s_started_us + s_config.duration_ms * 1000 : 0;
    s_accepting = true;
    portEXIT_CRITICAL(&s_rx_lock);
    // ISR may queue frames now, but only this task emits UART; the correlated
    // START response always precedes the first asynchronous status/frame.
    if (write_line("@D1 R %" PRIu32 " OK sid=%s device_us=%" PRIu64
                   " bitrate=%" PRIu32 " requested_bitrate=%" PRIu32
                   " ids=%s filter=%s listen_only=1",
                   request->rid, s_status.sid, s_started_us, s_status.bitrate,
                   s_status.requested_bitrate, logger_ids_name(s_status.ids), s_status.filter)) {
        emit_common("@D1 S");
    }
    publish_status();
}

static void handle_request(char *line, bool invalid)
{
    logger_request_t request;
    const char *error = logger_request_parse(line, &request);
    if (invalid) {
        error = "invalid_command";
    }
    // An explicit discovery request may re-establish framing after a failed
    // session. It cannot resume capture, replay frames, or retry the old line.
    if (!error && request.command == LOGGER_CMD_HELLO && s_uart_broken && !s_active) {
        s_uart_broken = false;
        if (!write_bytes("\n", 1)) {
            return;
        }
    }
    if (error) {
        reply_error(request.rid, error);
        return;
    }
    switch (request.command) {
    case LOGGER_CMD_HELLO:
        write_line("@D1 R %" PRIu32 " OK device=dashboard_v3 firmware=%s protocol=1 mode=%s device_us=%" PRIu64,
                   request.rid, s_firmware, logger_mode_name(s_status.mode), (uint64_t)esp_timer_get_time());
        break;
    case LOGGER_CMD_STATUS: {
        char prefix[40];
        snprintf(prefix, sizeof prefix, "@D1 R %" PRIu32 " OK", request.rid);
        emit_common(prefix);
        break;
    }
    case LOGGER_CMD_CAPS:
        write_line("@D1 R %" PRIu32 " OK min_bitrate=1000 max_bitrate=1000000 exact_only=1"
                   " id_types=11,29,both max_ranges=16 uart_baud=2000000 data_bits=8 parity=N"
                   " stop_bits=1 listen_only=1", request.rid);
        break;
    case LOGGER_CMD_START:
        start_capture(&request);
        break;
    case LOGGER_CMD_STOP:
        if (!current_session(&request)) {
            reply_error(request.rid, "invalid_session");
        } else {
            esp_err_t ownership = stop_capture(request.reason, false);
            if (ownership == ESP_OK && !s_uart_broken) {
                reply_ok(request.rid);
            } else {
                reply_error(request.rid, "hardware_error");
            }
        }
        break;
    case LOGGER_CMD_ACK:
    case LOGGER_CMD_PING:
        if (!current_session(&request)) {
            reply_error(request.rid, "invalid_session");
        } else if (request.command == LOGGER_CMD_ACK &&
                   (request.ack < s_status.ack || request.ack > s_status.tx_seq)) {
            reply_error(request.rid, "invalid_argument");
        } else {
            bool new_client = !s_status.client;
            if (request.command == LOGGER_CMD_ACK) {
                s_status.ack = request.ack;
            }
            s_last_service_us = (uint64_t)esp_timer_get_time();
            s_status.client = true;
            reply_ok(request.rid);
            if (new_client) {
                emit_common("@D1 S");
            }
        }
        break;
    }
}

static bool check_capture_deadline(void);

static void read_commands(void)
{
    uint8_t bytes[256];
    size_t count = 0;
    esp_err_t error = board_uart_read(bytes, sizeof bytes, &count, s_active ? 0 : 5);
    if (error == ESP_ERR_TIMEOUT) {
        return;
    }
    if (error != ESP_OK) {
        // HAL flushes corrupt RX bytes; do not splice them onto a partial line.
        s_rx_used = 0;
        s_rx_invalid = false;
        s_rx_cr = false;
        record_error(error);
        if (s_active) {
            stop_capture(LOGGER_REASON_HARDWARE_ERROR, false);
        }
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        unsigned char byte = bytes[i];
        if (byte == '\n') {
            s_rx_line[s_rx_used] = '\0';
            check_capture_deadline();
            handle_request(s_rx_line, s_rx_invalid);
            s_rx_used = 0;
            s_rx_invalid = false;
            s_rx_cr = false;
            if (s_active && s_uart_broken) {
                stop_capture(LOGGER_REASON_HARDWARE_ERROR, false);
            }
        } else if (byte == '\r' && !s_rx_cr) {
            s_rx_cr = true;
        } else {
            if (s_rx_cr || byte < 0x20 || byte > 0x7E) {
                s_rx_invalid = true;
            }
            if (s_rx_used == LOGGER_LINE_MAX) {
                s_rx_invalid = true; // Discard through LF, never parse a suffix.
            } else {
                s_rx_line[s_rx_used++] = byte >= 0x20 && byte <= 0x7E ? (char)byte : '?';
            }
        }
    }
}

static bool check_capture_deadline(void)
{
    if (!s_active) {
        return false;
    }
    bool fault;
    portENTER_CRITICAL(&s_rx_lock);
    fault = s_rx_fault;
    portEXIT_CRITICAL(&s_rx_lock);
    uint64_t now = (uint64_t)esp_timer_get_time();
    if (fault || s_uart_broken) {
        if (fault) {
            record_error(ESP_FAIL);
        }
        stop_capture(LOGGER_REASON_HARDWARE_ERROR, false);
    } else if (s_deadline_us && now >= s_deadline_us) {
        stop_capture(LOGGER_REASON_DURATION, false);
    } else if (now - s_last_service_us >= HEARTBEAT_US) {
        stop_capture(LOGGER_REASON_LINK_LOSS, false);
    }
    return s_active;
}

static void handle_control(void)
{
    control_t control;
    if (xQueueReceive(s_controls, &control, 0) != pdTRUE) {
        return;
    }
    esp_err_t result = ESP_OK;
    if (control == CONTROL_ENTER) {
        if (s_status.mode == LOGGER_MODE_DASHBOARD) {
            s_status.mode = LOGGER_MODE_IDLE;
        }
        emit_common("@D1 S");
    } else {
        bool was_active = s_active;
        result = stop_capture(LOGGER_REASON_USER_EXIT, true);
        if (!was_active && result == ESP_OK && !s_uart_broken) {
            emit_common("@D1 S"); // Active Exit already emitted terminal E.
        }
    }
    publish_status();
    portENTER_CRITICAL(&s_control_lock);
    s_control_result = result;
    portEXIT_CRITICAL(&s_control_lock);
    xSemaphoreGive(s_completion);
}

static void logger_task(void *argument)
{
    (void)argument;
    for (;;) {
        handle_control(); // UI exit wins over queued serial commands/frames.
        check_capture_deadline();
        read_commands();
        check_capture_deadline();
        logger_frame_t frame;
        unsigned emitted = 0;
        while (s_active && emitted++ < 16 && xQueueReceive(s_frames, &frame, 0) == pdTRUE) {
            emit_frame(&frame);
            handle_control();
            if (!check_capture_deadline()) {
                break;
            }
        }
        if (s_active && (uint64_t)esp_timer_get_time() - s_last_status_us >= STATUS_INTERVAL_US) {
            s_last_status_us = (uint64_t)esp_timer_get_time();
            emit_common("@D1 S");
            check_capture_deadline();
        }
        publish_status();
        // A bounded poll also services commands/UI on a silent CAN bus. Avoid
        // starving idle/LVGL when active with no incoming frames or UART bytes.
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

esp_err_t logger_init(void)
{
    if (s_task) {
        return ESP_OK;
    }
    esp_err_t error = board_uart_init();
    if (error != ESP_OK) {
        s_status.error = error;
        publish_status();
        return error;
    }
    s_frames = xQueueCreateStatic(FRAME_QUEUE_LENGTH, sizeof(logger_frame_t),
                                 s_frame_queue_bytes, &s_frame_queue_storage);
    s_controls = xQueueCreateStatic(1, sizeof(control_t), s_control_queue_bytes, &s_control_queue_storage);
    s_api_mutex = xSemaphoreCreateMutexStatic(&s_api_mutex_storage);
    s_completion = xSemaphoreCreateBinaryStatic(&s_completion_storage);
    s_boot_nonce = esp_random();
    const char *version = esp_app_get_description()->version;
    size_t i = 0;
    for (; version[i] && i + 1 < sizeof s_firmware; ++i) {
        unsigned char c = (unsigned char)version[i];
        s_firmware[i] = c > 0x20 && c < 0x7F && c != '=' ? (char)c : '_';
    }
    s_firmware[i] = '\0';
    if (!i) {
        strcpy(s_firmware, "unknown");
    }
    publish_status();
    // ESP-IDF's xTaskCreateStatic stack-depth argument is bytes (not words).
    s_task = xTaskCreateStatic(logger_task, "can_logger", LOGGER_TASK_STACK_BYTES,
                               NULL, 5, s_task_stack, &s_task_storage);
    if (!s_task) {
        s_status.error = ESP_ERR_NO_MEM;
        publish_status();
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

static esp_err_t control_request(control_t control)
{
    if (!s_task) {
        return ESP_ERR_INVALID_STATE;
    }
    if (xSemaphoreTake(s_api_mutex, pdMS_TO_TICKS(CONTROL_TIMEOUT_MS)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    // Only this serialized API consumes completions. Pending is not cleared
    // by the task before Give: a new caller must never consume a late token as
    // its own result. A timed-out transfer stays pending until its token arrives.
    bool completed = xSemaphoreTake(s_completion, 0) == pdTRUE;
    portENTER_CRITICAL(&s_control_lock);
    if (completed) {
        s_control_pending = false;
    }
    bool pending = s_control_pending;
    if (!pending) {
        s_control_pending = true;
    }
    portEXIT_CRITICAL(&s_control_lock);
    esp_err_t result;
    if (pending) {
        result = ESP_ERR_TIMEOUT;
    } else if (xQueueSend(s_controls, &control, 0) != pdTRUE) {
        portENTER_CRITICAL(&s_control_lock);
        s_control_pending = false;
        portEXIT_CRITICAL(&s_control_lock);
        result = ESP_ERR_INVALID_STATE;
    } else if (xSemaphoreTake(s_completion, pdMS_TO_TICKS(CONTROL_TIMEOUT_MS)) != pdTRUE) {
        // Storage remains alive. Never cancel a possibly-running ownership
        // transfer or let the caller assume that OBD can already restart.
        result = ESP_ERR_TIMEOUT;
    } else {
        portENTER_CRITICAL(&s_control_lock);
        result = s_control_result;
        s_control_pending = false;
        portEXIT_CRITICAL(&s_control_lock);
    }
    xSemaphoreGive(s_api_mutex);
    return result;
}

esp_err_t logger_enter(void)
{
    return control_request(CONTROL_ENTER);
}

esp_err_t logger_leave(void)
{
    return control_request(CONTROL_LEAVE);
}
