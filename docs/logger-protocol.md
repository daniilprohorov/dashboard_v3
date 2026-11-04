# UART v1 and event CSV reference

Companion to [operator/CLI guide](logger.md). This is the actual interface implemented by `main/logger.c`, `main/logger_protocol.c` and `tools/can_logger/can_logger/{protocol,transport,events,cli}.py`, not a proposed alternative format. The [acceptance record](logger-acceptance.md) separates executed host tests from unavailable hardware evidence. Unless explicitly labeled observed, all transcripts/CSV below are **illustrative valid syntax with invented times/identity**, not captured hardware output.

## Framing, identity and correlation

Transport: UART0 via CH343, fixed **2000000 8N1**, no flow control. Lines are printable ASCII, LF terminated; a single CR immediately before LF is accepted. Maximum content is **1024 bytes excluding LF and optional terminating CR**. Canonical output uses single ASCII spaces, no leading/trailing spaces; key/value tokens have exactly one `=`, nonempty values, no escaping or whitespace. Commands are uppercase and field names lowercase. The host rejects unknown/duplicate/missing response/event fields; field order is not significant. Do not emit JSON, arbitrary logs or binary data on this stream.

```text
@D1 Q <rid> <COMMAND> [key=value ...]
@D1 R <rid> OK [key=value ...]
@D1 R <rid> ERR code=<error_code>
@D1 F <frame fields>
@D1 S <common status fields>
@D1 E <common terminal fields>
```

`rid` is a positive decimal **1–2147483647**, correlating each response independently of asynchronous F/S/E lines. Each CLI connection increments from 1; unsolicited/duplicate responses are errors. A syntactically unsupported `@D<number>` request with a valid request ID gets a version-1 response with the same ID and `unsupported_version`. Malformed input without a usable ID can receive `@D1 R 0 ERR code=invalid_command`; 0 is a diagnostic for an uncorrelatable request, **not a valid host request/response ID**. The CLI rejects that response as a protocol error.

A session ID `sid` is exactly **16 uppercase hex digits**: 8 digits of random 32-bit boot nonce plus 8 of a monotonically incremented capture counter. It is not a clock, authentication token or guarantee against all cross-boot random collisions. A successful START creates a new ID, unique within that boot; exhaustion refuses a start rather than reusing an ID. Never join sessions by timestamp alone. `sid=none` means no successful capture yet. Last successful settings/counters/reason remain available after capture, until another successful START; a rejected START does not confirm a new configuration. No persistence, automatic resumption or replay of logger sessions exists.

Before a valid HELLO response the host ignores non-`@D` boot-noise lines only. A malformed protocol-prefixed line is never treated as boot noise, even before HELLO. After HELLO, non-ASCII/control bytes, overlength lines, nonprotocol output and malformed metadata are errors. A HELLO advertising `protocol!=1` is incompatible. Runtime console/logging must not interleave. A new explicit HELLO can re-establish framing after a broken inactive UART session; it cannot resume old capture or recover frames.

## All requests and successful responses

No arguments are accepted on HELLO/STATUS/CAPS. START takes exactly its required three fields and optional duration; STOP/ACK/PING require the current active session ID. Unknown, duplicate, empty and extra arguments fail.

| Request tail after request ID | Successful response fields |
|---|---|
| `HELLO` | `device=dashboard_v3 firmware=<token> protocol=1 mode=<mode> device_us=<us>` |
| `STATUS` | All common fields below. Does not change mode or refresh capture heartbeat. |
| `CAPS` | Exact capability fields below. Does not change mode or refresh heartbeat. |
| `START bitrate=<decimal> ids=11\|29\|both filter=<filter> [duration_ms=<positive decimal>]` | `sid=<SID> device_us=<start_us> bitrate=<effective> requested_bitrate=<requested> ids=<type> filter=<canonical> listen_only=1` |
| `STOP sid=<SID> reason=ctrl_c\|host_stop` | Bare `OK`, **after** queued frames and terminal E. |
| `ACK sid=<SID> seq=<decimal>` | Bare `OK`; may then emit a client-state S. |
| `PING sid=<SID>` | Bare `OK`; may then emit a client-state S. |

The literal pipe in table cells denotes alternatives, not a transmitted character. Exact CAPS payload:

```text
min_bitrate=1000 max_bitrate=1000000 exact_only=1 id_types=11,29,both max_ranges=16 uart_baud=2000000 data_bits=8 parity=N stop_bits=1 listen_only=1
```

Firmware version is the ESP application version sanitized into one printable token (whitespace/`=` replaced, maximum 63 characters; empty becomes `unknown`). Modes are `dashboard`, `logger_idle`, `capturing`; Settings still reports `dashboard` because it does not stop OBD.

START requires manual Logger entry; it never switches the UI remotely. Only exact hardware-achievable CAN bitrates 1000–1000000 bit/s are allowed, without rounding. Effective/requested bitrate must match. `filter=all` admits all selected ID formats. Otherwise filter syntax is comma-separated `HEX` or `HEX-HEX`, 1–8 hex digits per bound without `0x`, inclusive OR, ≤`1FFFFFFF` (≤`7FF` for `ids=11`). Sort/merge overlap and adjacency to at most 16 disjoint ranges. Canonical output pads each bound to 3 digits for `11` or 8 for `29`/`both`, uppercase; singleton ranges omit the dash. For `both`, the numeric test applies to both formats; F retains the independent format. Excluded frames do not consume sequence numbers or count as software drops.

Optional `duration_ms` is a positive decimal integer, parser-limited to `INT64_MAX/1000` and further limited so `start_us + duration_ms*1000` fits signed 64-bit microseconds. Omit it for no timer. The CLI rounds seconds up to milliseconds; very large CLI-valid uint64-millisecond durations can still fail this tighter firmware bound. Deadline and heartbeat use the device monotonic clock. Valid ACK/PING for the active SID alone refresh the **3-second** service timeout. ACK may remain equal but cannot regress or exceed the highest fully emitted frame (`tx_seq`); ACK 0 is valid before any frame. PING is a quiet heartbeat without changing the ACK. STOP terminates instead of refreshing. HELLO/STATUS/CAPS, invalid requests, stale SIDs and UART TX do not maintain capture. The CLI sends ACK every 0.75 s even on a silent bus, never ACKs a frame it did not parse, and does not require PING.

START OK precedes **every asynchronous event belonging to that new session**. The device emits an initial S, state-change S (including first valid client service), and periodic S at approximately 1-second intervals while capturing, even with no CAN frames. Serial/task scheduling and blocked output affect cadence; this is not a hard real-time delivery guarantee. Responses, S and F can interleave; correlate R by ID rather than assuming the next line is a command result. At stop, physical CAN/callback ownership is released before potentially slow UART output; already queued frames are drained in order before E. A successful STOP's E is emitted before its bare OK. The CLI can finish on E without waiting for the trailing STOP response. UI Exit's E reports dashboard mode after successful release; other terminal events normally report logger_idle. Failed release prevents OBD restart; hardware failure may replace the requested reason.

## Common status / terminal fields

Both `@D1 S`, `@D1 E` and successful STATUS contain exactly these fields:

```text
mode sid device_us bitrate requested_bitrate ids filter listen_only seq tx_seq ack sw_drops hw_overruns hw_lost tx client reason
```

| Field | Type, units and meaning |
|---|---|
| `mode` | `dashboard`, `logger_idle` or `capturing`. |
| `sid` | Current/last successful 16-hex session ID, or `none`. |
| `device_us` | Decimal boot-monotonic microseconds at status serialization. |
| `bitrate`, `requested_bitrate` | Effective/requested CAN bit/s; both 0 before configuration. |
| `ids`, `filter` | Confirmed `11`/`29`/`both` and normalized filter; both `none` before configuration. |
| `listen_only` | Always `1`, describing logger capture, not an assertion that dashboard OBD is passive. |
| `seq` | Last assigned accepted-frame sequence (0 initially), including queue-rejected new frames. |
| `tx_seq` | Last fully UART-emitted frame sequence (0 initially); not a frame count when gaps exist. |
| `ack` | Last valid host-confirmed received frame sequence (0 initially), not disk durability. |
| `sw_drops` | Exact count of accepted frames rejected by the bounded software queue (drop-new). |
| `hw_overruns` | Detected hardware RX FIFO overflow **event count** since capture baseline, not lost-frame count. |
| `hw_lost` | Literal `unknown`, even with zero detected overflow events; exact lost-frame cardinality unavailable. |
| `tx` | `1` when capture's UART output is enabled/healthy; `0` idle/ended/unavailable. Not reader detection. |
| `client` | `1` only after valid ACK/PING in current session; false initially and on termination. |
| `reason` | `none`, `duration`, `ctrl_c`, `host_stop`, `user_exit`, `link_loss`, `hardware_error`. |

Counters/times are unsigned decimal on wire; the host validates up to uint64, and nondecreasing counters within a session. Invariants include `ack <= tx_seq <= seq`, `sw_drops <= seq`. A capturing status has SID/configuration, `reason=none`, `tx=1`; E has a bound SID/configuration, noncapturing mode, `tx=0`, and a non-`none` reason. Device-confirmed E is the source of final counters, not merely a previously received S. After end, a status retains the last capture's configuration/counters/reason but a fresh device status time. UI/controller errors are separately visible on-screen, not an extra undocumented UART field.

## Frame fields and clock/loss semantics

```text
@D1 F sid=<SID> seq=<decimal> us=<decimal> bits=11|29 id=<HEX> rtr=0|1 dlc=0..8 data=<HEX|->
```

- `seq` starts at 1 and is assigned **after ID/type filtering, before enqueue**. Surviving frames preserve FIFO order; duplicates/regressions are protocol errors, forward gaps are valid loss evidence. Sequence does not enumerate filtered-out frames or losses before assignment.
- `us` is `esp_timer_get_time()` sampled immediately after `twai_node_receive_from_isr` dequeues the frame. It is **boot-relative**, 1 µs resolution, not host UTC/arrival or hardware wire SOF. ISR/FIFO latency and accuracy are unmeasured. A new boot resets its origin; START `device_us` binds the session. Host per-row UTC is not precise clock synchronization.
- `bits` is actual 11/29-bit format, not inferred from ID magnitude. `id` is uppercase zero-padded **3 or 8 hex digits** respectively, max `7FF` or `1FFFFFFF`.
- `rtr` and `dlc` are independent metadata. Classic DLC values 9–15 are exported as eight bytes; this interface does not preserve those original DLC codes. For a data frame with DLC 1–8, `data` has exactly `2*dlc` uppercase hex digits, including leading zero bytes. For RTR **regardless of DLC**, or a zero-length data frame, `data=-`; RTR DLC still records the requested byte length. CAN FD is not exported.
- Software queue length is 128; excess **new** matching frames increment `sw_drops` rather than evict old frames. Hardware overrun events come from a separate low-level observed-event counter and never claim exact lost frames. Hardware fault/UART failure can prevent delivery of a terminal line: do not manufacture final counts.
- UART 8N1's 2,000,000/10 = **200,000 bytes/s theoretical ceiling** precedes ASCII/control overhead. No measured sustained or lossless throughput is claimed. ACK proves host parsing, not `fsync`, durable storage or no upstream loss. No flow control/retransmission repairs gaps.

## Errors and host stream failures

Every normal device rejection is `@D1 R <rid> ERR code=<code>`:

| Code | Meaning |
|---|---|
| `unsupported_version` | Unsupported request version (response still `@D1`). |
| `invalid_command` | Unknown/malformed command or framing/nonprintable/overlength request. Unusable rid can yield diagnostic rid 0. |
| `invalid_argument` | Missing/extra/duplicate/invalid fields, ID/filter/duration bounds, malformed SID or invalid ACK progression. |
| `invalid_bitrate` | Nondecimal/out-of-range or exactly unreachable bitrate. |
| `logger_required` | START while not manually in Logger. |
| `busy` | START while active or unreleased hardware resources remain. |
| `invalid_session` | STOP/ACK/PING references a well-formed but noncurrent/inactive SID. |
| `hardware_error` | CAN/UART/timing/capture lifecycle failure; no new successful configuration implied. |

The host also rejects unrelated SID events, malformed field sets, changed effective config, mismatched START confirmation, invalid frame ID/data/DLC/type/filter, impossible ACK/counters, regressing timestamp/sequence/counters, event after E, wrong response ID and incompatible CAPS/HELLO. After HELLO, corrupt/non-ASCII/overlength lines are fatal `protocol_error`, not ignored boot output. Command responses time out at 3 seconds. Active capture fails locally after 3 seconds without valid messages, after 3 seconds beyond a requested duration with no E, or 3 seconds after STOP without E. There are no automatic retries or reconnects; see [exit/JSON rules](logger.md#json-results-and-stable-exit-codes).

## Illustrative wire transcript

These deliberately chosen numbers are **not observed device output**. The excerpt demonstrates legal lines, not a wall-clock schedule; periodic S and intermediate ACK requests are omitted. For an actual run retain complete request/response order and provenance.

```text
@D1 Q 1 HELLO
@D1 R 1 OK device=dashboard_v3 firmware=example-build protocol=1 mode=logger_idle device_us=900000
@D1 Q 2 START bitrate=500000 ids=both filter=00000001 duration_ms=1000
@D1 R 2 OK sid=0123456700000001 device_us=1000000 bitrate=500000 requested_bitrate=500000 ids=both filter=00000001 listen_only=1
@D1 S mode=capturing sid=0123456700000001 device_us=1000010 bitrate=500000 requested_bitrate=500000 ids=both filter=00000001 listen_only=1 seq=0 tx_seq=0 ack=0 sw_drops=0 hw_overruns=0 hw_lost=unknown tx=1 client=0 reason=none
@D1 F sid=0123456700000001 seq=1 us=1000100 bits=11 id=001 rtr=0 dlc=2 data=0001
@D1 F sid=0123456700000001 seq=2 us=1000200 bits=29 id=00000001 rtr=1 dlc=8 data=-
@D1 Q 3 ACK sid=0123456700000001 seq=2
@D1 R 3 OK
@D1 S mode=capturing sid=0123456700000001 device_us=1000300 bitrate=500000 requested_bitrate=500000 ids=both filter=00000001 listen_only=1 seq=2 tx_seq=2 ack=2 sw_drops=0 hw_overruns=0 hw_lost=unknown tx=1 client=1 reason=none
@D1 E mode=logger_idle sid=0123456700000001 device_us=2000000 bitrate=500000 requested_bitrate=500000 ids=both filter=00000001 listen_only=1 seq=2 tx_seq=2 ack=2 sw_drops=0 hw_overruns=0 hw_lost=unknown tx=0 client=0 reason=duration
```

Independent error/command syntax examples (not continuations of that completed session):

```text
@D2 Q 10 HELLO
@D1 R 10 ERR code=unsupported_version
@D1 Q 11 UNKNOWN
@D1 R 11 ERR code=invalid_command
@D1 Q 12 START bitrate=333333 ids=11 filter=all
@D1 R 12 ERR code=invalid_bitrate
@D1 Q 13 PING sid=0123456700000001
@D1 R 13 ERR code=invalid_session
```

For the unreachable-rate example Logger is idle; START on dashboard instead fails `logger_required` before timing validation. For graceful manual protocol stopping use `@D1 Q <rid> STOP sid=<active SID> reason=host_stop`, read queued F/E, then correlated bare OK. A SID from an ended session is not reusable for service requests.

## Event CSV: constant 30-column schema

UTF-8 CSV written with Python's CSV writer (comma delimiter, normal quoting and CRLF record terminators), header exactly:

```csv
event,session_id,firmware_version,protocol_version,cli_version,host_utc,device_us,requested_bitrate,effective_bitrate,id_type,filter,listen_only,uart_baud,uart_data_bits,uart_parity,uart_stop_bits,seq,id_bits,can_id,rtr,dlc,data_hex,state,tx_seq,ack_seq,sw_drops,hw_overruns,hw_lost_frames,reason,source
```

All rows contain `event`, `session_id`, `host_utc`; the host timestamp is ISO 8601 UTC, six fractional digits and `Z`, taken as the row is produced. Other inapplicable fields are **empty**, not synthetic zero. Numeric fields are decimal text except `can_id`/`data_hex`/`session_id` and hex filter bounds. Parse with a real CSV parser: filters with multiple ranges require quoted cells. Preserve textual leading zeros.

| Columns | Population / interpretation |
|---|---|
| `event` | Exactly `start`, `config`, `frame`, `status`, `end`. |
| `session_id` | 16-uppercase-hex START-confirmed ID, on every event. |
| `firmware_version`, `protocol_version`, `cli_version` | `start` only; sanitized HELLO firmware token, `1`, host package version. |
| `host_utc` | All events; host row-production UTC, not synchronized frame UTC. |
| `device_us` | `start`: START binding; `frame`: ISR receive timestamp (`us`); `status`: device status time; device `end`: terminal time. Empty for config/local end. |
| `requested_bitrate`, `effective_bitrate` | `config` only, CAN bit/s, separately named even though exact-only requires equality. |
| `id_type`, `filter` | `config` only, confirmed `11`/`29`/`both` and normalized filter. |
| `listen_only` | `config` only, `1`. |
| `uart_baud`, `uart_data_bits`, `uart_parity`, `uart_stop_bits` | `config` only, `2000000,8,N,1`. |
| `seq` | `frame`: its assigned sequence; `status`/device `end`: last assigned sequence (not received-frame count). |
| `id_bits`, `can_id` | `frame` only: actual `11`/`29`, padded uppercase ID hex. |
| `rtr`, `dlc`, `data_hex` | `frame` only: `0`/`1`, 0–8 decimal, exact uppercase payload hex or literal `-` for RTR/zero-length. |
| `state` | `status`/device `end`: wire mode. Empty for local end. |
| `tx_seq`, `ack_seq` | `status`/device `end`: wire `tx_seq` and `ack`. |
| `sw_drops`, `hw_overruns`, `hw_lost_frames` | `status`/device `end`: exact software rejection count, hardware event count, literal `unknown`. |
| `reason` | `status`: wire reason, generally `none`; `end`: device reason or local failure code. |
| `source` | `status`: `device`; `end`: `device` with validated E, otherwise `cli`; other events empty. |

`start` and `config` appear only after a successful START; the output header can preexist START rejection. `status` rows represent actual async S or supported service-response metadata, never extra CAN frames; the current firmware's ACK/PING/STOP OK is bare. The host does not copy `tx`/`client` flags to separate CSV columns; sequence/ACK metadata and source/reason preserve the exported status contract. Keep the complete event stream, not just payload rows.

### Complete illustrative CSV excerpt

The sample corresponds to the two illustrative frames above; it deliberately omits intermediate status events. Values and host times are invented, not board measurements. Both IDs are numeric 1 but remain distinct; the RTR requests DLC 8 with no payload. Each row uses the same header:

```csv
event,session_id,firmware_version,protocol_version,cli_version,host_utc,device_us,requested_bitrate,effective_bitrate,id_type,filter,listen_only,uart_baud,uart_data_bits,uart_parity,uart_stop_bits,seq,id_bits,can_id,rtr,dlc,data_hex,state,tx_seq,ack_seq,sw_drops,hw_overruns,hw_lost_frames,reason,source
start,0123456700000001,example-build,1,1.0.0,2026-11-05T12:00:00.000000Z,1000000,,,,,,,,,,,,,,,,,,,,,,,
config,0123456700000001,,,,2026-11-05T12:00:00.000001Z,,500000,500000,both,00000001,1,2000000,8,N,1,,,,,,,,,,,,,,
frame,0123456700000001,,,,2026-11-05T12:00:00.000100Z,1000100,,,,,,,,,,1,11,001,0,2,0001,,,,,,,,
frame,0123456700000001,,,,2026-11-05T12:00:00.000200Z,1000200,,,,,,,,,,2,29,00000001,1,8,-,,,,,,,,
status,0123456700000001,,,,2026-11-05T12:00:00.000300Z,1000300,,,,,,,,,,2,,,,,,capturing,2,2,0,0,unknown,none,device
end,0123456700000001,,,,2026-11-05T12:00:01.000000Z,2000000,,,,,,,,,,2,,,,,,logger_idle,2,2,0,0,unknown,duration,device
```

A local terminal row instead carries **no** final counters/device time/state, even if prior status counters were received:

```csv
end,0123456700000001,,,,2026-11-05T12:00:01.000000Z,,,,,,,,,,,,,,,,,,,,,,,link_loss,cli
```

JSON uses `null` for these unavailable counters; CSV uses empty cells. `source=device` confirms E, not durability on disk. Files flush at creation/start/end and periodically (0.25 s), close normally on graceful stop, but do not call `fsync`. A disk error can prevent even the local end row from being written; abrupt termination may leave a partial last row. **No end event is an incomplete session**. Do not synthesize final counters, treat a header-only rejection as configured, or auto-resume to conceal a gap. Preserve the parseable prefix and use a new output/session for a subsequent explicit capture.
