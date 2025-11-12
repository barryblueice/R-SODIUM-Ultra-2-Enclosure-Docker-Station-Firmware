#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "esp_task_wdt.h"

#include "i2c_1/lvgl_ui/lvgl_init.h"
#include "init_func/init_func.h"

#define TAG "TMP117"

#define TMP117_TMP_REG      0x00
#define TMP117_RESOLUTION   0.0078125f

static esp_err_t TMP117_read_temp(i2c_master_dev_handle_t dev, float *temp) {
    uint8_t reg = TMP117_TMP_REG;
    uint8_t data[2];

    esp_err_t ret = i2c_master_transmit_receive(dev, &reg, 1, data, 2, pdMS_TO_TICKS(100));
    if (ret != ESP_OK) return ret;

    int16_t raw = (int16_t)((data[0] << 8) | data[1]);
    *temp = raw * TMP117_RESOLUTION;
    return ESP_OK;
}

void tmp117_thread(void *arg) {

    char buf[32];

    while (1) {
        float temp_front = 0, temp_back = 0;
        TMP117_read_temp(tmp117_front, &temp_front);
        TMP117_read_temp(tmp117_back, &temp_back);

        snprintf(buf, sizeof(buf), "F: %.1f°C,B: %.1f°C", temp_front, temp_back);
        update_label_text(0, buf);
        ESP_LOGI(TAG, "%s", buf);

        vTaskDelay(pdMS_TO_TICKS(1000));

    }
}

// void tmp117_thread(void *arg) {

//     while (1) {
//         float temp_front = 0, temp_back = 0;
//         TMP117_read_temp(tmp117_front, &temp_front);
//         TMP117_read_temp(tmp117_back, &temp_back);

//         char temp_show[32];
//         snprintf(temp_show, sizeof(temp_show), "F: %.3f°C; B: %.3f°C", temp_front, temp_back);
//         lv_label_set_text(labels[1], temp_show);

//         ESP_LOGI(TAG, temp_show);

//         esp_task_wdt_reset();
//         vTaskDelay(pdMS_TO_TICKS(500));

//     }
// }