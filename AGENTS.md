# dashboard_v3 — car dashboard firmware

Target: Waveshare **ESP32-S3-Touch-LCD-7B** (ESP32-S3 N16R8, 1024×600 RGB, GT911 touch, CAN, RS485).
Stack: **ESP-IDF v5.5.5** (`idf.py`, CMake, component manager), **LVGL 9** via `espressif/esp_lvgl_adapter`. C11.
PlatformIO is intentionally not used: its espressif32 platform lags ESP-IDF, and vendor LVGL 9 code needs IDF ≥ 5.5.

## Commands

All via `scripts/idf` — wraps `idf.py` with the pinned IDF env; works from any fresh shell, no `export.sh` needed.

| Task | Command |
|---|---|
| Install toolchain (once) | `scripts/setup.sh` |
| Build | `scripts/idf build` |
| Flash + log | `scripts/idf -p /dev/cu.wchusbserial* flash monitor` (monitor is interactive; agents: `flash` only, read logs with a timeout) |
| Size report | `scripts/idf size` / `size-components` |
| Config UI | `scripts/idf menuconfig` (interactive; prefer editing `sdkconfig.defaults`) |
| Add managed component | `scripts/idf add-dependency "espressif/<name>^<ver>"` — or edit the component's `idf_component.yml` |
| Clean config regen | `rm sdkconfig && scripts/idf reconfigure` |
| Fetch vendor examples | `scripts/fetch-vendor.sh` |

Board serial port: the "UART" USB-C (CH343). The native USB port is disabled because GPIO19/20 are muxed to CAN.

## Layout

```
main/                 app entry (main.c), app-level deps in idf_component.yml
components/board/     board support: I2C bus, IO expander, RGB panel, GT911, backlight, battery
sdkconfig.defaults    ALL Kconfig choices live here; sdkconfig is generated and git-ignored
partitions.csv        16 MB: 7 MB factory app + LittleFS storage
dependencies.lock     component-manager lockfile, commit it
managed_components/   downloaded deps (git-ignored, read-only)
docs/hardware.md      pinout, IO-expander protocol, peripheral constraints — read before touching hardware
docs/vendor/          wiki dump (.md), datasheets (pdf/ + text/ for grep)
third_party/          vendor example repo, git-ignored, reference only
scripts/              env.sh (IDF version pin), idf, setup.sh, fetch-vendor.sh
```

## Where to look things up

- Board: `docs/hardware.md` → `docs/vendor/waveshare-wiki-ESP32-S3-Touch-LCD-7B.md`.
- Chip registers/peripherals: grep `docs/vendor/text/Esp32-s3_technical_reference_manual_en.txt`, `Esp32-s3_datasheet_en.txt`.
- Panel / touch / CAN PHY: `docs/vendor/text/{ST7262,GT911_EN_Datasheet,TJA1051}.txt`.
- ESP-IDF API: headers under `$HOME/esp/esp-idf-v5.5.5/components/*/include`, docs source in `$HOME/esp/esp-idf-v5.5.5/docs/en`, examples in `$HOME/esp/esp-idf-v5.5.5/examples` (e.g. `peripherals/lcd/rgb_panel`, `peripherals/twai`).
- LVGL 9 / adapter / GT911: source in `managed_components/` after first build. Online: https://docs.lvgl.io/9.4/.
- Vendor working code per peripheral: `third_party/waveshare-ESP32-S3-Touch-LCD-7B/examples/ESP-IDF/NN_*`. Quality is low (global state, `ESP_ERROR_CHECK` everywhere, typos like `wavesahre_`) — copy values, not style.
- UI design (screen layout, style, field names): `docs/design.md` — read before touching UI code.

## Rules

- Hardware access goes through `components/board`; app code never touches pins or expander registers directly.
- All config in `sdkconfig.defaults`. Never commit `sdkconfig`. Changing flash/PSRAM speed: keep both at 120 MHz or both at 80 MHz.
- LVGL calls only from the LVGL task or inside `esp_lv_adapter_lock()/unlock()`.
- Large buffers → PSRAM (`heap_caps_malloc(..., MALLOC_CAP_SPIRAM)`); DMA/ISR data → internal RAM.
- Library code returns `esp_err_t` (`ESP_RETURN_ON_ERROR`); `ESP_ERROR_CHECK` only in `app_main` bring-up.
- I2C bus is shared (expander 0x24, GT911 0x5D); reuse `board_t.i2c`, don't create a second bus on GPIO8/9.
- CAN and native USB share GPIO19/20 (EXIO5). Keep CAN.
- Panel rated 0–65 °C: car-cabin heat is a real constraint for backlight/CPU load decisions.
- Done = `scripts/idf build` passes with no new warnings. On-device behavior needs a flash + serial log check; say explicitly if not verified on hardware.
