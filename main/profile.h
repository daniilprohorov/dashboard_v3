#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

// Stable NVS values: do not reorder. These are the only dashboard profiles.
typedef enum {
    OBD_PROFILE_11BIT_250K = 0,
    OBD_PROFILE_11BIT_500K = 1,
    OBD_PROFILE_29BIT_250K = 2,
    OBD_PROFILE_29BIT_500K = 3,
} obd_profile_t;

obd_profile_t profile_default(void); // 11-bit, CONFIG_DASH_CAN_BITRATE
bool profile_valid(obd_profile_t profile);
uint32_t profile_bitrate(obd_profile_t profile); // 0 for invalid profile
unsigned profile_id_bits(obd_profile_t profile); // 0 for invalid profile
const char *profile_name(obd_profile_t profile); // "Invalid profile" if invalid

// Initialize NVS without erasing it. Always set *out to the default first.
// ESP_OK means a saved profile was read; ESP_ERR_NVS_NOT_FOUND means no saved
// value/namespace yet (the default is usable). Other init/open/read errors are
// returned unchanged; an out-of-range stored value is ESP_ERR_INVALID_RESPONSE.
// The caller must distinguish a usable default from successfully loaded data.
esp_err_t profile_load(obd_profile_t *out);

// Persist only after successful hardware application. Check both set and commit;
// ESP_OK alone means saved. On failure the active profile may be unsaved, and a
// flash failure can leave persistence uncertain. Does not start/stop OBD/CAN.
// NVS init failures are returned, never "repaired" by erasing unrelated data.
esp_err_t profile_save(obd_profile_t profile);
