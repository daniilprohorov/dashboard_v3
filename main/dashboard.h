#pragma once

#include <stdbool.h>
#include <stdint.h>

// Snapshot of everything the dashboard shows. Fractional values are in tenths.
typedef struct {
    uint32_t rpm;
    int8_t gear;           // 0 = N, -1 = R, 1..n
    uint32_t speed;        // km/h
    int16_t wtmp;          // °C
    int16_t otmp;          // °C
    uint8_t olvl;          // %
    uint16_t fcns_x10;     // l/100 km × 10
    uint32_t range;        // km
    uint16_t flvl_x10;     // l × 10
    int16_t etmp;          // °C
    uint32_t odometer;     // km
    const char *errors;    // NULL or "" = no errors
    uint8_t hour, minute;
} dash_data_t;

// Builds the screen on the active display. Call with the LVGL lock held.
void dashboard_create(void);

// Refreshes all fields. Call with the LVGL lock held (or from the LVGL task).
void dashboard_update(const dash_data_t *d);

// Starts an LVGL timer that sweeps all fields up to their maxima and back.
void dashboard_demo_start(void);
