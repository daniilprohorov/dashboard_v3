# Thin wrapper over scripts/idf (idf.py with pinned ESP-IDF env).
# Override port: make flash PORT=/dev/cu.wchusbserial1234

IDF  := scripts/idf
PORT ?= $(firstword $(wildcard /dev/cu.wchusbserial*))
PYTHON ?= python3

.PHONY: all build flash monitor flash-monitor size clean fullclean reconfigure menuconfig setup check-port help logger-install logger-help logger-test

all: build

build:
	$(IDF) build

flash: check-port build
	$(IDF) -p $(PORT) flash

monitor: check-port
	$(IDF) -p $(PORT) monitor

flash-monitor: check-port build
	$(IDF) -p $(PORT) flash monitor

size:
	$(IDF) size

clean:
	$(IDF) clean

fullclean:
	$(IDF) fullclean

reconfigure:
	rm -f sdkconfig
	$(IDF) reconfigure

menuconfig:
	$(IDF) menuconfig

setup:
	scripts/setup.sh

check-port:
	@test -n "$(PORT)" || { echo "No /dev/cu.wchusbserial* found; pass PORT=..." >&2; exit 1; }

help:
	@printf '%s\n' \
	  'Firmware: make setup | build | flash PORT=... | monitor PORT=... | size' \
	  'Logger (activate a Python >=3.10 venv first; override PYTHON=...):' \
	  '  make logger-install  # install tools/can_logger and pyserial' \
	  '  make logger-help     # actual root/capture command help' \
	  '  make logger-test     # public CLI serial-peer acceptance, no hardware proof' \
	  'Docs: docs/logger.md, docs/logger-protocol.md, docs/logger-acceptance.md' \
	  'Close capture/monitor before flashing; UART has one owner.'

logger-install:
	$(PYTHON) -m pip install ./tools/can_logger

logger-help:
	$(PYTHON) -m can_logger --help
	$(PYTHON) -m can_logger capture --help

logger-test:
	$(PYTHON) -m unittest discover -s tools/can_logger/tests -v
