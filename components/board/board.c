#include "board.h"

#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_twai_onchip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "board";

#define I2C_SDA GPIO_NUM_8
#define I2C_SCL GPIO_NUM_9
#define I2C_HZ (400 * 1000)
#define I2C_TIMEOUT_MS 100

#define TOUCH_INT GPIO_NUM_4

// TWAI on GPIO19/20 via TJA1051; only valid while EXIO5 (USB_CAN_SEL) = 1.
#define CAN_TX GPIO_NUM_20
#define CAN_RX GPIO_NUM_19

// Expander protocol (vendor firmware, not a catalog chip): write [reg, value].
#define EXIO_ADDR 0x24
#define EXIO_REG_MODE 0x02 // bit=1 -> output
#define EXIO_REG_OUT 0x03
#define EXIO_REG_PWM 0x05 // backlight dimming, 0..255, higher = darker
#define EXIO_REG_ADC 0x06 // u16 LE, 10-bit, 3.3 V ref

#define LCD_PCLK_HZ (24 * 1000 * 1000)

static i2c_master_bus_handle_t s_bus;
static i2c_master_dev_handle_t s_exio;
// All lines high (panel powered, out of reset, CAN selected) except backlight.
static uint8_t s_exio_out = 0xFF & ~(1u << BOARD_EXIO_BL_EN);

static esp_err_t exio_write(uint8_t reg, uint8_t value)
{
    const uint8_t buf[2] = {reg, value};
    return i2c_master_transmit(s_exio, buf, sizeof(buf), I2C_TIMEOUT_MS);
}

esp_err_t board_exio_set(board_exio_t line, bool level)
{
    uint8_t next = level ? (s_exio_out | (1u << line)) : (s_exio_out & ~(1u << line));
    ESP_RETURN_ON_ERROR(exio_write(EXIO_REG_OUT, next), TAG, "exio out");
    s_exio_out = next;
    return ESP_OK;
}

esp_err_t board_backlight_set(uint8_t percent)
{
    if (percent == 0) {
        return board_exio_set(BOARD_EXIO_BL_EN, false);
    }
    if (percent > 100) {
        percent = 100;
    }
    // Vendor caps dimming at 97% to keep the boost converter running.
    uint8_t dim = 100 - percent;
    if (dim > 97) {
        dim = 97;
    }
    ESP_RETURN_ON_ERROR(exio_write(EXIO_REG_PWM, (uint8_t)(dim * 255 / 100)), TAG, "exio pwm");
    return board_exio_set(BOARD_EXIO_BL_EN, true);
}

esp_err_t board_battery_mv(uint32_t *out_mv)
{
    uint8_t reg = EXIO_REG_ADC;
    uint8_t raw[2];
    ESP_RETURN_ON_ERROR(i2c_master_transmit_receive(s_exio, &reg, 1, raw, sizeof(raw), I2C_TIMEOUT_MS), TAG, "exio adc");
    const uint32_t code = (uint32_t)raw[1] << 8 | raw[0];
    *out_mv = code * 3 * 3300 / 1023;
    return ESP_OK;
}

static esp_err_t init_i2c(void)
{
    const i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_SCL,
        .sda_io_num = I2C_SDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_cfg, &s_bus), TAG, "i2c bus");
    const i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = EXIO_ADDR,
        .scl_speed_hz = I2C_HZ,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(s_bus, &dev_cfg, &s_exio), TAG, "exio dev");
    ESP_RETURN_ON_ERROR(exio_write(EXIO_REG_MODE, 0xFF), TAG, "exio mode");
    return exio_write(EXIO_REG_OUT, s_exio_out);
}

static esp_err_t init_panel(uint8_t num_fbs, esp_lcd_panel_handle_t *out)
{
    // Timings and pin order from vendor example 17_lvgl_v9_demo (ST7262 panel).
    const esp_lcd_rgb_panel_config_t cfg = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .timings = {
            .pclk_hz = LCD_PCLK_HZ,
            .h_res = BOARD_LCD_H_RES,
            .v_res = BOARD_LCD_V_RES,
            .hsync_pulse_width = 162,
            .hsync_back_porch = 152,
            .hsync_front_porch = 48,
            .vsync_pulse_width = 45,
            .vsync_back_porch = 13,
            .vsync_front_porch = 3,
            .flags.pclk_active_neg = 1,
        },
        .data_width = 16,
        .bits_per_pixel = 16,
        .num_fbs = num_fbs,
        .bounce_buffer_size_px = BOARD_LCD_H_RES * 20,
        .dma_burst_size = 64,
        .hsync_gpio_num = GPIO_NUM_46,
        .vsync_gpio_num = GPIO_NUM_3,
        .de_gpio_num = GPIO_NUM_5,
        .pclk_gpio_num = GPIO_NUM_7,
        .disp_gpio_num = GPIO_NUM_NC,
        .data_gpio_nums = {
            // B3..B7
            GPIO_NUM_14, GPIO_NUM_38, GPIO_NUM_18, GPIO_NUM_17, GPIO_NUM_10,
            // G2..G7
            GPIO_NUM_39, GPIO_NUM_0, GPIO_NUM_45, GPIO_NUM_48, GPIO_NUM_47, GPIO_NUM_21,
            // R3..R7
            GPIO_NUM_1, GPIO_NUM_2, GPIO_NUM_42, GPIO_NUM_41, GPIO_NUM_40,
        },
        .flags.fb_in_psram = 1,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_rgb_panel(&cfg, out), TAG, "rgb panel");
    return esp_lcd_panel_init(*out);
}

static esp_err_t init_touch(esp_lcd_touch_handle_t *out)
{
    // GT911 address strap: INT low while releasing reset -> 0x5D.
    const gpio_config_t int_cfg = {.pin_bit_mask = 1ULL << TOUCH_INT, .mode = GPIO_MODE_OUTPUT};
    ESP_RETURN_ON_ERROR(gpio_config(&int_cfg), TAG, "tp int");
    ESP_RETURN_ON_ERROR(board_exio_set(BOARD_EXIO_TP_RST, false), TAG, "tp rst");
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(TOUCH_INT, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_RETURN_ON_ERROR(board_exio_set(BOARD_EXIO_TP_RST, true), TAG, "tp rst");
    vTaskDelay(pdMS_TO_TICKS(200));

    esp_lcd_panel_io_handle_t io;
    esp_lcd_panel_io_i2c_config_t io_cfg = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    io_cfg.scl_speed_hz = I2C_HZ;
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c(s_bus, &io_cfg, &io), TAG, "tp io");

    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = BOARD_LCD_H_RES,
        .y_max = BOARD_LCD_V_RES,
        .rst_gpio_num = GPIO_NUM_NC,
        .int_gpio_num = TOUCH_INT,
    };
    return esp_lcd_touch_new_i2c_gt911(io, &tp_cfg, out);
}

esp_err_t board_init(uint8_t num_fbs, board_t *out)
{
    *out = (board_t){0};
    ESP_RETURN_ON_ERROR(init_i2c(), TAG, "i2c");
    ESP_RETURN_ON_ERROR(init_panel(num_fbs, &out->panel), TAG, "panel");
    if (init_touch(&out->touch) != ESP_OK) {
        ESP_LOGW(TAG, "GT911 not found, running without touch");
        out->touch = NULL;
    }
    out->i2c = s_bus;
    return ESP_OK;
}

esp_err_t board_can_new(uint32_t bitrate, twai_node_handle_t *out)
{
    ESP_RETURN_ON_ERROR(board_exio_set(BOARD_EXIO_USB_CAN_SEL, true), TAG, "can sel");
    const twai_onchip_node_config_t cfg = {
        .io_cfg = {
            .tx = CAN_TX,
            .rx = CAN_RX,
            .quanta_clk_out = GPIO_NUM_NC,
            .bus_off_indicator = GPIO_NUM_NC,
        },
        .bit_timing = {.bitrate = bitrate},
        .fail_retry_cnt = 0, // single shot: an unacked frame must not flood the bus
        .tx_queue_depth = 4,
    };
    return twai_new_node_onchip(&cfg, out);
}
