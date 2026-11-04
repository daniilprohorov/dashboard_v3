#pragma once

#include "esp_err.h"
#include "profile.h"

// Call from one serialized app-controller task, after dashboard_create(), and
// never while holding the LVGL lock. A live/stopping instance rejects start
// with ESP_ERR_INVALID_STATE. Copies the profile; caller need not retain it.
// Success means hardware configured and worker started, not ECU presence.
// Clears measurements, PID support and errors before publishing "No ECU".
// Does not save NVS. Hardware/allocation/UI-lock errors are returned honestly.
// A start failure normally unwinds completely; a cleanup failure retains the
// instance and returns that error. Call stop successfully before handing CAN
// to another owner or attempting another start.
esp_err_t obd_start(const obd_profile_t *profile);

// Idempotent. Requests cooperative stop, waits at most 500 ms for the worker,
// then deletes the joined task, disables/deletes CAN, and finally frees RX.
// Successful return guarantees no worker updates or ISR callbacks remain.
// Timeout/teardown failure retains resources for a later stop retry; NEVER
// hand CAN to another owner on failure. No force-delete of a running worker.
// Worker UI-lock waits and CAN requests are finite, so no LVGL join deadlock.
esp_err_t obd_stop(void);
