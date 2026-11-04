idf_component_register(
    SRCS "board.c" "board_uart.c" "board_can_overruns.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_i2c esp_driver_gpio esp_lcd esp_driver_twai
    PRIV_REQUIRES esp_driver_uart esp_timer hal
)

# IDF v5.5.5's TWAI driver drops RX-overrun observations before public callbacks.
# Keep the board-owned hook linked even when no application accessor is used.
target_link_options(${COMPONENT_LIB} INTERFACE
    "-Wl,--wrap=twai_hal_read_rx_fifo"
    "-Wl,--undefined=__wrap_twai_hal_read_rx_fifo"
)
