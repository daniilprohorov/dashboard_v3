"""Public python-module/PTY acceptance for the documented host CLI contract.

Run after installing tools/can_logger into the chosen Python environment:
    python -m unittest discover -s tools/can_logger/tests -v

These are serial-peer observations, not evidence of firmware CAN filtering,
real UART baud/reset behavior, CAN loss accounting, passivity, or UI behavior.
The Darwin PTY fixture adapts only its unsupported custom-baud ioctl; production
serial configuration and real hardware errors are never replaced.
"""

import csv
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import signal
import tempfile
import unittest

from serial_peer import (
    CAPABILITIES, COUNTER_COLUMNS, CSV_COLUMNS, FIRMWARE, OTHER_SID, SID,
    SerialPeer, cli,
)


@unittest.skipUnless(os.name == "posix", "PTY acceptance requires macOS or Linux")
class CliAcceptance(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix="dashboard-cli-acceptance-")
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.output = self.root / "capture.csv"

    def peer(self):
        peer = SerialPeer(self)
        self.addCleanup(peer.close)
        return peer

    def arguments(self, peer, *, output=None, ids="11", bitrate="500000", duration="0.01", allow=()):
        args = ["capture", "--port", peer.port, "--bitrate", bitrate, "--ids", ids,
                "--output", str(output or self.output)]
        if duration is not None:
            args += ["--duration", duration]
        for item in allow:
            args += ["--allow", item]
        return args

    def result(self, completed, exit_code=0, code=None):
        actual, stdout, stderr = completed
        self.assertEqual(actual, exit_code, (stdout, stderr))
        self.assertEqual(len(stdout.splitlines()), 1, stdout)
        result = json.loads(stdout)
        self.assertIs(result["ok"], exit_code == 0)
        if code:
            self.assertEqual(result["error"]["code"], code)
            self.assertTrue(result["error"]["message"])
            self.assertTrue(stderr.strip(), "Human diagnostics belong on stderr")
        else:
            self.assertNotIn("error", result)
            self.assertEqual(stderr, "")
        return result

    def rows(self, output=None):
        with (output or self.output).open(newline="", encoding="utf-8") as stream:
            reader = csv.DictReader(stream)
            self.assertEqual(reader.fieldnames, CSV_COLUMNS)
            rows = list(reader)
        for row in rows:
            self.assertNotIn(None, row, "Every record must fit the constant schema")
            self.assertNotIn(None, row.values(), "No truncated record is acceptable on a normal exit")
            self.assertRegex(row["session_id"], r"^[0-9A-F]{16}$")
            self.assertRegex(row["host_utc"], r"^\d{4}-\d\d-\d\dT.*Z$")
            self.assertEqual(datetime.fromisoformat(row["host_utc"].replace("Z", "+00:00")).tzinfo,
                             timezone.utc)
        return rows

    def assert_local_end(self, result, *, reason=None, frame_count=0):
        self.assertEqual(result["source"], "cli")
        if reason:
            self.assertEqual(result["reason"], reason)
        self.assertEqual(result["counters"], dict.fromkeys(COUNTER_COLUMNS))
        rows = self.rows()
        self.assertEqual([row["event"] for row in rows[:2]], ["start", "config"])
        self.assertEqual(sum(row["event"] == "frame" for row in rows), frame_count)
        self.assertEqual(sum(row["event"] == "end" for row in rows), 1)
        end = rows[-1]
        self.assertEqual((end["event"], end["source"]), ("end", "cli"))
        self.assertTrue(all(end[key] == "" for key in COUNTER_COLUMNS))
        self.assertEqual(end["device_us"], "")
        return rows

    def start(self, peer, *, duration="0.01", duration_ms="10", ids="11", bitrate="500000",
              filter_text="all", allow=(), output=None, sid=SID):
        process = peer.launch(self.arguments(peer, duration=duration, ids=ids, bitrate=bitrate,
                                             allow=allow, output=output))
        peer.hello()
        peer.begin(ids=ids, bitrate=bitrate, filter_text=filter_text, duration_ms=duration_ms, sid=sid)
        return process

    def test_explicit_parameters_are_required_and_argument_errors_are_one_json(self):
        complete = ["capture", "--port", "/not-opened", "--bitrate", "500000",
                    "--ids", "11", "--output", str(self.output)]
        cases = [[], ["unknown"], ["status"], ["capabilities"], ["capture"]]
        for option in ("--port", "--bitrate", "--ids", "--output"):
            index = complete.index(option)
            cases.append(complete[:index] + complete[index + 2:])
        for arguments in cases:
            with self.subTest(arguments=arguments):
                process = cli(arguments)
                stdout, stderr = process.communicate(timeout=5)
                self.result((process.returncode, stdout, stderr), 2, "argument_error")
                self.assertFalse(self.output.exists())

    def test_invalid_user_configuration_is_rejected_before_opening_port_or_output(self):
        base = ["capture", "--port", "/not-opened", "--bitrate", "500000", "--ids", "11",
                "--output", str(self.output)]
        cases = []
        for option, values in {
            "--bitrate": ["999", "1000001", "500000.0", "-1000", "0x7A120", "５０００００"],
            "--ids": ["12", "Both", "CANFD"],
            "--port": [""], "--output": [""],
        }.items():
            for value in values:
                arguments = base.copy()
                arguments[arguments.index(option) + 1] = value
                cases.append(arguments)
        for value in ["0", "-1", "nan", "inf", "-inf", "nonsense", "18446744073709551.616"]:
            cases.append(base + ["--duration", value])
        for value in ["", "0x001", "001,002", "002-001", "800", "20000000", "1-", "-1", "GG", "000000001"]:
            cases.append(base + ["--allow", value])
        cases.append(base + [value for number in range(17) for value in ("--allow", f"{number * 2:X}")])
        for arguments in cases:
            with self.subTest(arguments=arguments):
                process = cli(arguments)
                stdout, stderr = process.communicate(timeout=5)
                self.result((process.returncode, stdout, stderr), 2, "argument_error")
                self.assertFalse(self.output.exists())

    def test_help_is_human_readable_and_does_not_touch_a_port(self):
        for arguments in (["--help"], ["capture", "--help"]):
            with self.subTest(arguments=arguments):
                process = cli(arguments)
                stdout, stderr = process.communicate(timeout=5)
                self.assertEqual(process.returncode, 0)
                self.assertIn("usage:", stdout)
                self.assertEqual(stderr, "")
                self.assertFalse(self.output.exists())

    def test_missing_port_has_transport_exit_code_and_no_csv(self):
        peer = self.peer()
        arguments = self.arguments(peer)
        arguments[arguments.index("--port") + 1] = str(self.root / "missing-serial-device")
        process = cli(arguments)
        stdout, stderr = process.communicate(timeout=5)
        self.result((process.returncode, stdout, stderr), 3, "port_error")
        self.assertFalse(self.output.exists())

    def test_queries_report_all_modes_without_sending_start_or_stop(self):
        for mode in ("dashboard", "logger_idle", "capturing"):
            with self.subTest(mode=mode):
                peer = self.peer()
                process = peer.launch(["status", "--port", peer.port])
                peer.hello(mode=mode)
                request = peer.expect("STATUS")
                if mode == "capturing":
                    fields = peer.common()
                else:
                    fields = peer.common(mode=mode, sid="none", bitrate="0", requested_bitrate="0",
                                         ids="none", filter="none", tx="0", client="0")
                peer.response(request, fields)
                result = self.result(peer.finish(process, service=False))
                self.assertEqual(result["status"], fields)
                self.assertEqual(result["device"], "dashboard_v3")
                self.assertEqual(result["firmware_version"], FIRMWARE)
                self.assertEqual(result["protocol_version"], 1)
                self.assertEqual([request[1] for request in peer.requests], ["HELLO", "STATUS"])
                self.assertFalse(self.output.exists())

    def test_capabilities_are_reported_without_changing_mode(self):
        peer = self.peer()
        process = peer.launch(["capabilities", "--port", peer.port])
        peer.hello(mode="dashboard")
        peer.response(peer.expect("CAPS"), CAPABILITIES)
        result = self.result(peer.finish(process, service=False))
        self.assertEqual(result["capabilities"], CAPABILITIES)
        self.assertEqual([request[1] for request in peer.requests], ["HELLO", "CAPS"])

    def test_boot_noise_is_allowed_only_before_valid_hello(self):
        peer = self.peer()
        process = peer.launch(self.arguments(peer))
        peer.hello(noise=b"ESP-ROM boot\r\n\xff\xfe power-on\n" + b"x" * 1100 + b"\n")
        peer.begin()
        peer.end()
        self.result(peer.finish(process))
        self.assertEqual(self.rows()[-1]["reason"], "duration")

    def test_protocol_prefixed_damage_is_not_boot_noise(self):
        for payload, code in [(b"@D1 broken\n", "protocol_error"),
                              (b"@D2 R 1 OK\n", "unsupported_version"),
                              (b"@D1 R 1 OK device=dashboard_v3 device=other\n", "protocol_error")]:
            with self.subTest(payload=payload):
                peer = self.peer()
                process = peer.launch(self.arguments(peer))
                peer.expect("HELLO")
                peer.write(payload)
                self.result(peer.finish(process, service=False), 4, code)
                self.assertFalse(self.output.exists())

    def test_noise_after_hello_in_same_uart_batch_is_not_discarded(self):
        peer = self.peer()
        process = peer.launch(self.arguments(peer))
        request = peer.expect("HELLO")
        peer.write(f"@D1 R {request[0]} OK device=dashboard_v3 firmware={FIRMWARE} "
                   "protocol=1 mode=logger_idle device_us=900\nESP_LOG after HELLO\n")
        peer.expect("START", {"bitrate": "500000", "ids": "11", "filter": "all", "duration_ms": "10"})
        self.result(peer.finish(process, service=False), 4, "protocol_error")
        self.assertEqual(self.rows(), [])

    def test_hello_identity_and_version_are_validated(self):
        for fields, code in [({"protocol": "2"}, "unsupported_version"),
                             ({"device": "ecu_simulator"}, "protocol_error"),
                             ({"mode": "settings"}, "protocol_error")]:
            with self.subTest(fields=fields):
                peer = self.peer()
                process = peer.launch(self.arguments(peer))
                peer.hello(fields=fields)
                self.result(peer.finish(process, service=False), 4, code)
                self.assertFalse(self.output.exists())

    def test_start_rejections_preserve_exact_machine_codes_without_session_success(self):
        cases = [("dashboard", "logger_required", 5), ("capturing", "busy", 5),
                 ("logger_idle", "invalid_bitrate", 5), ("logger_idle", "invalid_argument", 5),
                 ("logger_idle", "invalid_command", 5), ("logger_idle", "hardware_error", 5),
                 ("logger_idle", "unsupported_version", 4)]
        for index, (mode, code, exit_code) in enumerate(cases):
            with self.subTest(mode=mode, code=code):
                output = self.root / f"rejected-{index}.csv"
                peer = self.peer()
                bitrate = "123457" if code == "invalid_bitrate" else "500000"
                process = peer.launch(self.arguments(peer, output=output, bitrate=bitrate))
                peer.hello(mode=mode)
                request = peer.expect("START", {"bitrate": bitrate, "ids": "11", "filter": "all", "duration_ms": "10"})
                peer.response(request, error=code)
                result = self.result(peer.finish(process, service=False), exit_code, code)
                self.assertIsNone(result["session_id"])
                self.assertIsNone(result["config"])
                self.assertEqual(result["counters"], dict.fromkeys(COUNTER_COLUMNS))
                self.assertEqual(self.rows(output), [])
                self.assertEqual([request[1] for request in peer.requests], ["HELLO", "START"])

    def test_start_must_confirm_the_exact_requested_configuration(self):
        cases = [{"bitrate": "250000", "requested_bitrate": "250000"},
                 {"bitrate": "499999"}, {"requested_bitrate": "499999"},
                 {"ids": "29"}, {"filter": "001"}, {"listen_only": "0"},
                 {"filter": "1-2"}, {"sid": "0001"}]
        for index, overrides in enumerate(cases):
            with self.subTest(overrides=overrides):
                output = self.root / f"wrong-config-{index}.csv"
                peer = self.peer()
                process = peer.launch(self.arguments(peer, output=output))
                peer.hello()
                peer.begin(overrides=overrides, status=False)
                result = self.result(peer.finish(process, service=False), 4, "protocol_error")
                self.assertIsNone(result["session_id"])
                self.assertIsNone(result["config"])
                self.assertEqual(self.rows(output), [])

    def test_all_id_modes_and_bitrate_boundaries_export_confirmed_metadata(self):
        for index, (ids, bitrate) in enumerate([("11", "1000"), ("29", "1000000"), ("both", "500000")]):
            with self.subTest(ids=ids, bitrate=bitrate):
                output = self.root / f"mode-{index}.csv"
                peer = self.peer()
                process = self.start(peer, ids=ids, bitrate=bitrate, output=output)
                peer.end()
                result = self.result(peer.finish(process))
                self.assertEqual(result["config"], {
                    "requested_bitrate": int(bitrate), "effective_bitrate": int(bitrate),
                    "id_type": ids, "filter": "all", "listen_only": True,
                    "uart_baud": 2000000, "uart_data_bits": 8, "uart_parity": "N", "uart_stop_bits": 1,
                })
                self.assertEqual(result["session_id"], SID)
                self.assertEqual(result["source"], "device")
                self.assertEqual(result["reason"], "duration")
                rows = self.rows(output)
                self.assertEqual([row["event"] for row in rows], ["start", "config", "status", "end"])
                self.assertEqual({row["session_id"] for row in rows}, {SID})
                self.assertEqual(rows[0]["device_us"], "1000")
                self.assertEqual(rows[0]["firmware_version"], FIRMWARE)
                self.assertEqual(rows[0]["protocol_version"], "1")
                self.assertEqual(rows[0]["cli_version"], result["cli_version"])
                self.assertEqual(rows[1]["requested_bitrate"], bitrate)
                self.assertEqual(rows[1]["effective_bitrate"], bitrate)
                self.assertEqual(rows[1]["id_type"], ids)
                self.assertEqual([rows[1][key] for key in ("filter", "listen_only", "uart_baud", "uart_data_bits", "uart_parity", "uart_stop_bits")],
                                 ["all", "1", "2000000", "8", "N", "1"])
                self.assertEqual(rows[-1]["hw_lost_frames"], "unknown")

    def test_allowlist_normalization_and_inclusive_boundaries_preserve_emitted_formats(self):
        # Literal peer frames are NOT evidence of firmware-side filtering.
        peer = self.peer()
        process = self.start(peer, ids="both", allow=["20-22", "1-3", "003-005", "6", "8", "7", "18db33f1"],
                             filter_text="00000001-00000008,00000020-00000022,18DB33F1")
        frames = [
            ("11", "001", "0", "2", "0001"),
            ("29", "00000001", "0", "8", "0001020304050000"),
            ("11", "008", "1", "8", "-"),
            ("29", "00000008", "0", "0", "-"),
            ("11", "020", "0", "1", "00"),
            ("29", "00000022", "1", "3", "-"),
            ("29", "18DB33F1", "0", "2", "00FF"),
        ]
        for seq, (bits, can_id, rtr, dlc, data) in enumerate(frames, 1):
            peer.frame(seq=seq, bits=bits, can_id=can_id, rtr=rtr, dlc=dlc, data=data, us=str(1100 + seq))
        peer.end()
        result = self.result(peer.finish(process))
        self.assertEqual(result["config"]["filter"], "00000001-00000008,00000020-00000022,18DB33F1")
        rows = [row for row in self.rows() if row["event"] == "frame"]
        self.assertEqual([(row["id_bits"], row["can_id"], row["rtr"], row["dlc"], row["data_hex"]) for row in rows], frames)
        self.assertEqual([row["seq"] for row in rows], ["1", "2", "3", "4", "5", "6", "7"])
        self.assertEqual([row["device_us"] for row in rows], ["1101", "1102", "1103", "1104", "1105", "1106", "1107"])
        self.assertEqual(result["counters"]["sw_drops"], 0)

    def test_single_id_and_11bit_overlapping_adjacent_ranges_are_canonical(self):
        for index, (allow, normalized, can_ids) in enumerate([
            (["00a"], "00A", ["00A"]),
            (["20-22", "1-3", "2-5", "6", "7ff"], "001-006,020-022,7FF", ["001", "006", "020", "022", "7FF"]),
        ]):
            with self.subTest(allow=allow):
                output = self.root / f"allow-{index}.csv"
                peer = self.peer()
                process = self.start(peer, allow=allow, filter_text=normalized, output=output)
                for seq, can_id in enumerate(can_ids, 1):
                    peer.frame(seq=seq, can_id=can_id)
                peer.end()
                self.result(peer.finish(process))
                self.assertEqual([row["can_id"] for row in self.rows(output) if row["event"] == "frame"], can_ids)

    def test_allowlist_limit_applies_after_normalizing_not_to_raw_argument_count(self):
        cases = [
            ([f"{number:X}" for number in range(17)], "000-010"),
            ([f"{number * 2:X}" for number in range(16)],
             "000,002,004,006,008,00A,00C,00E,010,012,014,016,018,01A,01C,01E"),
        ]
        for index, (allow, normalized) in enumerate(cases):
            with self.subTest(allow=allow):
                output = self.root / f"range-limit-{index}.csv"
                peer = self.peer()
                process = self.start(peer, allow=allow, filter_text=normalized, output=output)
                peer.end()
                result = self.result(peer.finish(process))
                self.assertEqual(result["config"]["filter"], normalized)

    def test_all_classic_dlc_values_and_rtr_lengths_are_preserved_independently(self):
        peer = self.peer()
        process = self.start(peer, ids="both")
        payloads = ["-", "00", "0001", "000102", "00010203", "0001020304",
                    "000102030405", "00010203040506", "0001020304050607"]
        expected = []
        sequence = 0
        for rtr, bits, can_id in [("0", "11", "7FF"), ("1", "29", "1FFFFFFF")]:
            for dlc, payload in enumerate(payloads):
                sequence += 1
                data = "-" if rtr == "1" else payload
                peer.frame(seq=sequence, bits=bits, can_id=can_id, rtr=rtr, dlc=str(dlc), data=data)
                expected.append((bits, can_id, rtr, str(dlc), data))
        peer.end()
        self.result(peer.finish(process))
        rows = [row for row in self.rows() if row["event"] == "frame"]
        self.assertEqual([(row["id_bits"], row["can_id"], row["rtr"], row["dlc"], row["data_hex"]) for row in rows],
                         expected)

    def test_host_rejects_frames_outside_inclusive_allowlist_or_selected_format(self):
        cases = [("11", "001-003", "11", "000"), ("11", "001-003", "11", "004"),
                 ("11", "001-003", "29", "00000002"),
                 ("29", "00000001-00000003", "11", "002"),
                 ("both", "00000001-00000003", "29", "00000004")]
        for index, (ids, filter_text, bits, can_id) in enumerate(cases):
            with self.subTest(ids=ids, bits=bits, can_id=can_id):
                output = self.root / f"out-of-filter-{index}.csv"
                peer = self.peer()
                process = self.start(peer, ids=ids, filter_text=filter_text, allow=["1-3"], output=output)
                peer.frame(seq=1, bits=bits, can_id=can_id)
                result = self.result(peer.finish(process), 4, "protocol_error")
                self.assertEqual(result["source"], "cli")
                self.assertEqual([row["event"] for row in self.rows(output)], ["start", "config", "status", "end"])

    def test_device_duration_is_rounded_up_and_sent_not_assumed_locally(self):
        for index, (duration, milliseconds) in enumerate([("0.0001", "1"), ("1.0001", "1001"), ("2", "2000")]):
            with self.subTest(duration=duration):
                output = self.root / f"duration-{index}.csv"
                peer = self.peer()
                process = self.start(peer, duration=duration, duration_ms=milliseconds, output=output)
                peer.end()
                result = self.result(peer.finish(process))
                self.assertEqual((result["reason"], result["source"]), ("duration", "device"))
                self.assertEqual(self.rows(output)[-1]["source"], "device")

    def test_quiet_capture_acks_zero_at_most_one_second_apart(self):
        peer = self.peer()
        process = self.start(peer, duration="30", duration_ms="30000")
        for _ in range(3):
            request = peer.request(timeout=1.2)
            peer.service(request)
            self.assertEqual(request[2]["seq"], "0")
        self.assertEqual(len({request[0] for request in peer.requests}), len(peer.requests))
        self.assertTrue(all(0 < later - earlier <= 1.1 for earlier, later in zip(peer.ack_times, peer.ack_times[1:])))
        peer.end()
        self.result(peer.finish(process))
        self.assertFalse(any(row["event"] == "frame" for row in self.rows()))

    def test_async_frames_status_and_reordered_responses_remain_correlated(self):
        peer = self.peer()
        process = self.start(peer, duration="30", duration_ms="30000")
        first = peer.request()
        peer.service(first, respond=False)
        self.assertEqual(first[2]["seq"], "0")
        peer.frame(seq=1, can_id="001", us="1111")
        peer.frame(seq=4, can_id="002", us="1444")
        peer.seq = 6
        peer.sw_drops = 4
        peer.hw_overruns = 2
        peer.status()
        second = peer.request(timeout=1.2)
        peer.service(second, respond=False)
        self.assertEqual(second[2]["seq"], "4", "ACK must confirm received frames, never device seq=6")
        self.assertNotEqual(first[0], second[0])
        peer.response(second, peer.common())
        peer.response(first)
        peer.end()
        result = self.result(peer.finish(process))
        self.assertEqual(result["counters"], {"seq": 6, "tx_seq": 4, "ack_seq": 4,
                                             "sw_drops": 4, "hw_overruns": 2, "hw_lost_frames": "unknown"})
        rows = self.rows()
        self.assertEqual([row["seq"] for row in rows if row["event"] == "frame"], ["1", "4"])
        self.assertEqual([rows[-1][key] for key in COUNTER_COLUMNS], ["6", "4", "4", "4", "2", "unknown"])
        self.assertTrue(any(row["event"] == "status" and row["sw_drops"] == "4" and row["hw_overruns"] == "2" for row in rows))

    def test_ctrl_c_requests_stop_and_waits_for_device_end_among_frames(self):
        peer = self.peer()
        process = self.start(peer, duration=None, duration_ms=None)
        peer.service(peer.request())
        process.send_signal(signal.SIGINT)
        stop = peer.expect("STOP", {"sid": SID, "reason": "ctrl_c"})
        peer.frame(seq=1, can_id="007", data="0000")
        peer.response(stop)
        peer.end("ctrl_c")
        result = self.result(peer.finish(process))
        self.assertEqual((result["reason"], result["source"]), ("ctrl_c", "device"))
        self.assertEqual([row["event"] for row in self.rows()], ["start", "config", "status", "frame", "end"])
        self.result(peer.assert_released())
        with self.output.open("a", encoding="utf-8") as stream:
            stream.write("")

    def test_device_terminal_reasons_have_distinct_results_and_preserve_partial_csv(self):
        for index, (reason, exit_code) in enumerate([("host_stop", 0), ("user_exit", 7),
                                                   ("link_loss", 6), ("hardware_error", 5)]):
            with self.subTest(reason=reason):
                output = self.root / f"terminal-{index}.csv"
                peer = self.peer()
                process = self.start(peer, output=output)
                peer.frame(seq=1, can_id="000", data="0000")
                peer.end(reason, mode="dashboard" if reason == "user_exit" else "logger_idle")
                result = self.result(peer.finish(process), exit_code, reason if exit_code else None)
                self.assertEqual((result["source"], result["reason"]), ("device", reason))
                self.assertEqual(result["counters"]["seq"], 1)
                rows = self.rows(output)
                self.assertEqual([row["can_id"] for row in rows if row["event"] == "frame"], ["000"])
                self.assertEqual((rows[-1]["reason"], rows[-1]["source"]), (reason, "device"))
                self.assertEqual(rows[-1]["hw_lost_frames"], "unknown")
                self.result(peer.assert_released())

    def test_physical_peer_disconnect_is_local_incomplete_not_device_final_counts(self):
        peer = self.peer()
        process = self.start(peer, duration=None, duration_ms=None)
        peer.frame(seq=1)
        # Waiting for ACK(1) proves the complete frame reached the CLI before EOF.
        for _ in range(3):
            request = peer.request(timeout=1.2)
            peer.service(request)
            if request[2]["seq"] == "1":
                break
        else:
            self.fail("CLI never confirmed the received frame")
        peer.disconnect()
        result = self.result(peer.finish(process), 6, "link_loss")
        self.assert_local_end(result, reason="link_loss", frame_count=1)

    def test_silent_serial_link_times_out_without_inventing_final_counters(self):
        peer = self.peer()
        process = self.start(peer, duration=None, duration_ms=None)
        # Confirm the first ACK before going silent so the receive-silence
        # deadline precedes the next outstanding command's response deadline.
        peer.service(peer.request())
        # Keep the PTY open but stop all subsequent device responses.
        result = self.result(peer.finish(process, respond=False), 6, "link_loss")
        self.assert_local_end(result, reason="link_loss")
        self.assertEqual(sum(request[1] == "START" for request in peer.requests), 1)
        self.result(peer.assert_released())

    def test_unanswered_ack_is_response_timeout_while_valid_frames_continue(self):
        peer = self.peer()
        process = self.start(peer, duration=None, duration_ms=None)
        peer.frame(seq=1)
        # Withhold exactly one correlated response, not the stream. Subsequent
        # valid frames/responses keep last_rx fresh beyond the command deadline.
        peer.service(peer.request(), respond=False)
        for sequence in (2, 3, 4):
            request = peer.request(timeout=1.2)
            peer.frame(seq=sequence, us=str(1100 + sequence))
            peer.service(request)
        result = self.result(peer.finish(process), 6, "response_timeout")
        rows = self.assert_local_end(result, reason="response_timeout", frame_count=4)
        self.assertEqual(rows[-1]["reason"], "response_timeout")
        self.assertEqual([row["seq"] for row in rows if row["event"] == "frame"],
                         ["1", "2", "3", "4"])
        self.assertEqual(sum(request[1] == "START" for request in peer.requests), 1)
        self.result(peer.assert_released())

    def test_ctrl_c_without_device_end_is_local_stop_timeout(self):
        peer = self.peer()
        process = self.start(peer, duration=None, duration_ms=None)
        peer.service(peer.request())
        process.send_signal(signal.SIGINT)
        peer.response(peer.expect("STOP", {"sid": SID, "reason": "ctrl_c"}))
        # Answer service messages so silence is not mistaken for a STOP deadline.
        result = self.result(peer.finish(process), 6, "stop_timeout")
        self.assert_local_end(result, reason="stop_timeout")

    def test_missing_device_end_after_duration_is_local_not_success(self):
        peer = self.peer()
        process = self.start(peer)
        result = self.result(peer.finish(process), 6, "end_timeout")
        self.assert_local_end(result, reason="end_timeout")

    def test_second_ctrl_c_does_not_claim_device_confirmation(self):
        peer = self.peer()
        process = self.start(peer, duration=None, duration_ms=None)
        peer.service(peer.request())
        process.send_signal(signal.SIGINT)
        peer.response(peer.expect("STOP", {"sid": SID, "reason": "ctrl_c"}))
        process.send_signal(signal.SIGINT)
        result = self.result(peer.finish(process), 6, "interrupted")
        self.assert_local_end(result, reason="interrupted")

    def test_repeated_captures_have_independent_sessions_configuration_and_resources(self):
        peer = self.peer()
        first = self.start(peer)
        peer.frame(seq=1, can_id="001")
        peer.end()
        first_result = self.result(peer.finish(first))
        self.assertEqual(first_result["corrupt_lines"], 0)
        first_bytes = self.output.read_bytes()
        self.result(peer.assert_released())
        output = self.root / "next.csv"
        second = self.start(peer, ids="29", bitrate="250000", allow=["18db33f1"],
                            filter_text="18DB33F1", output=output, sid=OTHER_SID)
        peer.frame(seq=1, bits="29", can_id="18DB33F1", data="0000")
        peer.end()
        second_result = self.result(peer.finish(second))
        self.assertNotEqual(first_result["session_id"], second_result["session_id"])
        self.assertEqual(second_result["config"]["effective_bitrate"], 250000)
        self.assertEqual(second_result["counters"]["seq"], 1)
        self.assertEqual(self.output.read_bytes(), first_bytes)
        rows = self.rows(output)
        self.assertEqual({row["session_id"] for row in rows}, {OTHER_SID})
        self.assertEqual([row["seq"] for row in rows if row["event"] == "frame"], ["1"])
        self.assertEqual(sum(request[1] == "START" for request in peer.requests), 2)
        self.result(peer.assert_released())

    def test_output_creation_failure_never_sends_start_or_overwrites(self):
        for output in (self.output, self.root / "missing-directory" / "capture.csv"):
            with self.subTest(output=output):
                if output == self.output:
                    output.write_text("existing recording\n", encoding="utf-8")
                peer = self.peer()
                process = peer.launch(self.arguments(peer, output=output))
                peer.hello()
                self.result(peer.finish(process, service=False), 8, "output_error")
                self.assertEqual([request[1] for request in peer.requests], ["HELLO"])
                if output == self.output:
                    self.assertEqual(output.read_text(encoding="utf-8"), "existing recording\n")
                else:
                    self.assertFalse(output.exists())
                self.result(peer.assert_released())

    def test_other_active_protocol_version_fails_loudly_with_local_end(self):
        peer = self.peer()
        process = self.start(peer)
        peer.write(f"@D2 F sid={SID} seq=2 us=1100 bits=11 id=001 rtr=0 dlc=2 data=0001\n")
        result = self.result(peer.finish(process), 4, "unsupported_version")
        self.assert_local_end(result, reason="unsupported_version")

    def test_framed_protocol_violations_fail_loudly_with_local_end(self):
        base = f"@D1 F sid={SID} seq=1 us=1100 bits=11 id=001 rtr=0 dlc=2 data=0001"
        cases = [
            ("@D1 unknown\n", "protocol_error"),
            (base.replace("id=001", "id=1") + "\n", "protocol_error"),
            (base.replace("id=001", "id=00a") + "\n", "protocol_error"),
            (base.replace("id=001", "id=800") + "\n", "protocol_error"),
            (base.replace("bits=11", "bits=12") + "\n", "protocol_error"),
            (base.replace("rtr=0", "rtr=2") + "\n", "protocol_error"),
            (base.replace("rtr=0", "rtr=1") + "\n", "protocol_error"),
            (base.replace("dlc=2", "dlc=0") + "\n", "protocol_error"),
            (base.replace("dlc=2", "dlc=9") + "\n", "protocol_error"),
            (base.replace("data=0001", "data=01") + "\n", "protocol_error"),
            (base.replace("data=0001", "data=00ff") + "\n", "protocol_error"),
            (base.replace("seq=1", "seq=0") + "\n", "protocol_error"),
            (base.replace("seq=1", "seq=-1") + "\n", "protocol_error"),
            (base.replace("seq=1", "seq=18446744073709551616") + "\n", "protocol_error"),
            (base.replace(SID, OTHER_SID) + "\n", "protocol_error"),
            (base + " extra=1\n", "protocol_error"),
            (base.replace(" data=0001", "") + "\n", "protocol_error"),
        ]
        for index, (payload, code) in enumerate(cases):
            with self.subTest(payload=payload):
                self.output = self.root / f"malformed-{index}.csv"
                peer = self.peer()
                process = self.start(peer)
                peer.write(payload)
                result = self.result(peer.finish(process), 4, code)
                self.assert_local_end(result, reason=code)

    def test_torn_active_lines_are_resynchronized_and_counted(self):
        base = f"@D1 F sid={SID} seq=1 us=1100 bits=11 id=001 rtr=0 dlc=2 data=0001"
        cases = [
            b"ESP_LOG unexpected\n", b"\n", b"\xff\xfe\n", b"@D1 F \x00\n",
            (base + " seq=2\n").encode(), (base.replace(" seq=1", "  seq=1") + "\n").encode(),
            (base.replace("data=0001", "data=000=1") + "\n").encode(),
            b"@D1 F sid\n",
        ]
        for index, payload in enumerate(cases):
            with self.subTest(payload=payload):
                self.output = self.root / f"torn-{index}.csv"
                peer = self.peer()
                process = self.start(peer)
                peer.write(payload)
                peer.frame(seq=2)
                peer.end()
                result = self.result(peer.finish(process))
                self.assertEqual(result["corrupt_lines"], 1)
                self.assertEqual(result["reason"], "duration")
                self.assertEqual(result["source"], "device")
                rows = self.rows()
                self.assertEqual([row["seq"] for row in rows if row["event"] == "frame"], ["2"])
                self.assertEqual(rows[-1]["reason"], "duration")

    def test_frame_sequence_duplicates_regressions_and_timestamp_regressions_are_errors(self):
        for index, (seq, us) in enumerate([(1, "1200"), (0, "1200"), (2, "1099")]):
            with self.subTest(seq=seq, us=us):
                self.output = self.root / f"regression-{index}.csv"
                peer = self.peer()
                process = self.start(peer)
                peer.frame(seq=1, us="1100")
                peer.frame(seq=seq, us=us)
                result = self.result(peer.finish(process), 4, "protocol_error")
                self.assert_local_end(result, frame_count=1)

    def test_impossible_or_changed_device_status_is_not_reported_as_confirmed_final(self):
        cases = [
            {"sid": OTHER_SID}, {"bitrate": "250000", "requested_bitrate": "250000"},
            {"ids": "29"}, {"filter": "001"}, {"listen_only": "0"},
            {"seq": "0", "tx_seq": "1"}, {"ack": "1", "tx_seq": "0"},
            {"seq": "0", "sw_drops": "1"}, {"hw_lost": "2"},
            {"device_us": "999"}, {"reason": "made_up"}, {"client": "2"},
        ]
        for index, changes in enumerate(cases):
            with self.subTest(changes=changes):
                self.output = self.root / f"bad-status-{index}.csv"
                peer = self.peer()
                process = self.start(peer)
                peer.status(**changes)
                result = self.result(peer.finish(process), 4, "protocol_error")
                self.assert_local_end(result)

    def test_status_counters_cannot_regress_or_claim_unsent_acknowledgement(self):
        cases = [({"seq": 5, "tx_seq": 2, "sw_drops": 3, "hw_overruns": 2}, {"seq": 4}),
                 ({"seq": 5, "tx_seq": 2}, {"tx_seq": 1}),
                 ({"seq": 5, "tx_seq": 2, "sw_drops": 3}, {"sw_drops": 2}),
                 ({"seq": 5, "tx_seq": 2, "hw_overruns": 2}, {"hw_overruns": 1}),
                 ({"seq": 5, "tx_seq": 2}, {"ack": 1}),
                 ({"seq": 5, "tx_seq": 2, "device_us": 3000}, {"device_us": 2999})]
        for index, (first, changes) in enumerate(cases):
            with self.subTest(changes=changes):
                self.output = self.root / f"counter-regression-{index}.csv"
                peer = self.peer()
                process = self.start(peer)
                peer.status(**first)
                peer.status(**{**first, **changes})
                result = self.result(peer.finish(process), 4, "protocol_error")
                self.assert_local_end(result)

    def test_unsolicited_duplicate_or_stale_service_responses_fail_correlation(self):
        for index, kind in enumerate(("unsolicited", "duplicate", "stale", "rejected")):
            with self.subTest(kind=kind):
                self.output = self.root / f"correlation-{index}.csv"
                peer = self.peer()
                process = self.start(peer)
                request = peer.request()
                peer.service(request, respond=False)
                if kind == "unsolicited":
                    peer.line("@D1 R 2147483647 OK")
                elif kind == "duplicate":
                    peer.response(request)
                    peer.response(request)
                elif kind == "stale":
                    peer.response(request, peer.common(sid=OTHER_SID))
                else:
                    peer.response(request, error="invalid_session")
                exit_code, code = (5, "invalid_session") if kind == "rejected" else (4, "protocol_error")
                result = self.result(peer.finish(process), exit_code, code)
                self.assert_local_end(result)

    def test_ack_counter_cannot_regress_after_host_confirmation(self):
        peer = self.peer()
        process = self.start(peer, duration="30", duration_ms="30000")
        peer.frame(seq=1)
        for _ in range(3):
            request = peer.request(timeout=1.2)
            peer.service(request, respond=False)
            if request[2]["seq"] == "1":
                peer.response(request, peer.common())
                break
            peer.response(request)
        else:
            self.fail("CLI never confirmed the received frame")
        peer.status(ack="0")
        result = self.result(peer.finish(process), 4, "protocol_error")
        self.assert_local_end(result, frame_count=1)

    def test_frames_cannot_regress_behind_device_transmit_status(self):
        peer = self.peer()
        process = self.start(peer)
        peer.status(seq="3", tx_seq="3")
        peer.frame(seq=2)
        result = self.result(peer.finish(process), 4, "protocol_error")
        self.assert_local_end(result)

    def test_status_counters_cannot_precede_received_frames(self):
        peer = self.peer()
        process = self.start(peer)
        peer.frame(seq=1)
        peer.status(seq="0", tx_seq="0")
        result = self.result(peer.finish(process), 4, "protocol_error")
        self.assert_local_end(result, frame_count=1)

    def test_abrupt_process_death_leaves_readable_partial_csv_without_false_end(self):
        peer = self.peer()
        process = self.start(peer, duration=None, duration_ms=None)
        peer.frame(seq=1, data="0000")
        for _ in range(3):
            request = peer.request(timeout=1.2)
            peer.service(request)
            if request[2]["seq"] == "1":
                break
        else:
            self.fail("CLI never confirmed the received frame")
        process.kill()
        exit_code, stdout, _ = peer.finish(process)
        self.assertEqual(exit_code, -signal.SIGKILL)
        self.assertEqual(stdout, "")
        rows = self.rows()
        self.assertEqual([row["event"] for row in rows], ["start", "config", "status", "frame"])
        self.assertEqual(rows[-1]["data_hex"], "0000")
        self.assertFalse(any(row["event"] == "end" for row in rows))
        self.result(peer.assert_released())

    def test_device_final_duration_without_requested_timer_is_protocol_error(self):
        peer = self.peer()
        process = self.start(peer, duration=None, duration_ms=None)
        peer.end("duration")
        result = self.result(peer.finish(process), 4, "protocol_error")
        self.assert_local_end(result)

    def test_crlf_protocol_lines_and_fragmented_payload_keep_leading_zeroes(self):
        peer = self.peer()
        process = self.start(peer)
        request = peer.request()
        peer.service(request)
        peer.write(f"@D1 F sid={SID} seq=1 us=1234 bits=11 id=000 rtr=0 dlc=2 data=00")
        peer.write("00\r\n")
        peer.seq = peer.tx_seq = 1
        peer.end()
        self.result(peer.finish(process))
        rows = [row for row in self.rows() if row["event"] == "frame"]
        self.assertEqual([(row["can_id"], row["data_hex"], row["device_us"]) for row in rows], [("000", "0000", "1234")])


if __name__ == "__main__":
    unittest.main()
