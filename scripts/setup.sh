#!/usr/bin/env bash
# One-time toolchain install (macOS/Homebrew). Idempotent.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
command -v brew >/dev/null && brew install cmake ninja ccache dfu-util python@3.12 poppler
source "$ROOT/scripts/env.sh" # after brew: puts python@3.12 first on PATH

if [ ! -d "$IDF_PATH" ]; then
  mkdir -p "$(dirname "$IDF_PATH")"
  git clone -b "$IDF_VERSION" --depth 1 --recursive --shallow-submodules -j8 \
    https://github.com/espressif/esp-idf "$IDF_PATH"
fi
"$IDF_PATH/install.sh" esp32s3
"$ROOT/scripts/fetch-vendor.sh"
