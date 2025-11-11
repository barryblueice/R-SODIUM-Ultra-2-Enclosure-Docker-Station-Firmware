#include <stdio.h>
#include "lvgl_ui/lvgl_init.h"
#include "esp_log.h"
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_strip.h"
#include "ws2812/ws2812.h"

static const char *TAG = "Controller Main Thread";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing R-SODIUM Ultra Enclosure 2.0 Controller");
    lv_init();
    lv_display_t *display = lv_display_create(LCD_H_RES, LCD_V_RES);
    xTaskCreate(show_ui, "LVGL_UI", 12*1024, display, 2, NULL);

    ESP_LOGI(TAG, "Initialize WS2812 (Process LED)");
    xTaskCreate(ws2812_thread, "WS2812_THREAD", 4*1024, display, 2, NULL);
}