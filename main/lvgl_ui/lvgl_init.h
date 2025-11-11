#ifndef LVGL_INIT_H
#define LVGL_INIT_H

#include <stdint.h>
#include "sys/lock.h"
#include "lvgl.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

#define I2C_BUS_PORT  0
#define LCD_PIXEL_CLOCK_HZ    (400 * 1000)
#define PIN_NUM_SDA           36
#define PIN_NUM_SCL           37
#define PIN_NUM_RST           -1
#define I2C_HW_ADDR           0x3D

#define LCD_H_RES              128
#define LCD_V_RES              64
#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8

#define LVGL_TICK_PERIOD_MS    5
#define LVGL_TASK_STACK_SIZE   (8 * 1024)
#define LVGL_TASK_PRIORITY     2
#define LVGL_PALETTE_SIZE      8
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 1000 / CONFIG_FREERTOS_HZ

extern uint8_t oled_buffer[LCD_H_RES * LCD_V_RES / 8];
extern _lock_t lvgl_api_lock;
extern lv_obj_t *live_label;

bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t io_panel, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void ssd1315_set_brightness(esp_lcd_panel_io_handle_t io, int level);
void show_ui(void *arg);
void increase_lvgl_tick(void *arg);
void lvgl_port_task(void *arg);

#endif
