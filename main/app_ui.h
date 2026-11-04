#pragma once

// Build retained dashboard/settings/Logger pages and the snapshot refresh timer.
// Call under the LVGL lock; no lifecycle work happens in event callbacks.
void app_ui_create(void);
