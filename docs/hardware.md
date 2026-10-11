# Waveshare ESP32-S3-Touch-LCD-7B — hardware reference

Condensed from `docs/vendor/waveshare-wiki-ESP32-S3-Touch-LCD-7B.md` and vendor code
(`third_party/waveshare-ESP32-S3-Touch-LCD-7B/examples/ESP-IDF/17_lvgl_v9_demo`).
Schematic: `docs/vendor/pdf/ESP32-S3-Touch-LCD-7B-Schematic.pdf` (image-only, no text layer).

## Core

| Item | Value |
|---|---|
| Module | ESP32-S3-WROOM-1-N16R8: 2×LX7 @240 MHz, 512 KB SRAM |
| Flash | 16 MB, QIO, run at 120 MHz |
| PSRAM | 8 MB octal (OPI), run at 120 MHz |
| Display | 7" IPS 1024×600, RGB565 parallel (16 data lines), driver ST7262 |
| Touch | GT911, 5 points, I2C 0x5D, INT on GPIO4 |
| Power | USB-C 5 V (~350 mA); 3.7 V Li-ion on PH2.0 (CS8501 charger, 580 mA, ≤2000 mAh recommended) |
| Temp | 0…65 °C (panel limit — relevant for a car cabin) |

## RGB LCD pins

| Signal | GPIO | Signal | GPIO | Signal | GPIO |
|---|---|---|---|---|---|
| PCLK | 7 | B3 | 14 | G2 | 39 |
| DE | 5 | B4 | 38 | G3 | 0 |
| HSYNC | 46 | B5 | 18 | G4 | 45 |
| VSYNC | 3 | B6 | 17 | G5 | 48 |
| R3 | 1 | B7 | 10 | G6 | 47 |
| R4 | 2 | | | G7 | 21 |
| R5 | 42 | R6 | 41 | R7 | 40 |

`data_gpio_nums` order (D0..D15) = B3..B7, G2..G7, R3..R7.
Timings: PCLK 24 MHz (vendor notes 30 MHz max, ~17 FPS LVGL benchmark single core), pclk falling edge,
HSYNC pw/bp/fp = 162/152/48, VSYNC pw/bp/fp = 45/13/3. Frame buffers in PSRAM + 20-line bounce buffer.

GPIO0 (strap/BOOT) and GPIO45/46 (straps) are LCD data lines — do not repurpose.

## I2C bus (GPIO8 SDA, GPIO9 SCL, 400 kHz)

| Addr | Device |
|---|---|
| 0x24 | IO expander (Waveshare custom MCU firmware) |
| 0x5D | GT911 touch |

User I2C devices share this bus via the PH2.0 I2C header; avoid these addresses.

### IO expander protocol (0x24)

Write `[reg, value]`; read: write `[reg]`, then read.

| Reg | Meaning |
|---|---|
| 0x02 | pin mode mask, bit=1 → output (vendor writes 0xFF) |
| 0x03 | output levels (8 bits, write whole byte — keep a shadow copy) |
| 0x04 | input levels |
| 0x05 | backlight PWM, 0..255, **higher = darker**; vendor caps at 97 % |
| 0x06 | ADC, u16 little-endian, 10-bit, 3.3 V ref, battery behind 3:1 divider → `mV = raw·9900/1023` |

| EXIO | Function |
|---|---|
| 1 | TP_RST (GT911 reset) |
| 2 | DISP / backlight enable |
| 3 | LCD reset |
| 4 | SD_CS (active low) |
| 5 | USB_SEL: 0 → GPIO19/20 = native USB, 1 → GPIO19/20 = CAN |
| 6 | LCD_VDD_EN (VCOM) |

GT911 reset sequence (selects 0x5D): GPIO4 out low, EXIO1 low, 100 ms, GPIO4 low, 100 ms, EXIO1 high, 200 ms.

## Other peripherals

| Peripheral | Pins | Notes |
|---|---|---|
| CAN (TWAI) | TX GPIO20, RX GPIO19 | TJA1051 transceiver; shares pins with native USB, needs EXIO5=1; 120 Ω terminator via DIP switch |
| RS485 | TX GPIO15, RX GPIO16 | auto direction; 120 Ω terminator via DIP switch |
| UART / flashing | GPIO43/44 via CH343 USB-UART | "UART" USB-C port; auto-download circuit; logs here |
| TF card (SPI) | MOSI 11, SCK 12, MISO 13, CS = EXIO4 | |
| Native USB | D− 19, D+ 20 | only when EXIO5=0 (then CAN unavailable) |
| Sensor/ADC header | see schematic | PH2.0 |

**Car-dashboard implication:** CAN and native USB are mutually exclusive. Our firmware keeps EXIO5=1 (CAN);
flash/monitor through the CH343 UART port (`/dev/cu.wchusbserial*` on macOS).

## Flashing / recovery

- Auto-download works over the UART USB-C port; after flashing press RESET if the app does not start.
- If the port is not detected: hold BOOT, plug USB, release BOOT.
- macOS: CH34x driver may be needed (link in wiki FAQ).
- RGB drift after Wi-Fi/flash activity: see ESP FAQ "drift of the display when ESP32-S3 drives an RGB LCD";
  mitigated by bounce buffers + `CONFIG_SPIRAM_XIP_FROM_PSRAM`.
