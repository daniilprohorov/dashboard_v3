#include "profile.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

_Static_assert(CONFIG_DASH_CAN_BITRATE == 250000 || CONFIG_DASH_CAN_BITRATE == 500000,
               "DASH_CAN_BITRATE must be an ISO 15765-4 rate (250000 or 500000)");

static const char *const NVS_NAMESPACE = "dashboard";
static const char *const NVS_PROFILE_KEY = "obd_profile";

obd_profile_t profile_default(void)
{
    return CONFIG_DASH_CAN_BITRATE == 250000 ? OBD_PROFILE_11BIT_250K : OBD_PROFILE_11BIT_500K;
}

bool profile_valid(obd_profile_t profile)
{
    return (unsigned)profile <= OBD_PROFILE_29BIT_500K;
}

uint32_t profile_bitrate(obd_profile_t profile)
{
    switch (profile) {
    case OBD_PROFILE_11BIT_250K:
    case OBD_PROFILE_29BIT_250K:
        return 250000;
    case OBD_PROFILE_11BIT_500K:
    case OBD_PROFILE_29BIT_500K:
        return 500000;
    default:
        return 0;
    }
}

unsigned profile_id_bits(obd_profile_t profile)
{
    if (!profile_valid(profile)) {
        return 0;
    }
    return profile >= OBD_PROFILE_29BIT_250K ? 29 : 11;
}

const char *profile_name(obd_profile_t profile)
{
    switch (profile) {
    case OBD_PROFILE_11BIT_250K:
        return "ISO 15765-4 11-bit/250k";
    case OBD_PROFILE_11BIT_500K:
        return "ISO 15765-4 11-bit/500k";
    case OBD_PROFILE_29BIT_250K:
        return "ISO 15765-4 29-bit/250k";
    case OBD_PROFILE_29BIT_500K:
        return "ISO 15765-4 29-bit/500k";
    default:
        return "Invalid profile";
    }
}

esp_err_t profile_load(obd_profile_t *out)
{
    if (!out) {
        return ESP_ERR_INVALID_ARG;
    }
    *out = profile_default();
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        return err;
    }
    nvs_handle_t handle;
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return err;
    }
    uint8_t stored;
    err = nvs_get_u8(handle, NVS_PROFILE_KEY, &stored);
    nvs_close(handle);
    if (err != ESP_OK) {
        return err;
    }
    if (!profile_valid((obd_profile_t)stored)) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    *out = (obd_profile_t)stored;
    return ESP_OK;
}

esp_err_t profile_save(obd_profile_t profile)
{
    if (!profile_valid(profile)) {
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        return err;
    }
    nvs_handle_t handle;
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_set_u8(handle, NVS_PROFILE_KEY, (uint8_t)profile);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}
