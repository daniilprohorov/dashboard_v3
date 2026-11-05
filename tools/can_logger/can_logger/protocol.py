"""Strict, bounded UART v1 grammar and device metadata validation."""

import re
from dataclasses import dataclass


class Failure(Exception):
    def __init__(self, exit_code, code, message, *, torn=False):
        super().__init__(message)
        self.exit_code = exit_code
        self.code = code
        # True only for a line torn on the UART link (framing damage), not for
        # a well-framed device message whose fields violate the contract.
        self.torn = torn


def invalid(message):
    raise Failure(4, "protocol_error", message)


def torn(message):
    raise Failure(4, "protocol_error", message, torn=True)


def require(condition, message):
    if not condition:
        invalid(message)


def integer(value, name, minimum=0, maximum=18446744073709551615):
    require(isinstance(value, str) and re.fullmatch(r"[0-9]+", value) is not None,
            f"Invalid decimal {name}")
    # Bound length before int conversion, including adversarial leading zeroes.
    require(len(value) <= 20, f"Out-of-range {name}")
    number = int(value)
    require(minimum <= number <= maximum, f"Out-of-range {name}")
    return number


def keys(fields, expected):
    require(set(fields) == set(expected.split()), "Unexpected or missing message fields")


def sid(value, allow_none=False):
    require((allow_none and value == "none") or
            re.fullmatch(r"[0-9A-F]{16}", value) is not None, "Invalid session ID")


def normalize_ranges(items, ids):
    """Parse user/wire filters; return merged inclusive ranges and wire text."""
    ranges = []
    for item in items:
        match = re.fullmatch(r"([0-9A-Fa-f]{1,8})(?:-([0-9A-Fa-f]{1,8}))?", item)
        if match is None:
            raise ValueError("Allowlist entries must be HEX or HEX-HEX (without 0x)")
        low = int(match[1], 16)
        high = int(match[2], 16) if match[2] else low
        maximum = 0x7FF if ids == "11" else 0x1FFFFFFF
        if low > high or high > maximum:
            raise ValueError("Allowlist range is reversed or exceeds the selected ID type")
        ranges.append((low, high))
    merged = []
    for low, high in sorted(ranges):
        if merged and low <= merged[-1][1] + 1:
            merged[-1] = (merged[-1][0], max(high, merged[-1][1]))
        else:
            merged.append((low, high))
    if len(merged) > 16:
        raise ValueError("At most 16 disjoint allowlist ranges are supported")
    width = 3 if ids == "11" else 8
    text = ",".join(f"{low:0{width}X}" if low == high else
                    f"{low:0{width}X}-{high:0{width}X}" for low, high in merged)
    return merged, text or "all"


def wire_filter(value, ids):
    if value == "all":
        return []
    try:
        ranges, normalized = normalize_ranges(value.split(","), ids)
    except ValueError as exc:
        invalid(str(exc))
    require(value == normalized, "Noncanonical effective filter")
    return ranges


MODES = {"dashboard", "logger_idle", "capturing"}
REASONS = {"none", "duration", "ctrl_c", "host_stop", "user_exit", "link_loss", "hardware_error"}
CONFIG_KEYS = "bitrate requested_bitrate ids filter listen_only"
COMMON_KEYS = ("mode sid device_us " + CONFIG_KEYS +
               " seq tx_seq ack sw_drops hw_overruns hw_lost tx client reason")


def config(fields, allow_empty=False):
    integer(fields["bitrate"], "bitrate", 0 if allow_empty else 1000, 1000000)
    integer(fields["requested_bitrate"], "requested_bitrate", 0 if allow_empty else 1000, 1000000)
    require(fields["listen_only"] == "1", "Device is not listen-only")
    if allow_empty and fields["ids"] == "none":
        require(fields["bitrate"] == fields["requested_bitrate"] == "0" and
                fields["filter"] == "none", "Inconsistent unconfigured device")
    else:
        require(fields["ids"] in {"11", "29", "both"}, "Invalid ID configuration")
        require(int(fields["bitrate"]) >= 1000 and int(fields["requested_bitrate"]) >= 1000,
                "Configured bitrate is zero")
        require(fields["bitrate"] == fields["requested_bitrate"], "Device rounded CAN bitrate")
        wire_filter(fields["filter"], fields["ids"])


def common(fields, end=False):
    keys(fields, COMMON_KEYS)
    require(fields["mode"] in MODES, "Invalid device mode")
    sid(fields["sid"], allow_none=True)
    integer(fields["device_us"], "device_us")
    config(fields, allow_empty=True)
    for name in ("seq", "tx_seq", "ack", "sw_drops", "hw_overruns"):
        integer(fields[name], name)
    require(int(fields["ack"]) <= int(fields["tx_seq"]) <= int(fields["seq"]),
            "Impossible device sequence/ACK counters")
    require(int(fields["sw_drops"]) <= int(fields["seq"]), "Impossible software drops")
    require(fields["hw_lost"] == "unknown", "Hardware lost-frame count must be unknown")
    require(fields["tx"] in {"0", "1"} and fields["client"] in {"0", "1"},
            "Invalid device connection flags")
    require(fields["reason"] in REASONS, "Invalid termination reason")
    if fields["sid"] == "none":
        require(fields["ids"] == "none" and fields["mode"] != "capturing" and
                all(fields[name] == "0" for name in ("seq", "tx_seq", "ack", "sw_drops", "hw_overruns")) and
                fields["reason"] == "none", "Unbound session has capture metadata")
    else:
        require(fields["ids"] != "none", "Bound session lacks configuration")
    if fields["mode"] == "capturing":
        require(fields["sid"] != "none" and fields["reason"] == "none" and fields["tx"] == "1",
                "Invalid capturing status")
    if end:
        require(fields["sid"] != "none" and fields["reason"] != "none" and
                fields["mode"] != "capturing" and fields["tx"] == "0", "Invalid terminal status")


def hello(fields):
    keys(fields, "device firmware protocol mode device_us")
    require(fields["device"] == "dashboard_v3", "Serial peer is not dashboard_v3")
    if fields["protocol"] != "1":
        raise Failure(4, "unsupported_version", "Device HELLO protocol is not 1")
    require(fields["mode"] in MODES, "Invalid HELLO mode")
    integer(fields["device_us"], "device_us")


def capabilities(fields):
    expected = {"min_bitrate": "1000", "max_bitrate": "1000000", "exact_only": "1",
                "id_types": "11,29,both", "max_ranges": "16", "uart_baud": "2000000",
                "data_bits": "8", "parity": "N", "stop_bits": "1", "listen_only": "1"}
    require(fields == expected, "Incompatible device capabilities")


def start(fields):
    keys(fields, "sid device_us " + CONFIG_KEYS)
    sid(fields["sid"])
    integer(fields["device_us"], "device_us")
    config(fields)


def frame(fields):
    keys(fields, "sid seq us bits id rtr dlc data")
    sid(fields["sid"])
    integer(fields["seq"], "seq", 1)
    integer(fields["us"], "us")
    require(fields["bits"] in {"11", "29"}, "Invalid frame ID type")
    width = 3 if fields["bits"] == "11" else 8
    require(re.fullmatch(r"[0-9A-F]{%d}" % width, fields["id"]) is not None,
            "Invalid or unpadded CAN ID")
    require(int(fields["id"], 16) <= (0x7FF if width == 3 else 0x1FFFFFFF), "CAN ID out of range")
    require(fields["rtr"] in {"0", "1"}, "Invalid RTR")
    dlc = integer(fields["dlc"], "dlc", 0, 8)
    if fields["rtr"] == "1" or dlc == 0:
        require(fields["data"] == "-", "RTR/empty frame must have '-' payload")
    else:
        require(re.fullmatch(r"[0-9A-F]{%d}" % (2 * dlc), fields["data"]) is not None,
                "Payload disagrees with DLC")


@dataclass
class Message:
    kind: str
    fields: dict
    rid: int = 0
    ok: bool = True


def parse(raw):
    if raw.endswith(b"\r"):
        raw = raw[:-1]
    require(len(raw) <= 1024, "Overlength protocol line")
    try:
        text = raw.decode("ascii")
    except UnicodeDecodeError:
        torn("Non-ASCII protocol line")
    if not all(32 <= ord(char) <= 126 for char in text):
        torn("Control byte in protocol line")
    tokens = text.split(" ")
    if len(tokens) < 2 or not all(tokens):
        torn("Invalid protocol spacing")
    if tokens[0] != "@D1":
        if re.fullmatch(r"@D[0-9]+", tokens[0]):
            raise Failure(4, "unsupported_version", "Unsupported UART protocol version")
        torn("Missing @D1 protocol prefix")
    kind = tokens[1]
    rid = 0
    ok = True
    if kind == "R":
        require(len(tokens) >= 4, "Truncated command response")
        rid = integer(tokens[2], "request ID", 1, 2147483647)
        require(tokens[3] in {"OK", "ERR"}, "Invalid response disposition")
        ok = tokens[3] == "OK"
        values = tokens[4:]
    else:
        require(kind in {"F", "S", "E"}, "Unknown protocol message type")
        values = tokens[2:]
    fields = {}
    for token in values:
        if token.count("=") != 1:
            torn("Malformed key=value token")
        key, value = token.split("=", 1)
        if re.fullmatch(r"[a-z][a-z0-9_]*", key) is None or not value or key in fields:
            torn("Duplicate/invalid protocol field")
        fields[key] = value
    if kind == "R" and not ok:
        keys(fields, "code")
        require(re.fullmatch(r"[a-z][a-z0-9_]*", fields["code"]) is not None, "Invalid device error code")
    elif kind == "F":
        frame(fields)
    elif kind in {"S", "E"}:
        common(fields, end=kind == "E")
    return Message(kind, fields, rid, ok)
