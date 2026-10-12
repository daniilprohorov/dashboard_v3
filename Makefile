# Thin wrapper over scripts/idf (idf.py with pinned ESP-IDF env).
# Override port: make flash PORT=/dev/cu.wchusbserial1234

IDF  := scripts/idf
PORT ?= $(firstword $(wildcard /dev/cu.wchusbserial*))

.PHONY: all build flash monitor flash-monitor size clean fullclean reconfigure menuconfig setup check-port

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
