#pragma once

#include "lvgl.h"

// The only colors on the dashboard besides black/white. Dark enough that white text
// on top of them stays readable; every colored element uses these exact shades.
#define PALETTE_RED    lv_color_hex(0xC62828)
#define PALETTE_YELLOW lv_color_hex(0xB58900)
#define PALETTE_GREEN  lv_color_hex(0x2E7D32)
#define PALETTE_BLUE   lv_color_hex(0x1565C0)
