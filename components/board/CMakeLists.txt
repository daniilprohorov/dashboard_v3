idf_component_register(
    SRCS "board.c" "board_uart.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_i2c esp_driver_gpio esp_lcd esp_driver_twai
    PRIV_REQUIRES esp_driver_uart esp_timer
)
