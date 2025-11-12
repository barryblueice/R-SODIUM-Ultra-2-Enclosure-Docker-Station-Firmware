#include "driver/i2c_master.h"

#ifndef TMP117_H
#define TMP117_H

void tmp117_thread(void *arg);
esp_err_t TMP117_read_temp(i2c_master_dev_handle_t dev, float *temp);

#endif
