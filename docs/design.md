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

`main/obd.c` polls OBD-II mode 01 over CAN (11-bit, `CONFIG_DASH_CAN_BITRATE`, request `0x7DF`, replies `0x7E8–0x7EF`) unless `CONFIG_DASH_DEMO` is set. Only PIDs the ECU reports as supported are polled; RPM, speed, MAF every ~50 ms cycle, the rest round robin.

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

## Style

- Black background; text, borders and bar white. Monochrome, high contrast, no icons. Exceptions: from (rev limit − 100) rpm the bar fill blinks red/white, and out-of-range cells get a solid background fill (text stays white).
- Colors come only from `main/palette.h` (red, yellow, green, blue), same shades everywhere.
- Cell fills: `W t` and `O t` < 80 blue, > 100 red; `F cns` < 7 green, < 10 yellow, ≥ 10 red; `Range` < 100 red; `F lvl` < 10 red; `E t` > 30 red, < −10 blue.
- Digits are tabular (equal width) so changing values don't jitter.
- Sans-serif. Plain values, no units shown in cells.
- Display-only, no touch interaction.
