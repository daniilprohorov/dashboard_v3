# Logger acceptance record and bench procedure

This file separates **observed evidence** from **instructions not yet executed**. Interface definitions live in [operator guide](logger.md), [UART/CSV reference](logger-protocol.md), and [UI design](design.md). Passing a serial-peer scenario does not establish firmware, CAN timing/filtering/passivity, CH343 baud/reset behavior, touch, power-cycle storage or physical loss accounting.

## Evidence available for this implementation

Integration-owner observations on macOS arm64, 2026-11-05:

| Area | Observed result / limit |
|---|---|
| Host runtime | `/opt/homebrew/bin/python3`, Python **3.14.6**. This is one tested runtime, not evidence that every supported Python version was exercised. |
| Installation/help | Installed `dashboard-can --help` succeeded. Full final CLI suite results are recorded below when supplied by the integration owner; this statement alone is not capture evidence. |
| Integrated firmware build | Initial `make reconfigure && make build` succeeded; app binary size **0xb23e0**. That pass emitted a new deprecated `lv_obj_add_flag` warning at `main/dashboard.c:118`; therefore it was **not** a clean no-new-warning acceptance pass. Reconfigure also emitted a CMake “Missing kconfig” rerun warning. Final post-fix results must supersede this initial observation before declaring the build gate clean. |
| Discovery before correction | `dashboard-can ports` exceeded **30 s** on non-USB macOS virtual ports (`Bluetooth-Incoming-Port`, `VXS`, `debug-console`, `wlan-debug`, all without VID metadata). A minimized run's stack showed blocking `serial.close` in discovery. This is a recorded defect, not a passing discovery run. The final USB-candidate-only auto-probe behavior and its observed result must be recorded with the final checks. |
| Initial macOS PTY limitation | A raw macOS PTY status smoke exited **3** with structured `port_error` / `ENOTTY` when pyserial attempted its 2-Mbaud `IOSSIOSPEED` ioctl. The initial suite could not handshake and was stopped after diagnosing this systemic fixture limitation. It is not passing capture evidence. A narrowly scoped test-only PTY baud adapter is required; production UART settings must remain fixed 2M8N1. |
| Linker integration | Integration-owner inspection found `__wrap_twai_hal_read_rx_fifo` at **0x4037abd4** in IRAM in `build/dashboard.map`, referenced by the ESP TWAI on-chip driver object; `board_can_get_overruns` was referenced by logger. This proves hook linkage, **not** that physical overruns were observed or counted correctly. |
| Physical board / serial | Initial `/dev/cu.*`, `/dev/ttyUSB*`, `/dev/ttyACM*` inspection found no connected board device. **No CH343, real UART 2 Mbaud, CAN, touch or power-cycle evidence** is available from that inspection. No flashing or board acceptance is claimed. |
| Linux | Docker executable existed, but daemon connection failed because its socket was absent. **No Linux execution evidence**. Linux support is an implementation target, not an observed Linux pass. |
| Hardware throughput / loss | **Unverified**. No measured sustained rate, lossless threshold, ACK/passivity or physical overrun result. 200000 bytes/s is a theoretical UART bound only. |

The initial build/discovery results are preserved so a later passing check cannot be confused with an earlier incomplete gate. The integration owner performs final checks once changes are integrated; the documentation worker does not run builds/tests/formatters independently.

## Reproducible host acceptance boundary

With the CLI installed in the chosen Python environment, the existing test command is:

```sh
python -m unittest discover -s tools/can_logger/tests -v
# Equivalent wrapper:
make logger-test PYTHON=python
```

The suite launches the public `python -m can_logger` subprocess against POSIX PTYs, observes outgoing UART requests, JSON, exit status and event CSV, and uses new temporary files/ports per scenario. It can run on macOS/Linux; execution on one does not prove the other. The serial peer emits controlled bytes rather than running firmware or generating physical CAN. macOS PTYs do not support the physical 2-Mbaud `IOSSIOSPEED` ioctl; a test-only adapter may skip that ioctl for actual Darwin PTY descriptors, never for CH343 or the production transport. Such byte-stream tests are **not physical 2-Mbaud verification**. Retain command, OS/Python/CLI revision, pass/fail/skip counts, and representative **actually observed** stdout/CSV/request-response output separately from the illustrative syntax in the reference.

Coverage to record from the final executed suite: required options and structured argument errors; exact/range/both-ID filter boundaries and normalization; zero-length/RTR/padded frames; incompatible version and malformed active stream; correlated responses amid async frames; no false successful configuration; device duration and Ctrl-C; repeat/port release; user_exit versus local link loss; quiet ACK/heartbeat; sequence gaps and distinct software/hardware metadata; unavailable final counters; incomplete output; output creation/write/close failures. These are host-consumer observations, not proof that the firmware performs physical filtering/drop-new or timing correctly.

## Required physical bench

Use Waveshare ESP32-S3-Touch-LCD-7B, pinned ESP-IDF v5.5.5 firmware, its **UART** CH343 USB-C connection, a compatible ISO 15765-4 ECU/simulator for OBD, and a separate controllable CAN source/analyzer for passive testing. Wire CAN-H/L and common signal ground, use 120 Ω termination at each of the two physical bus ends, set all participants to the test bitrate, and follow [hardware.md](hardware.md). Record actual supplies, termination positions, source/ACK-node/analyzer models and revisions, OS/driver, cable, port and firmware/CLI revisions.

**Listen-only ACK requirement:** the logger must not ACK or transmit. A source needing successful ACK therefore needs another active, correctly configured ACK-capable CAN node. Use an analyzer/scope or bench instrumentation that distinguishes the board's contribution; a populated CSV alone cannot prove passivity. Preserve analyzer traces and distinguish intentional source retransmissions from logger duplicates.

**Silent-simulator caveat:** IMSB5010 answers OBD requests but does not autonomously broadcast. Its CP2102 115200-baud USB port is not the board's CH343. After Logger stops OBD queries, silence from the IMSB5010 is expected. Use an independently transmitting source (and separate ACK node where needed) for passive capture; do not diagnose the logger solely from an empty simulator capture.

All checklist items below are **unverified on hardware** until an operator records real observations and artifacts. The procedures are not passing results.

## Hardware acceptance checklist

| Scenario | Procedure and required observable evidence |
|---|---|
| Safe installation and identity | Install CLI; run help/version/ports; physically identify CH343 UART cable. Keep simulator CP2102 distinguishable. Status/capabilities match dashboard identity/protocol, 2M8N1, bounds and exact-only. Close all other port owners. Record actual JSON and exit codes. |
| Navigation/style and errors | With and without ECU warnings, touch `Errors msg`; it opens Settings without losing warning meaning. Verify monochrome fonts, draft dropdown, Apply/Logger/Back. OBD readings continue while Settings is displayed. Repeated navigation causes no stale/deleted-widget updates. |
| Draft and Back | Change dropdown without Apply; analyzer shows unchanged CAN profile, displayed active/saved values remain correct. Back and reopen discard the draft; no NVS write is claimed. |
| Every OBD profile | Configure compatible ECU/simulator for each of 11/250, 11/500, 29/250, 29/500. Apply; record actual CAN bitrate and ID-format/addressing: standard `7DF`/`7E8–7EF`, extended `18DB33F1`/`18DAF1xx`. Verify real supported-PID discovery and measurements, not just labels. Reset simulator after DIP changes as required. |
| Fresh data / No ECU | Before switching profile record measurements/support. Apply another profile; old measurements/support clear and detection restarts. With no ECU response show `No ECU` without rollback; applied and saved selection stays the chosen profile. |
| Apply and storage failures | Under a safe controlled fault, verify stop/start failure is visible, not labeled applied/saved, no second CAN owner starts, polling state is honest, no implicit rollback is claimed. Inject a storage failure safely: new active-unsaved profile is distinguished from last confirmed saved and uncertainty shown. Retry Apply after restoring the prerequisite. Do not erase valuable NVS just to conceal a failure. |
| Boot/load/power cycle | Save each profile and power-cycle; only saved dashboard profile is restored, no Logger/capture auto-start. With absent profile show default/not yet stored. With controlled read error show load error/running default rather than successful restore; NVS is not automatically erased. Record physical power-cycle, not merely source review. |
| Manual Logger gate | While dashboard and Settings are active, ports/status/capabilities leave mode/OBD unchanged. START returns `logger_required`, no capture/config event. Touch Logger; analyzer verifies OBD owner/callbacks stop before passive capture can begin. |
| Explicit/exact configuration | Omit each required CLI option: exit 2, no session start. In Logger try invalid/unreachable bitrate (e.g. 333333): device rejection, no rounding or partial new configuration. Start a valid exact rate: requested/effective, type/filter/listen-only/UART metadata match JSON, CSV and screen. Active START is busy; config changes only between sessions. |
| Passive source/ACK | With separate ACK node, send controlled frames; verify board sends no OBD, test transmissions, CAN ACK or other traffic during capture. Repeat without ACK node as appropriate to show why source behavior may change; do not use missing frames as sole passivity proof. |
| Inclusive filters | Emit outside IDs, exact IDs, both inclusive range endpoints, interior IDs, overlapping/adjacent/disjoint ranges; compare expected OR/type result with CSV. Test no allowlist, 11, 29, both and numerically equal standard/extended IDs. Confirm normalized screen/START filter, rejected excess >16 normalized ranges, and excluded frames do not raise software drops. |
| Frame fidelity/time | Send leading-zero IDs/data, DLC 0/8, RTR with nonzero DLC, standard/extended same numeric ID; preserve independent bits/RTR/DLC and padded hex. Verify retained-frame order and boot-monotonic ISR-dequeue timestamps against instrumented timing, measuring latency separately rather than claiming SOF precision. Record UTC/device binding limits. |
| Bounded duration and Ctrl-C | Capture to new files with duration then without duration/Ctrl-C. Observe final E reason, exit 0, parsable end row and released port. Logger remains idle/open. Start a second explicit capture with another valid config/file; new SID, no automatic resume or inherited config. |
| Exit idle/active | Exit idle restores last confirmed saved/default OBD profile. Exit active stops physical capture before OBD, needs no confirmation, yields `user_exit`/exit 7 if link usable and retains CSV prefix. Repeat after active-unsaved Apply and with unapplied draft: neither is restored in place of saved. |
| Restore failures | Simulate release failure safely: no OBD overlap, Logger remains available to retry Exit. If saved profile unknown/read still failing, CAN is released but OBD remains stopped with Apply-to-resume error, not guessed restoration. Failed saved-profile restart is visible. A UART notification failure must not prevent safe physical release. |
| Quiet heartbeat / flags | With quiet bus, inspect ACK(0)/PING service, active capture survives >3 s. Screen distinguishes TX enabled from actual current-session client receipt and ACK sequence. No ACK implies fsync or loss-free acquisition; explain that to operator. |
| Disconnect / stale identity | Disconnect/stop service without graceful STOP; device ends after 3 s without valid ACK/PING, reason link_loss, stays Logger. Queries/malformed/stale SID do not extend deadline. Reconnect cannot resume/replay; new explicit capture has new SID. Reset during capture also breaks identity and does not silently continue. Preserve local-end/null-counter evidence when E cannot arrive. |
| Sustained load and overload | Increase known source rate under documented CAN bitrate, IDs/payload/RTR mix, bus utilization, UART/cable/host conditions. Record offered/received rate and duration plus independent source/analyzer counts. Demonstrate bounded queue drop-new behavior and retained order under controlled overload, continued capture, gaps/seq/TX/ACK, **exact sw_drops separately from hardware event count**. Mark hardware lost frames unknown, not a derived exact count. Zero detected events is not proof of losslessness. |
| Active-stream framing | Record actual CH343 2M8N1 with no runtime console interleaving. Exercise request correlation with async traffic, supported HELLO/CAPS, unsupported version/command, framing limits. Boot noise is ignored only before HELLO; active corruption fails visibly rather than being silently skipped. |
| CSV / terminal provenance | Parse with ordinary CSV reader. Verify constant 30-column header, versions/config/session/UTC, IDs/payload text, all event types. Compare device END final counts with CSV/JSON. Break link: local terminal counters/time/state empty, JSON counters null. Kill process: no END means incomplete; handle truncated final record without inventing counters. Preserve received prefix. |
| File/port/flash lifecycle | Existing output refuses overwrite; bad/unwritable output fails, not success. Graceful completion closes CSV/port; next capture/status acquires same port. Stop/close CLI and monitor, flash via CH343, restart, verify normal dashboard and explicit discovery still work. Do not test by concurrent flash/capture. |
| macOS and Linux | Execute the public CLI scenarios on both actual OS environments with real serial permissions/driver/port. Record supported commands/options, JSON/exit codes, capture/end behavior and CH343 results separately per OS. A macOS PTY run or an unavailable Docker daemon is not Linux evidence. |

## Recording a real run

For every executed scenario retain: date, OS/Python, firmware commit/build version, CLI version, exact command, chosen port and HELLO identity, actual stdout/stderr/exit code, original CSV (including terminal provenance), and CAN analyzer/source artifacts where applicable. Record manual screen actions and visible active/saved/unsaved/error states. For loss tests also record offered traffic rate/mix, duration, bus/host conditions and independent counters. Do not replace missing board artifacts with fixture-generated frames or illustrative examples. If a prerequisite is unavailable mark that row **unverified** and name it; no empty capture or successful build can close the hardware acceptance gate.
