// Passive CAN capture and the UART @D1 service. Hardware belongs to board.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOGGER_MAX_RANGES 16
// Sixteen inclusive 29-bit ranges, commas and terminating NUL.
#define LOGGER_FILTER_TEXT_SIZE (LOGGER_MAX_RANGES * 18)
#define LOGGER_SESSION_TEXT_SIZE 17

typedef enum {
    LOGGER_MODE_DASHBOARD,
    LOGGER_MODE_IDLE,
    LOGGER_MODE_CAPTURING,
} logger_mode_t;

typedef enum {
    LOGGER_IDS_NONE,
    LOGGER_IDS_11,
    LOGGER_IDS_29,
    LOGGER_IDS_BOTH,
} logger_ids_t;

typedef enum {
    LOGGER_REASON_NONE,
    LOGGER_REASON_DURATION,
    LOGGER_REASON_CTRL_C,
    LOGGER_REASON_HOST_STOP,
    LOGGER_REASON_USER_EXIT,
    LOGGER_REASON_LINK_LOSS,
    LOGGER_REASON_HARDWARE_ERROR,
} logger_reason_t;

typedef struct {
    logger_mode_t mode;
    char sid[LOGGER_SESSION_TEXT_SIZE]; // "none" until the first successful START
    uint64_t device_us;                // monotonic microseconds since boot
    uint32_t bitrate;                  // effective, zero before successful START
    uint32_t requested_bitrate;
    logger_ids_t ids;
    char filter[LOGGER_FILTER_TEXT_SIZE]; // normalized, "all", or "none"
    bool listen_only;                  // always true, including idle snapshots
    uint64_t duration_ms;              // zero means no device duration deadline
    uint64_t seq;                      // assigned after filtering, includes drop-new
    uint64_t tx_seq;                   // last whole frame drained to UART TX
    uint64_t ack;                      // last frame acknowledged by the CLI
    uint64_t sw_drops;                 // exact queue-full drop-new count
    uint64_t hw_overruns;              // detected overflow events, NOT lost frames
    bool tx;                          // capture UART transmission healthy/enabled
    bool client;                      // valid current-session ACK/PING, never TX-inferred
    logger_reason_t reason;
    esp_err_t error;                   // latest lifecycle/UART hardware error
} logger_status_t;

// Call once during bring-up. Starts discovery/status service, never enters Logger.
// sdkconfig.defaults must disable the runtime console. No LVGL calls are made.
esp_err_t logger_init(void);

// Controller only, outside the LVGL lock, AFTER obd_stop() has succeeded.
// Manual screen entry grants permission for subsequent explicit UART STARTs.
esp_err_t logger_enter(void);

// Stop physical capture immediately, then drain accepted frames before END with
// user_exit. Returns ESP_OK ONLY when the CAN node/callbacks have been released,
// so the controller can safely restart OBD. UART failure is recorded in status;
// it does not prevent returning ownership once CAN is safely quiesced.
// Synchronous control waits are bounded; on timeout do not restart OBD.
esp_err_t logger_leave(void);

// Short-lock copy; safe from the UI task. No hardware access or UART I/O.
void logger_get_status(logger_status_t *out);

const char *logger_mode_name(logger_mode_t mode);
const char *logger_ids_name(logger_ids_t ids);
const char *logger_reason_name(logger_reason_t reason);

#ifdef __cplusplus
}
#endif
