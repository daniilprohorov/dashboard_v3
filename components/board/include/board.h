// Waveshare ESP32-S3-Touch-LCD-7B board support.
// Pin map and protocol: docs/hardware.md.
#pragma once

#include <stdbool.h>
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

// Selects CAN on GPIO19/20 (EXIO5) and creates a disabled TWAI node at `bitrate`.
// Caller configures filters/callbacks, then twai_node_enable().
esp_err_t board_can_new(uint32_t bitrate, twai_node_handle_t *out);

// Battery voltage via expander ADC (3:1 divider), millivolts.
esp_err_t board_battery_mv(uint32_t *out_mv);

#ifdef __cplusplus
}
#endif
