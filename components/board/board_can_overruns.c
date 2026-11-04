#include "board.h"

#include "esp_attr.h"
#include "esp_idf_version.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "hal/twai_hal.h"
#include "soc/soc_caps.h"

// This is an intentionally version-pinned private HAL dependency, NOT a
// general-purpose replacement driver. Audit both source paths below when
// upgrading; accepting an unreviewed HAL could silently misreport loss.
#if ESP_IDF_VERSION != ESP_IDF_VERSION_VAL(5, 5, 5)
#error "Audit board RX-overrun HAL hook before upgrading from ESP-IDF v5.5.5"
#endif
#ifndef CONFIG_IDF_TARGET_ESP32S3
#error "Board RX-overrun HAL hook supports ESP32-S3 only"
#endif
_Static_assert(SOC_TWAI_CONTROLLER_NUM == 1, "Overrun counter requires one TWAI controller");
_Static_assert(SOC_TWAI_SUPPORTS_RX_STATUS == 1, "Overrun hook requires per-buffer RX status");

// IDF v5.5.5 components/hal/twai_hal_v1.c:424-440:
// On S3 (SOC_TWAI_SUPPORTS_RX_STATUS), false is returned ONLY when STATUS_MS
// marks the current RX buffer as overrun. The HAL releases that buffer before
// returning false; all ordinary reads return true. No bus error is counted.
// components/esp_driver_twai/esp_twai_onchip.c:257-279 consumes this result but
// exposes no overrun event/statistic. Linker --wrap observes that exact path,
// without changing RX buffer handling, callback ordering, or driver state.
// Each false return is one detected overrun marker, not a lost-frame estimate.
extern bool __real_twai_hal_read_rx_fifo(twai_hal_context_t *hal_ctx, twai_hal_frame_t *rx_frame);

// Xtensa 64-bit C atomics need not be lock-free. A tiny ISR-safe spinlock makes
// increment/snapshot atomic without a library lock, allocation, or logging.
// Internal RAM + IRAM also support CONFIG_TWAI_ISR_CACHE_SAFE.
static DRAM_ATTR portMUX_TYPE s_overrun_lock = portMUX_INITIALIZER_UNLOCKED;
static DRAM_ATTR uint64_t s_overrun_events;

bool IRAM_ATTR __wrap_twai_hal_read_rx_fifo(twai_hal_context_t *hal_ctx, twai_hal_frame_t *rx_frame)
{
    const bool received = __real_twai_hal_read_rx_fifo(hal_ctx, rx_frame);
    if (!received) {
        portENTER_CRITICAL_ISR(&s_overrun_lock);
        ++s_overrun_events;
        portEXIT_CRITICAL_ISR(&s_overrun_lock);
    }
    return received;
}

esp_err_t board_can_get_overruns(uint64_t *out_events)
{
    if (!out_events) {
        return ESP_ERR_INVALID_ARG;
    }
    portENTER_CRITICAL(&s_overrun_lock);
    *out_events = s_overrun_events;
    portEXIT_CRITICAL(&s_overrun_lock);
    return ESP_OK;
}
