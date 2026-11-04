"""Test subprocess bootstrap: adapt Darwin's unsupported PTY baud ioctl only.

This directory is added to PYTHONPATH only by SerialPeer.launch, with the exact
PTY that fixture owns. It is not packaged or loaded by the production CLI.
"""

import os
import sys


if sys.platform == "darwin":
    import serial.serialposix

    _set_special_baudrate = serial.serialposix.PlatformSpecific._set_special_baudrate
    _fixture_port = os.environ.get("DASHBOARD_TEST_PTY", "")

    def _set_pty_baudrate(self, baudrate):
        name = os.ttyname(self.fd)
        if (name == _fixture_port and name.startswith("/dev/ttys") and
                name[len("/dev/ttys"):].isdigit()):
            if baudrate != 2000000:
                raise AssertionError("PTY acceptance requires the fixed 2M baud request")
            return
        return _set_special_baudrate(self, baudrate)

    serial.serialposix.PlatformSpecific._set_special_baudrate = _set_pty_baudrate
