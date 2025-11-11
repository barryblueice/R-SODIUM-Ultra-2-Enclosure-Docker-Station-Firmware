idf_component_register(
    SRCS
        "lvgl_demo_ui.c" 
        "main.c"
        "res/logo.c"
        "res/fonts.c"
        "lvgl_ui/lvgl_init.c"
        "lvgl_ui/display.c"
        "ws2812/ws2812.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_driver_gpio 
        led_strip 
        esp_driver_mcpwm 
        esp_driver_i2c
    REQUIRES driver 
        esp_lcd 
        esp_lvgl_port 
        lvgl esp_timer
    )
