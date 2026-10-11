#!/usr/bin/env bash
# Vendor reference code (Waveshare examples, ~600MB) -> third_party/ (git-ignored). Read-only reference.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST="$ROOT/third_party/waveshare-ESP32-S3-Touch-LCD-7B"
if [ -d "$DEST/.git" ]; then git -C "$DEST" pull --ff-only; else
  git clone --depth 1 https://github.com/waveshareteam/ESP32-S3-Touch-LCD-7B "$DEST"; fi
