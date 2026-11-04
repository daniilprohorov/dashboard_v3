"""POSIX serial peer for public CLI acceptance, not a CAN/firmware emulator.

Only the subprocess boundary, JSON/exit status, outgoing UART requests, and CSV
are observed. Deliberately emitted frames test the host consumer; they do not
prove firmware filtering, drop-new queues, CAN passivity, CH343, or UI behavior.
"""

import os
from pathlib import Path
import pty
import select
import subprocess
import sys
import time
import tty


PACKAGE_ROOT = Path(__file__).resolve().parents[1]
SID = "0123456789ABCDEF"
OTHER_SID = "FEDCBA9876543210"
FIRMWARE = "pty-acceptance-1"
CSV_COLUMNS = (
    "event,session_id,firmware_version,protocol_version,cli_version,host_utc,device_us,"
    "requested_bitrate,effective_bitrate,id_type,filter,listen_only,uart_baud,uart_data_bits,"
    "uart_parity,uart_stop_bits,seq,id_bits,can_id,rtr,dlc,data_hex,state,tx_seq,ack_seq,"
    "sw_drops,hw_overruns,hw_lost_frames,reason,source"
).split(",")
COUNTER_COLUMNS = ("seq", "tx_seq", "ack_seq", "sw_drops", "hw_overruns", "hw_lost_frames")
CAPABILITIES = {
    "min_bitrate": "1000", "max_bitrate": "1000000", "exact_only": "1",
    "id_types": "11,29,both", "max_ranges": "16", "uart_baud": "2000000",
    "data_bits": "8", "parity": "N", "stop_bits": "1", "listen_only": "1",
}


def cli(arguments):
    return subprocess.Popen(
        [sys.executable, "-m", "can_logger", *map(str, arguments)],
        cwd=PACKAGE_ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        text=True, start_new_session=True,
    )


def fields_text(fields):
    return " ".join(f"{key}={value}" for key, value in fields.items())


class SerialPeer:
    def __init__(self, case):
        self.case = case
        self.master, self.slave = pty.openpty()
        tty.setraw(self.slave)
        self.port = os.ttyname(self.slave)
        self.buffer = bytearray()
        self.processes = []
        self.requests = []
        self.ack_times = []
        self.config = {"bitrate": "500000", "requested_bitrate": "500000",
                       "ids": "11", "filter": "all", "listen_only": "1"}
        self.sid = SID
        self.seq = 0
        self.tx_seq = 0
        self.ack = 0
        self.sw_drops = 0
        self.hw_overruns = 0
        self.closed = False

    def close(self):
        for process in self.processes:
            if process.poll() is None:
                process.kill()
            process.communicate(timeout=5)
        if not self.closed:
            os.close(self.master)
        os.close(self.slave)

    def launch(self, arguments):
        process = cli(arguments)
        self.processes.append(process)
        return process

    def write(self, payload):
        data = payload.encode("ascii") if isinstance(payload, str) else payload
        while data:
            sent = os.write(self.master, data)
            data = data[sent:]

    def line(self, text):
        self.write(text + "\n")

    def response(self, request, fields=None, error=None):
        rid, _, _ = request
        tail = f"ERR code={error}" if error else "OK"
        if fields:
            tail += " " + fields_text(fields)
        self.line(f"@D1 R {rid} {tail}")

    def request(self, timeout=5):
        deadline = time.monotonic() + timeout
        while b"\n" not in self.buffer:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise AssertionError("CLI did not send the next UART request")
            readable, _, _ = select.select([self.master], [], [], remaining)
            if not readable:
                raise AssertionError("CLI did not send the next UART request")
            self.buffer.extend(os.read(self.master, 4096))
        raw, _, rest = self.buffer.partition(b"\n")
        self.buffer = bytearray(rest)
        tokens = raw.decode("ascii").rstrip("\r").split(" ")
        self.case.assertEqual(tokens[:2], ["@D1", "Q"])
        self.case.assertGreaterEqual(len(tokens), 4)
        rid = int(tokens[2])
        self.case.assertGreaterEqual(rid, 1)
        self.case.assertLessEqual(rid, 2147483647)
        request = (rid, tokens[3], dict(token.split("=", 1) for token in tokens[4:]))
        self.requests.append(request)
        return request

    def expect(self, command, fields=None):
        request = self.request()
        self.case.assertEqual(request[1:], (command, fields or {}))
        return request

    def hello(self, mode="logger_idle", fields=None, noise=None):
        request = self.expect("HELLO")
        if noise:
            self.write(noise)
        response = {"device": "dashboard_v3", "firmware": FIRMWARE,
                    "protocol": "1", "mode": mode, "device_us": "900"}
        response.update(fields or {})
        self.response(request, response)
        return request

    def begin(self, *, ids="11", filter_text="all", bitrate="500000", duration_ms="10",
              sid=SID, overrides=None, status=True):
        self.sid = sid
        self.config = {"bitrate": bitrate, "requested_bitrate": bitrate,
                       "ids": ids, "filter": filter_text, "listen_only": "1"}
        self.seq = self.tx_seq = self.ack = self.sw_drops = self.hw_overruns = 0
        self.ack_times = []
        expected = {"bitrate": bitrate, "ids": ids, "filter": filter_text}
        if duration_ms is not None:
            expected["duration_ms"] = duration_ms
        request = self.expect("START", expected)
        confirmed = {"sid": self.sid, "device_us": "1000", **self.config}
        confirmed.update(overrides or {})
        self.response(request, confirmed)
        if status:
            self.status()
        return request

    def common(self, *, reason="none", **changes):
        fields = {"mode": "capturing" if reason == "none" else "logger_idle",
                  "sid": self.sid, "device_us": "2000", **self.config,
                  "seq": str(self.seq), "tx_seq": str(self.tx_seq), "ack": str(self.ack),
                  "sw_drops": str(self.sw_drops), "hw_overruns": str(self.hw_overruns),
                  "hw_lost": "unknown", "tx": "1" if reason == "none" else "0",
                  "client": "1", "reason": reason}
        fields.update({key: str(value) for key, value in changes.items()})
        return fields

    def status(self, **changes):
        self.line("@D1 S " + fields_text(self.common(**changes)))

    def frame(self, *, seq, bits="11", can_id="001", us="1100", rtr="0", dlc="2", data="0001", sid=None):
        self.seq = self.tx_seq = seq
        self.line("@D1 F " + fields_text({
            "sid": sid or self.sid, "seq": seq, "us": us, "bits": bits,
            "id": can_id, "rtr": rtr, "dlc": dlc, "data": data,
        }))

    def end(self, reason="duration", **changes):
        self.line("@D1 E " + fields_text(self.common(reason=reason, **changes)))

    def service(self, request, *, respond=True):
        _, command, fields = request
        self.case.assertEqual(command, "ACK")
        self.case.assertEqual(set(fields), {"sid", "seq"})
        self.case.assertEqual(fields["sid"], self.sid)
        received = int(fields["seq"])
        self.case.assertLessEqual(self.ack, received)
        self.case.assertLessEqual(received, self.tx_seq)
        self.ack = received
        self.ack_times.append(time.monotonic())
        if respond:
            self.response(request)

    def finish(self, process, timeout=7, service=True, respond=True):
        deadline = time.monotonic() + timeout
        while True:
            alive = process.poll() is None
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise AssertionError("CLI failed to terminate within the acceptance deadline")
            if self.closed:
                if alive:
                    process.wait(timeout=remaining)
                break
            readable, _, _ = select.select(
                [self.master], [], [], min(0.05, remaining) if alive else 0,
            )
            if readable or b"\n" in self.buffer:
                request = self.request(timeout=remaining)
                if not service:
                    raise AssertionError(f"Unexpected request: {request!r}")
                self.service(request, respond=respond)
            elif not alive:
                break
        stdout, stderr = process.communicate(timeout=5)
        return process.returncode, stdout, stderr

    def disconnect(self):
        os.close(self.master)
        self.closed = True

    def assert_released(self):
        # A new public command must acquire the exact same port successfully.
        process = self.launch(["status", "--port", self.port])
        self.hello()
        request = self.expect("STATUS")
        self.response(request, self.common(reason="host_stop"))
        return self.finish(process, service=False)
