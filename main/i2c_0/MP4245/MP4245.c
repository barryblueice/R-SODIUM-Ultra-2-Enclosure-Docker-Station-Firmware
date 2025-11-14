#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "init_func/init_func.h"
#include "i2c_1/lvgl_ui/lvgl_init.h"

#define TAG "MP4245"

#define MP4245_I2C_PORT       0
#define MP4245_I2C_SDA_GPIO   17
#define MP4245_I2C_SCL_GPIO   18
#define MP4245_I2C_TIMEOUT    1000

#define PMBUS_CMD_VOUT_MODE           0x20
#define PMBUS_CMD_READ_VIN            0x88      // Linear11
#define PMBUS_CMD_READ_VOUT           0x8B      // Linear16 (uses VOUT_MODE exponent)
#define PMBUS_CMD_READ_TEMPERATURE_1  0x8D      // Linear11
#define PMBUS_CMD_STATUS_INPUT        0x7C
#define PMBUS_CMD_STATUS_VOUT         0x7A
#define PMBUS_CMD_STATUS_WORD         0x79

#define MP4245_REG_MFR_MODE_CTRL      0xD0
#define PFMP_VM_BIT_MASK              (1 << 0)

static float pmbus_linear11_decode(uint16_t raw, int n_exp_override)
{
    int16_t mant = (int16_t)(raw & 0x07FF);
    int8_t  exp  = (int8_t)((raw >> 11) & 0x1F);

    if (mant & 0x0400) mant |= ~0x07FF;
    if (exp & 0x10) exp |= ~0x1F;

    if (n_exp_override != 99) exp = (int8_t)n_exp_override;

    return (float)mant * powf(2.0f, (float)exp);
}

static float pmbus_linear16_decode(uint16_t raw, int8_t exp)
{
    return (float)raw * powf(2.0f, (float)exp);
}

static esp_err_t mp4245_read_word(uint8_t cmd, uint16_t *out)
{
    uint8_t rx[2] = {0};
    esp_err_t err = i2c_master_transmit_receive(mp4245_dev, &cmd, 1, rx, 2, MP4245_I2C_TIMEOUT);
    if (err == ESP_OK) {
        *out = (uint16_t)((rx[1] << 8) | rx[0]); // LSB first
    }
    return err;
}

static esp_err_t mp4245_read_byte(uint8_t cmd, uint8_t *out)
{
    return i2c_master_transmit_receive(mp4245_dev, &cmd, 1, out, 1, MP4245_I2C_TIMEOUT);
}

static esp_err_t mp4245_write_byte(uint8_t reg, uint8_t value)
{
    uint8_t tx[2] = { reg, value };
    return i2c_master_transmit(mp4245_dev, tx, sizeof(tx), MP4245_I2C_TIMEOUT);
}

static esp_err_t mp4245_get_vout_exponent(int8_t *exp_out)
{
    uint8_t vout_mode = 0;
    esp_err_t err = mp4245_read_byte(PMBUS_CMD_VOUT_MODE, &vout_mode);
    if (err != ESP_OK) return err;

    int8_t exp = (int8_t)(vout_mode & 0x1F);
    if (exp & 0x10) exp |= ~0x1F;
    *exp_out = exp;
    return ESP_OK;
}

static esp_err_t mp4245_set_mode_pfmp_vm(bool enable_pfmp_vm)
{
    uint8_t mode_ctrl = 0;
    esp_err_t err = mp4245_read_byte(MP4245_REG_MFR_MODE_CTRL, &mode_ctrl);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Read MODE_CTRL failed: %s", esp_err_to_name(err));
        return err;
    }

    uint8_t new_mode = mode_ctrl;
    if (enable_pfmp_vm) new_mode |= PFMP_VM_BIT_MASK;
    else new_mode &= (uint8_t)(~PFMP_VM_BIT_MASK);

    if (new_mode != mode_ctrl) {
        err = mp4245_write_byte(MP4245_REG_MFR_MODE_CTRL, 0x00);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Write MODE_CTRL failed: %s", esp_err_to_name(err));
            return err;
        }
    }
    return ESP_OK;
}

#define MP4245_REG_MFR_CURRENT_LIMIT  0xD1

static esp_err_t mp4245_set_current_limit_max(void) {
    uint8_t curr_lim_val = 0x7F;
    esp_err_t err = mp4245_write_byte(MP4245_REG_MFR_CURRENT_LIMIT, curr_lim_val);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Write CURRENT_LIMIT failed: %s", esp_err_to_name(err));
        return err;
    }
    uint8_t verify = 0;
    err = mp4245_read_byte(MP4245_REG_MFR_CURRENT_LIMIT, &verify);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "CURRENT_LIMIT set to 0x%02X (%.2f A)", 
                 verify, verify * 0.05f);
    }
    return err;
}

static esp_err_t mp4245_init_settings(void) {
    bool enable_pfmp_vm = true;
    esp_err_t err = mp4245_set_mode_pfmp_vm(enable_pfmp_vm);
    if (err != ESP_OK) return err;

    return mp4245_set_current_limit_max();
}


static void mp4245_read_all_status(void) {
    esp_err_t err;
    uint16_t raw;
    float vin, vout, temp;
    char buf[32];

    int8_t vout_exp = 0;
    err = mp4245_get_vout_exponent(&vout_exp);
    if (err != ESP_OK) ESP_LOGE(TAG, "Read VOUT_MODE failed");

    // VIN
    // err = mp4245_read_word(PMBUS_CMD_READ_VIN, &raw);
    // if (err == ESP_OK) {
    //     vin = pmbus_linear11_decode(raw, 99);
    //     ESP_LOGI(TAG, "VIN=%.4f V", vin);
    // }

    // VOUT
    // err = mp4245_read_word(PMBUS_CMD_READ_VOUT, &raw);
    // if (err == ESP_OK) {
    //     vout = pmbus_linear16_decode(raw, vout_exp);
    //     ESP_LOGI(TAG, "VOUT=%.4f V", vout);
    // }

    // TEMP
    // err = mp4245_read_word(PMBUS_CMD_READ_TEMPERATURE_1, &raw);
    // if (err == ESP_OK) {
    //     temp = pmbus_linear11_decode(raw, 99);
    //     ESP_LOGI(TAG, "TEMP=%.2f °C", temp);
    // }

    mp4245_read_word(PMBUS_CMD_READ_VIN, &raw);
    vin = pmbus_linear11_decode(raw, 99);
    mp4245_read_word(PMBUS_CMD_READ_VOUT, &raw);
    vout = pmbus_linear16_decode(raw, vout_exp);
    snprintf(buf, sizeof(buf), "VIN=%.1fV,VOUT=%.1fV", vin, vout);
    update_label_text(2, buf);
    ESP_LOGI(TAG, "%s", buf);

    mp4245_read_word(PMBUS_CMD_READ_TEMPERATURE_1, &raw);
    temp = pmbus_linear11_decode(raw, 99);
    snprintf(buf, sizeof(buf), "MP4245 Temp: %.0f°C", temp);
    update_label_text(3, buf);
    ESP_LOGI(TAG, "%s", buf);
    
}

void mp4245_thread(void *arg) {

    ESP_LOGI(TAG, "Apply init settings (mode control)...");
    mp4245_init_settings();

    while (1) {
        mp4245_read_all_status();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}