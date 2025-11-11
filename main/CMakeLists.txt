idf_component_register(
    SRCS
        "main.c"

        # Resources
        "res/logo.c"
        "res/fonts.c"

        # I2C 1 - LVGL UI
        "i2c_1/lvgl_ui/lvgl_init.c"
        "i2c_1/lvgl_ui/display.c"
        "i2c_1/lvgl_ui/lvgl_show.c"

        # WS2812 LED Control
        "ws2812/ws2812.c"

    INCLUDE_DIRS "."

    REQUIRES driver 
        esp_lcd 
        esp_lvgl_port 
        lvgl 
        esp_timer
        esp_driver_gpio 
        led_strip 
        esp_driver_mcpwm 
        esp_driver_i2c
    )
