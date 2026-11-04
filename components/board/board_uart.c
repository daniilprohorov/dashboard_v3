#include "board.h"

#include <limits.h>
#include <stdarg.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#define PROTOCOL_UART UART_NUM_0
#define UART_RX_BYTES 4096
#define UART_EVENT_DEPTH 32

static bool s_uart_ready;
static QueueHandle_t s_uart_events;

static int discard_log(const char *format, va_list args)
{
    (void)format;
    (void)args;
    return 0;
}

esp_err_t board_uart_init(void)
{
    // Do not steal a live console/monitor driver. Console NONE affects only
    // application startup, not the reset ROM's UART download protocol.
#ifndef CONFIG_ESP_CONSOLE_NONE
    return ESP_ERR_INVALID_STATE;
#endif
    if (s_uart_ready) {
        return ESP_OK;
    }
    if (uart_is_driver_installed(PROTOCOL_UART)) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_log_level_set("*", ESP_LOG_NONE);
    esp_log_set_vprintf(discard_log);
    // Also silence ESP_EARLY_LOG/ESP_DRAM_LOG and direct ROM printf channels.
    // These RAM-installed hooks are restored by reset, never persisted.
    esp_rom_install_channel_putc(1, NULL);
    esp_rom_install_channel_putc(2, NULL);

    // No TX ring buffer: uart_tx_chars() below only fills available FIFO
    // space, unlike uart_write_bytes() which can wait indefinitely.
    esp_err_t err = uart_driver_install(PROTOCOL_UART, UART_RX_BYTES, 0,
                                       UART_EVENT_DEPTH, &s_uart_events, 0);
    if (err != ESP_OK) {
        return err;
    }
    const uart_config_t cfg = {
        .baud_rate = BOARD_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        // 40 MHz crystal / 20 is exactly 2 Mbaud and independent of APB DFS.
        .source_clk = UART_SCLK_XTAL,
    };
    err = uart_param_config(PROTOCOL_UART, &cfg);
    if (err == ESP_OK) {
        // Never connect/toggle RTS or CTS: the CH343 auto-download circuit
        // remains under the host's control, and BOOT/RESET are untouched.
        err = uart_set_pin(PROTOCOL_UART, GPIO_NUM_43, GPIO_NUM_44,
                           UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    }
    if (err == ESP_OK) {
        err = uart_set_sw_flow_ctrl(PROTOCOL_UART, false, 0, 0);
    }
    if (err != ESP_OK) {
        const esp_err_t cleanup_err = uart_driver_delete(PROTOCOL_UART);
        s_uart_events = NULL;
        return cleanup_err == ESP_OK ? err : cleanup_err;
    }
    s_uart_ready = true;
    return ESP_OK;
}

static esp_err_t uart_rx_health(void)
{
    // Event delivery itself is bounded. A saturated event queue cannot prove
    // stream integrity, even if an overflow event was dropped behind DATA.
    bool failed = uxQueueMessagesWaiting(s_uart_events) >= UART_EVENT_DEPTH;
    uart_event_t event;
    for (unsigned i = 0; i < UART_EVENT_DEPTH; ++i) {
        if (xQueueReceive(s_uart_events, &event, 0) != pdTRUE) {
            break;
        }
        switch (event.type) {
        case UART_FIFO_OVF:
        case UART_BUFFER_FULL:
        case UART_BREAK:
        case UART_PARITY_ERR:
        case UART_FRAME_ERR:
            failed = true;
            break;
        default:
            break;
        }
    }
    if (failed) {
        // Never expose a silently spliced command after lost/corrupt RX bytes.
        const esp_err_t err = uart_flush_input(PROTOCOL_UART);
        xQueueReset(s_uart_events);
        return err == ESP_OK ? ESP_FAIL : err;
    }
    return ESP_OK;
}

esp_err_t board_uart_read(void *buffer, size_t capacity, size_t *out_read, uint32_t timeout_ms)
{
    if (!out_read) {
        return ESP_ERR_INVALID_ARG;
    }
    *out_read = 0;
    if (!buffer || capacity == 0 || capacity > INT_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!s_uart_ready) {
        return ESP_ERR_INVALID_STATE;
    }
    const int64_t deadline = esp_timer_get_time() + (int64_t)timeout_ms * 1000;
    for (;;) {
        esp_err_t err = uart_rx_health();
        if (err != ESP_OK) {
            return err;
        }
        // IDF's blocking read applies its timeout again for each chunk. Poll
        // instead and enforce one deadline here. With the sole-owner contract
        // neither the RX nor TX driver's private mutex can be contended.
        // Bound work even if a large caller buffer meets continuous input.
        const uint32_t chunk = capacity > 256 ? 256 : (uint32_t)capacity;
        const int count = uart_read_bytes(PROTOCOL_UART, buffer, chunk, 0);
        if (count < 0) {
            return ESP_FAIL;
        }
        err = uart_rx_health();
        if (err != ESP_OK) {
            return err;
        }
        if (count > 0) {
            *out_read = (size_t)count;
            return ESP_OK;
        }
        if (esp_timer_get_time() >= deadline) {
            return ESP_ERR_TIMEOUT;
        }
        vTaskDelay(1);
        if (esp_timer_get_time() >= deadline) {
            return ESP_ERR_TIMEOUT;
        }
    }
}

esp_err_t board_uart_write(const void *buffer, size_t length, size_t *out_written, uint32_t timeout_ms)
{
    if (!out_written) {
        return ESP_ERR_INVALID_ARG;
    }
    *out_written = 0;
    if ((!buffer && length != 0) || length > INT_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!s_uart_ready) {
        return ESP_ERR_INVALID_STATE;
    }
    const int64_t deadline = esp_timer_get_time() + (int64_t)timeout_ms * 1000;
    for (;;) {
        esp_err_t err = uart_rx_health();
        if (err != ESP_OK) {
            return err;
        }
        if (*out_written < length) {
            const int count = uart_tx_chars(PROTOCOL_UART,
                                           (const char *)buffer + *out_written,
                                           length - *out_written);
            if (count < 0) {
                return ESP_FAIL;
            }
            *out_written += (size_t)count;
        }
        if (*out_written == length) {
            err = uart_wait_tx_done(PROTOCOL_UART, 0);
            if (err != ESP_ERR_TIMEOUT) {
                return err;
            }
        }
        if (esp_timer_get_time() >= deadline) {
            return ESP_ERR_TIMEOUT;
        }
        vTaskDelay(1);
        if (esp_timer_get_time() >= deadline) {
            return ESP_ERR_TIMEOUT;
        }
    }
}
