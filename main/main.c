#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "i2c_0/TMP117/TMP117.h"

#include "i2c_1/lvgl_ui/lvgl_init.h"

#include "init_func/init_func.h"

#include "ws2812/ws2812.h"

static const char *TAG = "Controller Main Thread";

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing R-SODIUM Ultra Enclosure 2.0 Controller");
    ESP_ERROR_CHECK(i2c_master_init());
    sensor_queue = xQueueCreate(8, sizeof(sensor_text_t));
    assert(sensor_queue != NULL); 
    lv_init();
    lv_display_t *display = lv_display_create(LCD_H_RES, LCD_V_RES);
    xTaskCreate(show_ui, "LVGL_UI", 12*1024, display, 2, NULL);
    xTaskCreate(lvgl_update_task, "lvgl_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Initialize WS2812 (Process LED)");
    xTaskCreate(ws2812_thread, "WS2812_THREAD", 4*1024, 0, 2, NULL);

    ESP_LOGI(TAG, "Initialize TMP117 (Temperature Sensor)");
    xTaskCreate(tmp117_thread, "TMP117_THREAD", 4*1024, 0, 2, NULL);
}