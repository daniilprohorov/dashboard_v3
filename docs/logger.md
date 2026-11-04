# Passive CAN logger: operator and automation guide

This guide describes the implemented `dashboard-can` 1.0.0 interface. See [wire/CSV reference](logger-protocol.md), [UI behavior](design.md#settings-and-can-ownership), [hardware wiring](hardware.md), and [actual evidence/checklist](logger-acceptance.md). Commands and example results below are **illustrative syntax**, not captured board observations. A serial-peer test proves host behavior, not physical CAN, CH343, touch or persistence.

## Install and discover commands

Python **3.10 or newer** is required; packaging installs `pyserial>=3.5,<4`. From the repository root, on macOS or Linux:

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install ./tools/can_logger
dashboard-can --help
dashboard-can capture --help
dashboard-can --version
```

Equivalent module entry point: `python -m can_logger`. `make logger-install` runs the same package installation with `PYTHON` (default `python3`); activate a virtual environment first or override `PYTHON=.venv/bin/python`. `make logger-help` shows both root and capture help; `make logger-test` runs the existing public-CLI PTY acceptance suite with that interpreter. These targets do not discover/select hardware or start capture. After editing the package, reinstall it before invoking the installed command/tests.

On macOS, the acceptance fixture alone adds its isolated `tests/pty_compat` bootstrap to child `PYTHONPATH` for the exact PTY it owns. It skips pyserial's unsupported Darwin custom-baud ioctl only on that PTY and asserts the requested baud is still 2000000. No adapter is packaged or enabled by installed CLI commands, and no physical serial errors are suppressed. PTY success verifies byte-stream behavior, **not physical 2-Mbaud operation**.

Complete command surface:

```text
dashboard-can [-h|--help] [--version] {ports,status,capabilities,capture} ...
dashboard-can ports [-h|--help]
dashboard-can status [-h|--help] --port PORT
dashboard-can capabilities [-h|--help] --port PORT
dashboard-can capture [-h|--help] --port PORT --bitrate BITRATE --ids {11,29,both}
                      [--allow HEX[-HEX]] ... --output PATH [--duration SECONDS]
```

`--version` is a root option and prints the package version as human text. Help prints human text and exits 0. All other command completions, including argument failures, emit exactly one JSON object on stdout; diagnostics go to stderr. No progress/frame stream is printed to stdout.

## Correct device and exclusive UART ownership

Use the board's USB-C socket labeled **UART**: CH343 to UART0 GPIO43/44. Native USB shares GPIO19/20 with CAN and stays disabled (EXIO5 selects CAN); RS485 is not this transport. The logger uses fixed **2,000,000 baud, 8 data bits, no parity, 1 stop bit (8N1)**, no software/hardware flow control. `--bitrate` changes **CAN only**, never UART.

```sh
dashboard-can ports
# Choose the entry with is_dashboard=true and verify its physical cable:
dashboard-can status --port /dev/cu.wchusbserial1234
dashboard-can capabilities --port /dev/cu.wchusbserial1234
```

Port paths are examples. Linux commonly uses `/dev/ttyUSB0` or `/dev/ttyACM0`; use the actual enumerated path and appropriate serial-device permissions. `ports` enumerates **all** ports but automatically opens/HELLO-probes only USB candidates with non-null VID metadata. Non-USB/system entries remain visible with `probed:false`, `skip_reason:"no_usb_vid"` and `is_dashboard:false`; they are not opened. This avoids the observed macOS non-USB virtual-port close hang. A skipped entry is not proof that no dashboard exists there: an explicitly identified path can still be used with `status`, `capabilities` or `capture --port`.

USB entries have `probed:true`; failures remain per-entry `error` objects, never silently converted to a successful identity. Only exact `device=dashboard_v3`, compatible-protocol peers get `is_dashboard:true`. A successful enumeration with no board (empty, skipped or failed entries) still exits 0 and never auto-selects. VID/PID, description and filenames are candidate metadata, not identity. Probing sends bytes to USB candidates: disconnect unrelated sensitive USB serial equipment before discovery. Final post-repair discovery execution is recorded separately in the [acceptance record](logger-acceptance.md#evidence-available-for-this-implementation).

The IMSB5010 ECU simulator is **not** the dashboard: its CP2102 USB serial interface uses **115200 8N1** and a different command/echo protocol. Do not pass its port to capture; do not mistake its USB echo for this logger's UART. Check the board's physical UART cable and successful HELLO identity, not merely the presence of a USB serial device.

The CLI configures an unopened serial object, deasserts DTR/RTS before open to avoid the default reset/download pulse, and requests exclusive access where the OS supports it. Real CH343 reset behavior still needs hardware acceptance. Only one process may own the port: close serial terminals, IDF monitor, IDE monitors and other CLI processes first. Do not run `ports` during capture (it probes ports rather than attaching to an existing process).

To flash, stop capture and wait for the process/CSV to close, then use `make flash PORT=ACTUAL_BOARD_PORT`. Do not run flash, monitor and CLI concurrently. Firmware console/log interleaving is disabled on the protocol UART; an IDF monitor is not a capture client and does not keep a session alive. Boot ROM output may precede HELLO, but runtime logging must not pollute an established protocol session. Recovery/BOOT instructions are in [hardware.md](hardware.md#flashing--recovery). Ordinary flashing still uses CH343; the protocol baud does not redefine the flash tool's baud.

## Manual gate, profiles and repeated bounded experiments

1. On the board touch **Errors msg → Settings**. OBD continues while Settings is open. The dropdown is a draft until **Apply**. Four ISO 15765-4 profiles are available: 11-bit/250k, 11-bit/500k, 29-bit/250k, 29-bit/500k. Active, saved and draft values are distinct; see [failure/restore semantics](design.md#settings-and-can-ownership).
2. Touch **Logger**. This stops OBD and hands the single TWAI controller to the logger. Logger should say idle/waiting for CLI. The CLI cannot do this touch remotely: capture on dashboard/Settings returns `logger_required` and does not switch modes or stop OBD. Discovery/status/capabilities are also read-only with respect to mode.
3. Configure the traffic source/bench and use a **new** output path for each explicit capture:

```sh
mkdir -p captures
dashboard-can capture --port /dev/cu.wchusbserial1234 \
  --bitrate 500000 --ids 11 --allow 001 --allow 100-10F \
  --output captures/first.csv --duration 10
# Logger remains open. Change the source bitrate only when safe, then:
dashboard-can capture --port /dev/cu.wchusbserial1234 \
  --bitrate 250000 --ids both --allow 001-00F \
  --output captures/second.csv --duration 5.5
# No --duration: run until Ctrl-C, screen Exit, failure or link timeout.
dashboard-can capture --port /dev/cu.wchusbserial1234 \
  --bitrate 500000 --ids 29 --output captures/manual.csv
```

4. Inspect JSON reason/source/counters and the CSV `end` row; do not classify an empty frame set as proof of a quiet or correctly wired bus. Capture does not analyze payloads or diagnose a vehicle.
5. A first Ctrl-C requests STOP with `ctrl_c` and waits up to 3 seconds for device END. A second Ctrl-C aborts the wait and yields local interrupted provenance. A duration is owned by the device, not just a host sleep. Both normal stops release the serial port and leave Logger idle. Every repeat needs a new explicit command; no retry/resume/replay exists.
6. Touch **Exit** to stop capture without an extra confirmation and return to OBD using the **last confirmed saved/default profile**, not an unapplied draft or active-unsaved choice. A connected CLI records `user_exit` and retains received rows, but exits 7 (not normal duration completion). Release/storage/restart failures remain visible and do not claim that polling was restored. Reboot opens the dashboard, never resumes Logger.

A capture exclusively creates its output (never overwrites); the parent directory must already exist. The CSV header is created after HELLO but before START. A rejected START can leave a header-only file with no session/configuration; that is not a successful capture.

## CAN configuration and filtering

- Classic CAN only, 11/29-bit ID and up to 8 payload bytes; no CAN FD. Firmware exports Classic DLC codes 9–15 as the corresponding eight-byte length, so raw DLC codes above 8 are not preserved.
- `--port`, `--bitrate`, `--ids`, `--output` are required on every capture; there is no inherited dashboard/logger configuration. `--bitrate` is a decimal integer in **1000–1000000 bit/s**, accepted only if exactly achievable. There is no rounding. Requested and effective rates must match. 250000/500000 are illustrative conventional configurations; capabilities advertises bounds, not a list of all reachable rates.
- The board uses an 80 MHz APB clock and exact timing satisfying `bitrate = 80000000 / (BRP × (1 + seg1 + seg2))`, even BRP 2–16384, seg1 1–16 and seg2 1–8. Divisibility and timing constraints both matter; e.g. 333333 is not exactly reachable and is rejected with `invalid_bitrate`. A syntactically/in-range rate may therefore pass CLI validation but fail at the device before applying a new session.
- Repeat `--allow` for exact IDs or inclusive `LOW-HIGH` ranges: hex without `0x`, 1–8 digits, case-insensitive input, no commas within a CLI argument. The wire joins entries with commas. Values must be ≤`7FF` for `11`, otherwise ≤`1FFFFFFF`; reversed ranges are invalid. No `--allow` means `all` selected IDs.
- Ranges are numeric OR, then restricted by `--ids`. They sort and merge overlap **and adjacency**, with at most **16 normalized disjoint ranges**. Canonical output is uppercase, padded to 3 hex digits for `11`, 8 for `29`/`both`; a singleton has no dash. Example: `--ids 11 --allow 010-01F --allow 001 --allow 020` becomes `001,010-020`; `--ids both --allow 001` becomes `00000001` and admits both standard `001` and extended `00000001` with independent format fields.
- Filtering happens before sequence assignment and UART output. Excluded ID/type frames do not increase software-drop counts. Reconfigure only between captures; an active START is `busy`.
- `--duration` is positive finite seconds, rounded **up** to whole milliseconds (minimum 1 ms); omission means no duration deadline. The host accepts values fitting uint64 milliseconds, but firmware additionally requires a duration that fits its signed boot-monotonic microsecond deadline. An excessively large host-valid duration is rejected by the device as `invalid_argument`; do not treat it as a usable timer.

## Passive bench and interpreting loss

Capture is listen-only: no board OBD queries, test transmissions or CAN ACK bits. A transmitting bench node may need a **separate active ACK-capable CAN node** at the same bitrate. Do not enable board transmissions just to make a passive test pass. Provide correct CAN-H/L, common signal ground and termination at the two physical ends; see [hardware.md](hardware.md#bench-tool-imsapp-imsb5010-obd-ii-ecu-simulator).

The IMSB5010 responds to requests and does **not** autonomously broadcast. When Logger stops OBD queries, silence from that simulator is expected; a silent capture alone neither proves nor disproves logger function. For passive acceptance use a known independently transmitting source plus ACK node and an analyzer that can verify the board contributes no ACK/transmissions.

UART 2 Mbaud 8N1 has a **theoretical 200,000 bytes/s** ceiling before ASCII framing, metadata, status/response traffic, driver/OS and CAN processing overhead. This is neither measured throughput nor a lossless-load guarantee. There is no flow control or retransmission. An overloaded 128-frame software queue drops **new** accepted frames and preserves the order of queued frames. `sw_drops` is an exact count of these software queue rejections. `hw_overruns` is a count of detected hardware overflow events, **not** lost frames; `hw_lost=unknown` always states that hardware lost-frame cardinality is unavailable. Sequence gaps are allowed and expose missing accepted frames, but zero gaps/drops do not prove no upstream loss.

UART `tx`/`tx_seq` means output enabled/fully emitted by the device, not computer receipt. Valid ACK/PING for the current session marks `client`; ACK names only the last frame actually parsed by the host (0 on a quiet session). The CLI sends ACK every 0.75 s including quiet sessions. Three seconds without valid current-session service messages stops capture with `link_loss`; queries or malformed/stale messages do not keep it alive. ACK confirms **CLI receipt**, not `fsync`, durable disk storage or absence of pre-UART losses. CSV flush is periodic (0.25 s) and on start/end/close, not an `fsync` durability promise.

## JSON results and stable exit codes

Normal `status`/`capabilities` JSON includes `ok`, `command`, `port`, `device`, `firmware_version`, integer `protocol_version`, `cli_version`, and `status` or `capabilities`. Device field values inside those two maps remain **strings** (including numbers/flags). `ports` returns `ok`, `command`, `ports`; each entry includes `port`, `description`, `hwid`, nullable numeric `vid`/`pid`, nullable `serial_number`, boolean `is_dashboard` and boolean `probed`. Skipped entries add `skip_reason:"no_usb_vid"`; probed entries add identified `device`, `firmware_version`, integer `protocol_version`, `mode`, or `error:{code,message}`. A close failure can add `error` even after successful identification; inspect both fields.

A started capture result contains `ok`, `command`, `session_id`, firmware/protocol/CLI versions, `config`, `counters`, `source`, `reason`, `output`, and `error` on failure. `config` has integer `requested_bitrate`, `effective_bitrate`, string `id_type`, `filter`, boolean `listen_only`, integer `uart_baud`, `uart_data_bits`, `uart_stop_bits`, string `uart_parity`. `counters` has integer `seq`, `tx_seq`, `ack_seq`, `sw_drops`, `hw_overruns`, and string `hw_lost_frames`. These counters come **only from device END**; without END every final-counter value is JSON `null`, even when earlier status rows exist. `source` is `device` only with END, otherwise `cli`.

Before successful START, a capture failure has `session_id:null`, `config:null`, null counters and local `source`; it does not claim confirmed settings. Generic failures contain `ok:false`, `command` (possibly null for an unknown/missing command), and `error:{code,message}`. Human `message` text is diagnostic, not the stable automation key. A file-write/close error can make `ok:false` even if END was received; provenance/reason can still be device-confirmed, so inspect `error` and exit code as well as reason.

Illustrative argument-failure output (not a board response):

```json
{"ok":false,"command":"status","error":{"code":"argument_error","message":"the following arguments are required: --port"}}
```

Illustrative device-confirmed duration result (exit 0), matching the reference's invented two-frame example:

```json
{"ok":true,"command":"capture","session_id":"0123456700000001","firmware_version":"example-build","protocol_version":1,"cli_version":"1.0.0","config":{"requested_bitrate":500000,"effective_bitrate":500000,"id_type":"both","filter":"00000001","listen_only":true,"uart_baud":2000000,"uart_data_bits":8,"uart_parity":"N","uart_stop_bits":1},"counters":{"seq":2,"tx_seq":2,"ack_seq":2,"sw_drops":0,"hw_overruns":0,"hw_lost_frames":"unknown"},"source":"device","reason":"duration","output":"captures/example.csv"}
```

Illustrative local link-loss result after START (exit 6), with every final counter unavailable rather than copied from a prior status:

```json
{"ok":false,"command":"capture","session_id":"0123456700000001","firmware_version":"example-build","protocol_version":1,"cli_version":"1.0.0","config":{"requested_bitrate":500000,"effective_bitrate":500000,"id_type":"both","filter":"00000001","listen_only":true,"uart_baud":2000000,"uart_data_bits":8,"uart_parity":"N","uart_stop_bits":1},"counters":{"seq":null,"tx_seq":null,"ack_seq":null,"sw_drops":null,"hw_overruns":null,"hw_lost_frames":null},"source":"cli","reason":"link_loss","output":"captures/example.csv","error":{"code":"link_loss","message":"No valid serial messages for 3 seconds"}}
```

| Exit | Meaning / machine codes |
|---|---|
| 0 | Successful query/discovery; or device-confirmed capture `duration`, `ctrl_c`, `host_stop`. Discovery 0 does not mean a board was found. |
| 2 | CLI `argument_error` (missing/invalid options, bounds, filter, duration). |
| 3 | Before/without an active session: `port_error`, `transport_error`, `response_timeout` (3 s request timeout). |
| 4 | `protocol_error` or `unsupported_version`: incompatible/malformed/contradictory established stream. |
| 5 | Device rejection (`logger_required`, `invalid_bitrate`, `invalid_argument`, `invalid_command`, `busy`, `invalid_session`, `hardware_error`), or device END hardware failure. Unsupported-version rejection uses 4 instead. |
| 6 | Active-session `link_loss`, `stop_timeout`, `end_timeout`, `interrupted` or outstanding `response_timeout`; transport failure during capture is mapped to local link loss. |
| 7 | Device END `user_exit` (the person left Logger). |
| 8 | `output_error`: exclusive creation, write, flush or close failure. |

Local end reasons retain the specific failure code (`response_timeout`, `protocol_error`, `stop_timeout`, `end_timeout`, `interrupted`, `output_error`, etc.). An outstanding ACK response can time out **while valid frames continue**: after START this is exit 6 with both `error.code` and local CSV/JSON `reason=response_timeout`, not `link_loss`. Physical transport errors and 3-second receive silence instead produce local `link_loss`; a device-confirmed heartbeat timeout has `source=device`, `reason=link_loss`. All local ends leave final counters unavailable. There is no public `stop` subcommand; `host_stop` is supported by UART STOP for another client. Ctrl-C is the CLI's graceful stop action.

## Timestamp and incomplete-file limits

`device_us`/frame `us` are boot-monotonic microseconds from `esp_timer_get_time()`, not Unix time or session-relative time; reset restarts the clock. Frame time is software sampling **immediately after TWAI dequeue in the receive ISR**, not CAN wire start-of-frame or host arrival. Resolution is 1 µs, but ISR/FIFO latency, absolute accuracy and synchronization error have not been measured. START supplies the device-time binding; each CSV row also has host UTC ISO 8601 with six fractional digits and `Z`. Host UTC is when that row is produced; it can jump with wall-clock corrections and does not establish precise device/UTC synchronization. Compare frame intervals in the device timebase and retain session identity.

Treat ID/payload CSV fields as text to preserve leading zeros (spreadsheet auto-conversion is unsafe). Retain all events, not only frames, to preserve configuration and loss/end provenance. `end,source=device` confirms final counters; `end,source=cli` has blank final counters/device time/state. **No end row means incomplete**, and abrupt process/OS/power failure may leave a final truncated CSV record. Preserve the readable prefix and mark the capture incomplete rather than inventing terminal counts. A header-only rejection is not a bound session. No resume/replay fills the missing interval; use a new file and new explicit capture.
