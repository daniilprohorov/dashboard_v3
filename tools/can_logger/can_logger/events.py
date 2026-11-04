"""Constant event CSV schema; only END supplies final device counters."""

import csv
import time
from datetime import datetime, timezone

from . import __version__
from .protocol import Failure


COLUMNS = (
    "event,session_id,firmware_version,protocol_version,cli_version,host_utc,device_us,"
    "requested_bitrate,effective_bitrate,id_type,filter,listen_only,uart_baud,uart_data_bits,"
    "uart_parity,uart_stop_bits,seq,id_bits,can_id,rtr,dlc,data_hex,state,tx_seq,ack_seq,"
    "sw_drops,hw_overruns,hw_lost_frames,reason,source"
).split(",")


def utc_now():
    return datetime.now(timezone.utc).isoformat(timespec="microseconds").replace("+00:00", "Z")


def counters(fields=None):
    mapping = {"seq": "seq", "tx_seq": "tx_seq", "ack_seq": "ack", "sw_drops": "sw_drops",
               "hw_overruns": "hw_overruns", "hw_lost_frames": "hw_lost"}
    return {name: (None if fields is None else fields[key] if key == "hw_lost" else int(fields[key]))
            for name, key in mapping.items()}


def configuration(fields):
    return {"requested_bitrate": int(fields["requested_bitrate"]),
            "effective_bitrate": int(fields["bitrate"]), "id_type": fields["ids"],
            "filter": fields["filter"], "listen_only": True,
            "uart_baud": 2000000, "uart_data_bits": 8, "uart_parity": "N", "uart_stop_bits": 1}


class EventCSV:
    def __init__(self, path):
        self.file = None
        self.sid = None
        self.last_flush = time.monotonic()
        try:
            self.file = open(path, "x", encoding="utf-8", newline="")
            self.writer = csv.DictWriter(self.file, fieldnames=COLUMNS)
            self.writer.writeheader()
            self.flush(force=True)
        except (Failure, OSError, UnicodeError, ValueError) as exc:
            if self.file is not None:
                try:
                    self.file.close()
                except (OSError, UnicodeError):
                    pass  # Preserve the sink creation/flush failure.
                finally:
                    self.file = None
            if isinstance(exc, Failure):
                raise
            raise Failure(8, "output_error", f"Cannot create CSV {path}: {exc}") from exc

    def row(self, event, **fields):
        row = {"event": event, "session_id": self.sid, "host_utc": utc_now()}
        row.update(fields)
        try:
            self.writer.writerow(row)
            self.flush()
        except (OSError, UnicodeError) as exc:
            raise Failure(8, "output_error", f"Cannot write CSV: {exc}") from exc

    def flush(self, force=False):
        if self.file is None:
            return
        now = time.monotonic()
        if force or now - self.last_flush >= 0.25:
            try:
                self.file.flush()
            except (OSError, UnicodeError) as exc:
                raise Failure(8, "output_error", f"Cannot flush CSV: {exc}") from exc
            self.last_flush = now

    def start(self, fields, hello):
        self.sid = fields["sid"]
        self.row("start", firmware_version=hello["firmware"], protocol_version=1,
                 cli_version=__version__, device_us=fields["device_us"])
        settings = configuration(fields)
        settings["listen_only"] = 1
        self.row("config", **settings)
        self.flush(force=True)

    def frame(self, fields):
        self.row("frame", device_us=fields["us"], seq=fields["seq"], id_bits=fields["bits"],
                 can_id=fields["id"], rtr=fields["rtr"], dlc=fields["dlc"], data_hex=fields["data"])

    def status(self, fields):
        self.row("status", device_us=fields["device_us"], state=fields["mode"],
                 reason=fields["reason"], source="device", **counters(fields))

    def end(self, reason, fields=None):
        self.row("end", reason=reason, source="device" if fields is not None else "cli",
                 device_us=fields["device_us"] if fields is not None else "",
                 state=fields["mode"] if fields is not None else "", **counters(fields))
        self.flush(force=True)

    def close(self):
        if self.file is not None:
            try:
                self.file.close()
            except (OSError, UnicodeError) as exc:
                raise Failure(8, "output_error", f"Cannot close CSV: {exc}") from exc
            finally:
                self.file = None
