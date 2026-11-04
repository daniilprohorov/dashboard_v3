idf_component_register(
    SRCS "main.c" "app_controller.c" "app_ui.c" "dashboard.c" "obd.c" "profile.c" "logger.c" "logger_protocol.c" "fonts/font_b612_100.c" "fonts/font_b612_40.c" "fonts/font_b612_24.c"
    INCLUDE_DIRS "."
    REQUIRES board nvs_flash esp_timer esp_app_format esp_driver_twai
)
