# Dashboard design

Visual reference taken from an earlier browser prototype. Only layout, style and field names are normative; data sources, units and update logic are defined by the firmware.

## Fields

|Key|Caption|Meaning|Example|
|---|---|---|---|
|`rpm`|RPM|engine speed|1500|
|`gear`|Gear|current gear, `N` = neutral|N|
|`speed`|Speed|vehicle speed|120|
|`wtmp`|W t|coolant (water) temp|105|
|`otmp`|O t|oil temp|100|
|`olvl`|O lvl|oil level|100|
|`fcns`|F cns|fuel consumption|6.7|
|`range`|Range|remaining range|500|
|`flvl`|F lvl|fuel level|20.4|
|`etmp`|E t|exterior (ambient) temp|10|
|`odometer`|Odometer|total distance|255003|
|`errors`|Errors msg|error/warning text|errors|
|`time`|Time|clock `HH:MM`|00:00|

## Data source

`main/obd.c` polls OBD-II mode 01 over CAN using the active runtime ISO 15765-4 profile unless `CONFIG_DASH_DEMO` is set. Settings offers exactly 11-bit/250k, 11-bit/500k, 29-bit/250k and 29-bit/500k. Standard requests use `0x7DF`, replies `0x7E8–0x7EF`; extended requests use `0x18DB33F1`, replies `0x18DAF1xx` with extended-format verification. `CONFIG_DASH_CAN_BITRATE` selects the initial 11-bit default only when there is no saved profile; it is not a fixed runtime profile. Only PIDs the ECU reports as supported are polled; RPM, speed, MAF every ~50 ms cycle, the rest round robin.

Demo mode is disabled by default (`# CONFIG_DASH_DEMO is not set` in `sdkconfig.defaults`).

|Field|Source|
|---|---|
|`rpm`|PID 0C|
|`speed`|PID 0D|
|`wtmp`|PID 05|
|`otmp`|PID 5C|
|`etmp`|PID 46|
|`flvl`|PID 2F % × `CONFIG_DASH_FUEL_TANK_L`|
|`fcns`|PID 10 MAF / speed (petrol, AFR 14.7); unknown at standstill|
|`odometer`|PID A6|
|`errors`|`No ECU` after 5 missed replies; `MIL n DTC` from PID 01|
|`gear`, `olvl`, `range`, `time`|no standard OBD source, shown as `-`|

Unknown values render as `-` without cell fill.

## Layout (top → bottom, full width)

1. **RPM bar** (~100 px tall): horizontal fill bar, 0 … rev limit rounded up to 1000 (`CONFIG_DASH_RPM_LIMIT`, default 6000), 25 px side margins. Tick marks every 1000 rpm (labels `0`, `1`, … = ×1000) above the bar; fill 50 px tall, linear width.
2. **Primary row** — 3 equal cells, huge digits (~100 px font, one line): `RPM` | `Gear` | `Speed`.
3. **Secondary row** — 7 equal cells, ~40 px font: `W t` | `O t` | `O lvl` | `F cns` | `Range` | `F lvl` | `E t`.
4. **Footer row** — 3 equal cells, ~40 px font: `Odometer` | `Errors msg` | `Time`.

Each cell: bordered box (3 px white border, 5 px margin), small caption (~24 px) centered directly above the top border (not overlapping it), value centered in the box.

Font: **B612 Regular** (Airbus cockpit-display face, OFL; Bold looked too heavy) at 100 / 40 / 24 px, 4 bpp, generated into `main/fonts/` with `lv_font_conv` (command in each file header; 100 px has digits, `-.:`, `N`, `R` only; 40/24 px have printable ASCII). Digits are forced tabular in `dashboard.c`.

## Style

- Black background; text, borders and bar white. Monochrome, high contrast, no icons. Exceptions: from (rev limit − 100) rpm the bar fill blinks red/white, and out-of-range cells get a solid background fill (text stays white).
- Colors come only from `main/palette.h` (red, yellow, green, blue), same shades everywhere.
- Cell fills: `W t` and `O t` < 80 blue, > 100 red; `F cns` < 7 green, < 10 yellow, ≥ 10 red; `Range` < 100 red; `F lvl` < 10 red; `E t` > 30 red, < −10 blue.
- Digits are tabular (equal width) so changing values don't jitter.
- Sans-serif. Plain values, no units shown in cells.
- Touching `Errors msg` always opens Settings, including when no ECU warning exists. The field continues to show ECU errors; application/storage errors are additional notices, not replacements for the warning.

## Settings and CAN ownership

Settings uses the same monochrome B612 style and contains the four-profile dropdown, `Apply`, `Logger` and `Back`. Opening it does not stop OBD.

- **Draft** is the dropdown selection. Changing it does not touch CAN or NVS. Back uses the active profile; reopening Settings discards an unapplied selection.
- **Active** is the confirmed running OBD profile. Apply stops the previous owner, starts the selected profile, resets measurements and supported-PID discovery, then saves it. `No ECU` after successful Apply means no response; it does not roll the profile back.
- **Saved** is the last confirmed NVS profile. A save error can leave the new profile active but **not confirmed saved**, with persistence uncertain. Hardware/start failure leaves polling stopped without an implicit rollback; the visible error directs the operator to Apply to resume. A stop/release failure prevents another owner from starting.
- First boot with no stored profile uses the configured 11-bit default (not yet stored). A storage read error is visible, does not erase NVS, and labels a running default as a default rather than a restored saved profile.

`Logger` stops OBD before granting the single TWAI controller to the logger. CLI discovery, queries and START cannot open this screen; START outside it returns `logger_required`. OBD and logger never intentionally run together. Logger capture settings are independent of the dropdown and are not persisted.

## Logger screen and return

The Logger page has only one action, `Exit`. Its scrollable, wrapped text shows effective/requested CAN rate, ID type, normalized filter, listen-only mode, waiting/capturing/ended/error state, assigned/TX/ACK sequence numbers, exact software drops, detected hardware overflow events, unknown hardware lost-frame count, last reason and errors. UART TX and confirmed CLI ACK/heartbeat are separate indicators; TX alone does not prove a reader exists. No raw frames, payloads or on-screen capture configuration/start/stop controls are provided.

Duration, Ctrl-C and link loss end capture but leave Logger open for another explicit CLI command. Exit during capture needs no confirmation: it releases CAN first and reports `user_exit` if the UART remains usable, preserving the host's received data. Exit then restores the **last confirmed saved/default** OBD profile, never a draft or merely active-unsaved profile. If the saved profile was unknown after a read failure, Exit retries loading it; persistent failure leaves OBD stopped with a visible Apply-to-resume error. A CAN-release error keeps Logger open for an Exit retry; a restart failure is visible rather than a false restoration success.

All three screens remain allocated for the application lifetime. LVGL timers update retained objects; controller work that stops/starts CAN runs outside the LVGL lock. Busy operations disable conflicting actions. Reboot opens the normal dashboard, not Logger, and does not resume capture.

See [operator/CLI guide](logger.md), [UART and CSV contract](logger-protocol.md), and [acceptance evidence and hardware checklist](logger-acceptance.md). Source inspection documents these behaviors; real touch/CAN/NVS/power-cycle acceptance is recorded separately, not implied by this design description.
