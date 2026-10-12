#pragma once

#include "esp_err.h"

// Brings up CAN and starts a task that polls OBD-II mode 01 PIDs and refreshes
// the dashboard. Call after dashboard_create(); takes the LVGL lock itself.
esp_err_t obd_start(void);
