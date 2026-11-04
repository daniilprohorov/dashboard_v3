// Waveshare ESP32-S3-Touch-LCD-7B board support.
// Pin map and protocol: docs/hardware.md.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"
#include "esp_twai.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_LCD_H_RES 1024
#define BOARD_LCD_V_RES 600

// Expander (I2C 0x24) output lines.
typedef enum {
    BOARD_EXIO_TP_RST = 1,
    BOARD_EXIO_BL_EN = 2,
    BOARD_EXIO_LCD_RST = 3,
    BOARD_EXIO_SD_CS = 4,
    BOARD_EXIO_USB_CAN_SEL = 5, // 0 = USB on GPIO19/20, 1 = CAN (TWAI) on GPIO19/20
    BOARD_EXIO_LCD_VDD_EN = 6,
} board_exio_t;

typedef struct {
    esp_lcd_panel_handle_t panel;
    esp_lcd_touch_handle_t touch; // NULL on the non-touch variant
    i2c_master_bus_handle_t i2c;  // shared bus GPIO8/9: expander 0x24, GT911 0x5D, user devices
} board_t;

// Brings up I2C, IO expander, RGB panel (num_fbs frame buffers in PSRAM) and GT911.
// Backlight stays off until board_backlight_set().
esp_err_t board_init(uint8_t num_fbs, board_t *out);

esp_err_t board_exio_set(board_exio_t line, bool level);

// 0 = off, 1..100 = brightness percent.
esp_err_t board_backlight_set(uint8_t percent);

// Exact Classic CAN timing on ESP32-S3 / IDF v5.5.5:
// bitrate = 80 MHz / (BRP * (1 + TSEG1 + TSEG2)), BRP even in [2,16384],
// TSEG1 in [1,16], TSEG2 in [1,8]. Only integer-exact rates in [1000,1000000]
// are accepted; validation has no hardware side effects. ESP_ERR_INVALID_ARG
// means out of range or unreachable (never rounded).
esp_err_t board_can_validate_bitrate(uint32_t bitrate);

// Selects CAN on GPIO19/20 (EXIO5), then creates an explicitly timed, disabled
// node. listen_only=true disables all transmissions, including ACK and error
// frames; the IDF driver also rejects transmit calls on that node.
// Requires board_init(). A live node must be disabled/deleted before creating
// another; the single-controller driver's allocation error is propagated.
// Caller owns filters, callbacks and enable; release through board_can_release().
// *out is NULL on failure, except if cleanup itself fails: then *out retains
// the disabled node so the caller can retry deletion; cleanup error is returned.
esp_err_t board_can_create(uint32_t bitrate, bool listen_only, twai_node_handle_t *out);

// Disable then delete a caller-owned node, including IDF's already-BUS_OFF
// stopped-state quirk. A NULL *node is already released. On failure retain the
// handle and its enabled state for retry; only successful deletion clears it.
// Successful return joins the node's ISR before callback context can be reused.
esp_err_t board_can_release(twai_node_handle_t *node, bool *enabled);

// Atomic, task-context snapshot of boot-cumulative detected RX-overrun events
// for the single CAN controller, including earlier OBD operation. Baseline
// after node creation and before enable; capture count is current - baseline,
// with a final snapshot after disabling the node. Never reset by node deletion.
// Counts IDF's detected overrun markers, NOT an exact number of lost frames;
// zero means no markers observed, not proof of lossless hardware reception.
// Private HAL/linker hook is deliberately pinned to ESP32-S3 + IDF v5.5.5;
// upgrading IDF requires auditing that dependency before this code will build.
esp_err_t board_can_get_overruns(uint64_t *out_events);

#define BOARD_UART_BAUD 2000000

// Runtime protocol UART0: GPIO43 TX, GPIO44 RX, 2M8N1, no hardware/software
// flow control. Call once at boot, before the logger task starts. Requires
// CONFIG_ESP_CONSOLE_NONE; otherwise returns ESP_ERR_INVALID_STATE unchanged.
// Suppresses runtime ESP_LOG/ROM print channels until reset. Never touches
// BOOT/RESET, DTR/RTS or native USB; reset restores ROM flashing behavior.
// A previously installed UART0 driver is an ownership error, not adopted.
// Successful initialization is idempotent. No UART allocation occurs per I/O.
esp_err_t board_uart_init(void);

// All UART operations belong to ONE task, serialized, never from an ISR.
// Other code must not use UART0 directly. Timeouts are overall milliseconds,
// not per-byte waits; zero polls. Scheduling can add up to a tick of latency.
// Read returns once any bytes are available; ESP_ERR_TIMEOUT means no bytes.
// RX overflow, break, framing/parity errors return ESP_FAIL and discard the
// corrupted RX stream; caller must discard any partially assembled command.
esp_err_t board_uart_read(void *buffer, size_t capacity, size_t *out_read, uint32_t timeout_ms);

// Bounded FIFO writes (no unbounded uart_write_bytes). *out_written counts
// bytes accepted into the FIFO, including on error/timeout; do not blindly
// retry a partial protocol line. ESP_OK means ALL bytes reached UART TX idle,
// NOT that CH343/host received them. Timeout can leave accepted bytes draining.
esp_err_t board_uart_write(const void *buffer, size_t length, size_t *out_written, uint32_t timeout_ms);

// Battery voltage via expander ADC (3:1 divider), millivolts.
esp_err_t board_battery_mv(uint32_t *out_mv);

#ifdef __cplusplus
}
#endif
