idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_driver_gpio led_strip esp_driver_mcpwm esp_driver_i2c
    REQUIRES driver esp_lcd
    )
