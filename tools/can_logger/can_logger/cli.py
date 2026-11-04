"""Public dashboard-can commands and truthful capture termination."""

import argparse
import json
import signal
import sys
import time
from decimal import Decimal, InvalidOperation, ROUND_CEILING

from serial.tools import list_ports

from . import __version__
from . import protocol as wire
from .events import EventCSV, configuration, counters
from .protocol import Failure
from .transport import Connection, RESPONSE_TIMEOUT


class Parser(argparse.ArgumentParser):
    def error(self, message):
        raise Failure(2, "argument_error", message)


def parser():
    root = Parser(prog="dashboard-can", description="Passive dashboard_v3 CAN capture over 2 Mbaud UART.")
    root.add_argument("--version", action="version", version=__version__)
    commands = root.add_subparsers(dest="command", required=True, parser_class=Parser)
    commands.add_parser("ports", help="List all ports; HELLO-probe USB candidates only; never auto-select")
    for command in ("status", "capabilities"):
        sub = commands.add_parser(command, help=f"Read device {command} without changing mode")
        sub.add_argument("--port", required=True, help="Explicit UART device path")
    capture = commands.add_parser("capture", help="Capture after manually opening Logger on the board")
    capture.add_argument("--port", required=True)
    capture.add_argument("--bitrate", required=True, help="Exact CAN bitrate, 1000..1000000 bit/s")
    capture.add_argument("--ids", choices=("11", "29", "both"), required=True)
    capture.add_argument("--allow", action="append", default=[], metavar="HEX[-HEX]",
                         help="Repeatable inclusive allowlist; empty means all selected IDs")
    capture.add_argument("--output", required=True, help="New event CSV path (never overwrites)")
    capture.add_argument("--duration", help="Positive seconds; device owns the timer (rounded up to milliseconds)")
    return root


def capture_arguments(args):
    if not args.port or not args.output:
        raise Failure(2, "argument_error", "Port and output path must not be empty")
    if not args.bitrate.isascii() or not args.bitrate.isdecimal() or len(args.bitrate) > 7:
        raise Failure(2, "argument_error", "Bitrate must be a decimal integer")
    args.bitrate = int(args.bitrate)
    if not 1000 <= args.bitrate <= 1000000:
        raise Failure(2, "argument_error", "Bitrate must be between 1000 and 1000000 bit/s")
    try:
        args.ranges, args.filter = wire.normalize_ranges(args.allow, args.ids)
    except ValueError as exc:
        raise Failure(2, "argument_error", str(exc)) from exc
    args.duration_ms = None
    if args.duration is not None:
        try:
            seconds = Decimal(args.duration)
            if not seconds.is_finite() or seconds <= 0 or seconds > Decimal("18446744073709551.615"):
                raise ValueError("Duration must be finite, positive, and fit uint64 milliseconds")
            args.duration_ms = (1 if seconds <= Decimal("0.001") else
                                int((seconds * 1000).to_integral_value(rounding=ROUND_CEILING)))
        except (InvalidOperation, ValueError, OverflowError) as exc:
            raise Failure(2, "argument_error", "Duration must be positive finite seconds fitting uint64 milliseconds") from exc


def error_result(command, failure):
    return {"ok": False, "command": command,
            "error": {"code": failure.code, "message": str(failure)}}


class Capture:
    def __init__(self, sink, start, hello, args):
        self.sink = sink
        self.start = start
        self.hello = hello
        self.args = args
        self.sid = start["sid"]
        self.last_frame = 0
        self.last_frame_us = 0
        self.last_status = None
        self.ack_sent = 0
        self.final = None

    def event(self, message):
        fields = message.fields
        wire.require(fields["sid"] == self.sid, "Async event has unrelated session ID")
        wire.require(self.final is None, "Event after terminal session event")
        if message.kind == "F":
            sequence = int(fields["seq"])
            stamp = int(fields["us"])
            wire.require(sequence > self.last_frame, "Frame sequence duplicated or regressed")
            if self.last_status:
                wire.require(sequence > int(self.last_status["tx_seq"]),
                             "Frame sequence regressed behind emitted status")
            wire.require(stamp >= self.last_frame_us, "Frame timestamp regressed")
            wire.require(self.args.ids == "both" or fields["bits"] == self.args.ids,
                         "Frame violates configured ID type")
            can_id = int(fields["id"], 16)
            wire.require(not self.args.ranges or any(low <= can_id <= high for low, high in self.args.ranges),
                         "Frame violates configured allowlist")
            self.last_frame = sequence
            self.last_frame_us = stamp
            self.sink.frame(fields)
            return
        for key in wire.CONFIG_KEYS.split():
            wire.require(fields[key] == self.start[key], "Session configuration changed")
        wire.require(int(fields["device_us"]) >= int(self.start["device_us"]), "Status predates session")
        wire.require(int(fields["seq"]) >= self.last_frame and int(fields["tx_seq"]) >= self.last_frame,
                     "Status counters precede received frames")
        wire.require(int(fields["ack"]) <= self.ack_sent, "Device ACK exceeds host confirmations")
        if self.last_status:
            for key in ("seq", "tx_seq", "ack", "sw_drops", "hw_overruns", "device_us"):
                wire.require(int(fields[key]) >= int(self.last_status[key]), f"Device {key} counter regressed")
        self.last_status = fields
        if message.kind == "E":
            wire.require(fields["reason"] != "duration" or self.args.duration_ms is not None,
                         "Untimed session ended with duration")
            self.final = fields
            self.sink.end(fields["reason"], fields)
        else:
            self.sink.status(fields)

    def result(self, reason, failure=None):
        result = {"ok": failure is None, "command": "capture", "session_id": self.sid,
                  "firmware_version": self.hello["firmware"], "protocol_version": 1,
                  "cli_version": __version__, "config": configuration(self.start),
                  "counters": counters(self.final), "source": "device" if self.final else "cli",
                  "reason": self.final["reason"] if self.final else reason,
                  "output": self.args.output}
        if failure:
            result["error"] = {"code": failure.code, "message": str(failure)}
        return result


def terminal_failure(fields):
    reason = fields["reason"]
    if reason in {"duration", "ctrl_c", "host_stop"}:
        return None
    if reason == "user_exit":
        return Failure(7, "user_exit", "User left Logger on the device")
    if reason == "link_loss":
        return Failure(6, "link_loss", "Device ended capture after losing the host heartbeat")
    return Failure(5, "hardware_error", "Device ended capture after a hardware error")


def run_capture(args):
    connection = None
    sink = None
    capture = None
    failure = None
    reason = None
    interrupted = 0

    def interrupt(_signum, _frame):
        nonlocal interrupted
        interrupted += 1

    previous = signal.signal(signal.SIGINT, interrupt)
    try:
        connection = Connection(args.port)
        hello = connection.request("HELLO")
        if interrupted:
            raise Failure(6, "interrupted", "Interrupted before capture started")
        # Exclusive-create the sink before START. A rejected START leaves only
        # a header, never a successful configuration or an invented session.
        sink = EventCSV(args.output)
        if interrupted:
            raise Failure(6, "interrupted", "Interrupted before capture started")
        arguments = {"bitrate": args.bitrate, "ids": args.ids, "filter": args.filter}
        if args.duration_ms is not None:
            arguments["duration_ms"] = args.duration_ms
        started = connection.request("START", **arguments)
        capture = Capture(sink, started, hello, args)
        connection.on_event = capture.event
        sink.start(started, hello)
        started_at = time.monotonic()
        next_ack = started_at
        stop_at = None
        while capture.final is None:
            now = time.monotonic()
            if interrupted > 1:
                raise Failure(6, "interrupted", "Interrupted while awaiting device-confirmed stop")
            if interrupted and stop_at is None:
                connection.send("STOP", sid=capture.sid, reason="ctrl_c")
                stop_at = now
            if stop_at is not None and now - stop_at >= RESPONSE_TIMEOUT:
                raise Failure(6, "stop_timeout", "No device END within 3 seconds of STOP")
            if args.duration_ms is not None and now - started_at >= args.duration_ms / 1000 + 3:
                raise Failure(6, "end_timeout", "No device END within 3 seconds of the capture deadline")
            if now - connection.last_rx >= 3:
                raise Failure(6, "link_loss", "No valid serial messages for 3 seconds")
            if now >= next_ack:
                # ACK refers only to a fully parsed received frame, not TX or
                # the device-assigned count. ACK(0) is also a quiet heartbeat.
                capture.ack_sent = capture.last_frame
                connection.send("ACK", sid=capture.sid, seq=capture.ack_sent)
                next_ack = now + 0.75
            connection.receive()
            if capture.final is None:
                connection.check_timeouts()
            sink.flush()
        failure = terminal_failure(capture.final)
        reason = capture.final["reason"]
    except Failure as exc:
        failure = exc
        if capture is not None and capture.final is None:
            if exc.exit_code == 3:
                # A missing command response can coexist with healthy frames.
                # Only actual transport failures become link loss.
                code = exc.code if exc.code == "response_timeout" else "link_loss"
                failure = Failure(6, code, str(exc))
            reason = failure.code
            try:
                sink.end(reason)
            except Failure as output_failure:
                failure = output_failure
        else:
            reason = exc.code
    finally:
        # Close both resources even when one close fails. Never turn an output
        # error into success merely because END was received from the device.
        for resource in (sink, connection):
            if resource is not None:
                try:
                    resource.close()
                except Failure as exc:
                    if failure is None or exc.exit_code == 8:
                        failure = exc
        signal.signal(signal.SIGINT, previous)
    if capture is not None:
        result = capture.result(reason, failure)
    else:
        result = error_result("capture", failure)
        result.update(session_id=None, config=None, counters=counters(), source="cli",
                      reason=reason, output=args.output)
    return result, failure.exit_code if failure else 0


def run_query(args):
    connection = Connection(args.port)
    failure = None
    result = None
    try:
        hello = connection.request("HELLO")
        fields = connection.request("STATUS" if args.command == "status" else "CAPS")
        result = {"ok": True, "command": args.command, "port": args.port,
                  "device": hello["device"], "firmware_version": hello["firmware"],
                  "protocol_version": 1, "cli_version": __version__,
                  "status" if args.command == "status" else "capabilities": fields}
    except Failure as exc:
        failure = exc
    finally:
        try:
            connection.close()
        except Failure as exc:
            if failure is None:
                failure = exc
    if failure:
        raise failure
    return result, 0


def discover():
    try:
        candidates = sorted(list_ports.comports(), key=lambda item: item.device)
    except OSError as exc:
        raise Failure(3, "port_error", f"Cannot enumerate serial ports: {exc}") from exc
    ports = []
    for candidate in candidates:
        entry = {"port": candidate.device, "description": candidate.description,
                 "hwid": candidate.hwid, "vid": candidate.vid, "pid": candidate.pid,
                 "serial_number": candidate.serial_number, "is_dashboard": False,
                 "probed": candidate.vid is not None}
        if not entry["probed"]:
            entry["skip_reason"] = "no_usb_vid"
            ports.append(entry)
            continue
        connection = None
        try:
            connection = Connection(candidate.device)
            hello = connection.request("HELLO")
            entry.update(is_dashboard=True, device=hello["device"], firmware_version=hello["firmware"],
                         protocol_version=1, mode=hello["mode"])
        except Failure as exc:
            entry["error"] = {"code": exc.code, "message": str(exc)}
        finally:
            if connection is not None:
                try:
                    connection.close()
                except Failure as exc:
                    entry["error"] = {"code": exc.code, "message": str(exc)}
        ports.append(entry)
    return {"ok": True, "command": "ports", "ports": ports}, 0


def main(argv=None):
    command = None
    try:
        args = parser().parse_args(argv)
        command = args.command
        if command == "capture":
            capture_arguments(args)
            result, exit_code = run_capture(args)
        elif command == "ports":
            result, exit_code = discover()
        else:
            if not args.port:
                raise Failure(2, "argument_error", "Port must not be empty")
            result, exit_code = run_query(args)
    except Failure as exc:
        # argparse failures can occur before Namespace exists; retain a known
        # public command without interpreting option values as subcommands.
        if command is None:
            arguments = sys.argv[1:] if argv is None else argv
            if arguments and arguments[0] in {"ports", "status", "capabilities", "capture"}:
                command = arguments[0]
        result, exit_code = error_result(command, exc), exc.exit_code
    except KeyboardInterrupt:
        result, exit_code = error_result(command, Failure(6, "interrupted", "Command interrupted")), 6
    if not result["ok"]:
        print(result["error"]["message"], file=sys.stderr)
    print(json.dumps(result, separators=(",", ":"), ensure_ascii=True))
    return exit_code
