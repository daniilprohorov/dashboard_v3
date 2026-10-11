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

## Layout (top → bottom, full width)

1. **RPM bar** (~100 px tall): horizontal fill bar, 0 … rev limit rounded up to 1000 (`CONFIG_DASH_RPM_LIMIT`, default 6000), 25 px side margins. Tick marks every 1000 rpm (labels `0`, `1`, … = ×1000) above the bar; fill 50 px tall, linear width.
2. **Primary row** — 3 equal cells, huge digits (~100 px font, one line): `RPM` | `Gear` | `Speed`.
3. **Secondary row** — 7 equal cells, ~40 px font: `W t` | `O t` | `O lvl` | `F cns` | `Range` | `F lvl` | `E t`.
4. **Footer row** — 3 equal cells, ~40 px font: `Odometer` | `Errors msg` | `Time`.

Each cell: bordered box (3 px white border, 5 px margin), small caption (~24 px) centered in the top border, value centered below.

## Style

- Black background; text, borders and bar white. Monochrome, high contrast, no icons. Only exception: from (rev limit − 100) rpm the bar fill blinks red/white.
- Digits are tabular (equal width) so changing values don't jitter.
- Sans-serif. Plain values, no units shown in cells.
- Display-only, no touch interaction.
