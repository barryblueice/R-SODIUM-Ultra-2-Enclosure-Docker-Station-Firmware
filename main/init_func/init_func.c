#include "driver/i2c_master.h"

#define I2C_BUS0_SDA        17
#define I2C_BUS0_SCL        18
#define I2C_MASTER_FREQ_HZ  400000
#define TMP117_ADDR_FRONT   0x48
#define TMP117_ADDR_BACK    0x49
#define TMP117_TMP_REG      0x00
#define TMP117_RESOLUTION   0.0078125f
#define MP4245_I2C_ADDR       0x61
#define MP4245_I2C_FREQ       100000

#define I2C_BUS1_SDA           36
#define I2C_BUS1_SCL           37

#include "esp_log.h"

#include <stdio.h>

#include "init_func.h"

i2c_master_bus_handle_t I2C_BUS0 = NULL;
i2c_master_bus_handle_t I2C_BUS1 = NULL;
i2c_master_dev_handle_t tmp117_front = NULL;
i2c_master_dev_handle_t tmp117_back = NULL;
i2c_master_dev_handle_t mp4245_dev = NULL;

esp_err_t i2c_master_init(void) {
    i2c_master_bus_config_t bus_cfg = {
        .sda_io_num = I2C_BUS0_SDA,
        .scl_io_num = I2C_BUS0_SCL,
        .i2c_port = 0,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &I2C_BUS0));

    i2c_device_config_t tmp117_front_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TMP117_ADDR_FRONT,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(I2C_BUS0, &tmp117_front_cfg, &tmp117_front));

    i2c_device_config_t tmp117_back_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TMP117_ADDR_BACK,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(I2C_BUS0, &tmp117_back_cfg, &tmp117_back));

    i2c_device_config_t mp4245_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MP4245_I2C_ADDR,
        .scl_speed_hz = MP4245_I2C_FREQ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(I2C_BUS0, &mp4245_cfg, &mp4245_dev));

    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .i2c_port = 1,
        .sda_io_num = I2C_BUS1_SDA,
        .scl_io_num = I2C_BUS1_SCL,
        .flags.enable_internal_pullup = false,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &I2C_BUS1));

    return ESP_OK;
}