#ifndef INIT_FUNC_H
#define INIT_FUNC_H

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

esp_err_t i2c_master_init(void);

extern i2c_master_dev_handle_t tmp117_front;
extern i2c_master_dev_handle_t tmp117_back;
extern i2c_master_dev_handle_t mp4245_dev;
extern i2c_master_bus_handle_t I2C_BUS0;
extern i2c_master_bus_handle_t I2C_BUS1;

#endif
