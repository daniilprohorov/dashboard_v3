"""One serial owner; no reset pulse, retries, or session resumption."""

import time
from collections import deque

import serial

from . import protocol as wire
from .protocol import Failure


RESPONSE_TIMEOUT = 3.0


class Connection:
    def __init__(self, port):
        self.serial = None
        self.buffer = bytearray()
        self.lines = deque()
        self.discard_noise = False
        self.established = False
        self.pending = {}
        self.next_rid = 1
        self.last_rx = time.monotonic()
        self.on_event = None
        # Configure an unopened object: Serial(port=...) would assert the default
        # DTR/RTS levels during open, triggering the board's download circuit.
        try:
            self.serial = serial.Serial(port=None, baudrate=2000000, bytesize=serial.EIGHTBITS,
                                        parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
                                        timeout=0.05, write_timeout=1.0, xonxoff=False,
                                        rtscts=False, dsrdtr=False, exclusive=True)
            self.serial.dtr = False
            self.serial.rts = False
            self.serial.port = port
            self.serial.open()
        except (serial.SerialException, OSError, ValueError) as exc:
            if self.serial is not None:
                try:
                    self.serial.close()
                except (serial.SerialException, OSError):
                    pass  # Preserve the original open failure.
            raise Failure(3, "port_error", f"Cannot open {port}: {exc}") from exc

    def close(self):
        if self.serial is not None:
            try:
                self.serial.close()
            except (serial.SerialException, OSError) as exc:
                raise Failure(3, "transport_error", f"Cannot close serial port: {exc}") from exc
            finally:
                self.serial = None

    def send(self, command, **fields):
        rid = self.next_rid
        self.next_rid += 1
        wire.require(rid <= 2147483647, "Request ID space exhausted")
        text = f"@D1 Q {rid} {command}"
        if fields:
            text += " " + " ".join(f"{key}={value}" for key, value in fields.items())
        data = (text + "\n").encode("ascii")
        wire.require(len(data) - 1 <= 1024, "Outgoing request exceeds protocol limit")
        # Register before writing: an interrupted write must never be retried.
        self.pending[rid] = (command, fields, time.monotonic())
        try:
            written = self.serial.write(data)
            if written != len(data):
                raise Failure(3, "transport_error", "Incomplete serial request write")
        except (serial.SerialException, OSError) as exc:
            raise Failure(3, "transport_error", f"Serial write failed: {exc}") from exc
        return rid

    def _line(self):
        if not self.lines:
            try:
                data = self.serial.read(min(max(self.serial.in_waiting, 1), 4096))
            except (serial.SerialException, OSError) as exc:
                raise Failure(3, "transport_error", f"Serial read failed: {exc}") from exc
            if not data:
                return None
            for byte in data:
                if byte == 10:
                    if not self.discard_noise:
                        self.lines.append(bytes(self.buffer))
                    self.buffer.clear()
                    self.discard_noise = False
                elif not self.discard_noise:
                    self.buffer.append(byte)
                    if len(self.buffer) > 1025 or (len(self.buffer) == 1025 and byte != 13):
                        # Defer rejection until this item reaches the parser:
                        # HELLO may precede it in this same serial read.
                        self.lines.append(bytes(self.buffer))
                        self.buffer.clear()
                        self.discard_noise = True
        if self.lines:
            return self.lines.popleft()
        return None

    def receive(self):
        raw = self._line()
        if raw is None:
            return None
        if not self.established and not raw.startswith(b"@D"):
            return None  # Boot noise is legal only before successful HELLO.
        message = wire.parse(raw)
        self.last_rx = time.monotonic()
        if message.kind == "R":
            wire.require(message.rid in self.pending, "Unsolicited/duplicate command response")
            command, arguments, _ = self.pending.pop(message.rid)
            if not message.ok:
                code = message.fields["code"]
                raise Failure(4 if code == "unsupported_version" else 5, code,
                              f"Device rejected {command}: {code}")
            if command == "HELLO":
                wire.hello(message.fields)
                self.established = True
            elif command == "CAPS":
                wire.capabilities(message.fields)
            elif command == "STATUS":
                wire.common(message.fields)
            elif command == "START":
                wire.start(message.fields)
                for key in wire.CONFIG_KEYS.split():
                    expected = "1" if key == "listen_only" else arguments.get(
                        "bitrate" if key == "requested_bitrate" else key)
                    wire.require(message.fields[key] == str(expected),
                                 f"START did not confirm requested {key}")
            elif message.fields:
                wire.common(message.fields)
                wire.require(message.fields["sid"] == arguments["sid"],
                             "Service response has unrelated session")
            if self.on_event and command in {"ACK", "PING", "STOP", "STATUS"} and message.fields:
                self.on_event(wire.Message("S", message.fields))
        elif self.on_event:
            self.on_event(message)
        return message

    def check_timeouts(self):
        now = time.monotonic()
        if any(now - sent >= RESPONSE_TIMEOUT for _, _, sent in self.pending.values()):
            raise Failure(3, "response_timeout", "Device did not answer a command within 3 seconds")

    def request(self, command, **fields):
        rid = self.send(command, **fields)
        while True:
            response = self.receive()
            if response is not None and response.kind == "R" and response.rid == rid:
                return response.fields
            self.check_timeouts()
