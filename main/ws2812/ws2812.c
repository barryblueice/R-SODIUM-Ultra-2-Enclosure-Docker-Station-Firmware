#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_strip.h"
#include "esp_timer.h"

#include "ws2812.h"

#define WS2812_PIN 3
#define LED_NUM    8
#define DELAY_MS   50

// static const char *TAG = "WS2812_THREAD";
const float TEMP_THRESHOLD = 50.0f;

volatile led_mode_t led_mode = LED_MODE_RGB;
led_strip_handle_t led_strip = NULL; 

static void hsv2rgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b) {
    float c = v * s;
    float x = c * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    float m = v - c;
    float r_, g_, b_;
    if (h < 60) { r_ = c; g_ = x; b_ = 0; }
    else if (h < 120) { r_ = x; g_ = c; b_ = 0; }
    else if (h < 180) { r_ = 0; g_ = c; b_ = x; }
    else if (h < 240) { r_ = 0; g_ = x; b_ = c; }
    else if (h < 300) { r_ = x; g_ = 0; b_ = c; }
    else { r_ = c; g_ = 0; b_ = x; }

    *r = (uint8_t)((r_ + m) * 255);
    *g = (uint8_t)((g_ + m) * 255);
    *b = (uint8_t)((b_ + m) * 255);
}

void ws2812_thread(void *arg) {
    led_strip_config_t strip_config = {
        .strip_gpio_num = WS2812_PIN,
        .max_leds = LED_NUM,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,  // 10MHz
        .mem_block_symbols = 64,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_ERROR_CHECK(led_strip_clear(led_strip));

    float hue = 0.0f;
    float breath_phase = 0.0f;
    led_mode_t current_mode = LED_MODE_RGB;

    while (1) {
        if (current_mode != led_mode) {
            for (int t = 10; t >= 0; t--) {
                for (int i = 0; i < LED_NUM; i++) {
                    uint8_t r = 0, g = 0, b = 0;
                    if (current_mode == LED_MODE_RGB) {
                        float h = fmodf(hue + (360.0f / LED_NUM) * i, 360.0f);
                        hsv2rgb(h, 1.0f, 0.2f * t / 10.0f, &r, &g, &b);
                    } else {
                        r = (uint8_t)(255 * t / 10.0f);
                    }
                    led_strip_set_pixel(led_strip, i, r, g, b);
                }
                led_strip_refresh(led_strip);
                vTaskDelay(pdMS_TO_TICKS(50));
            }
            current_mode = led_mode;
        }

        if (current_mode == LED_MODE_RGB) {
            for (int i = 0; i < LED_NUM; i++) {
                float h = fmodf(hue + (360.0f / LED_NUM) * i, 360.0f);
                uint8_t r, g, b;
                hsv2rgb(h, 1.0f, 0.2f, &r, &g, &b);
                led_strip_set_pixel(led_strip, i, r, g, b);
            }
            hue += 3.0f;
            if (hue >= 360.0f) hue -= 360.0f;
        } else {
            float brightness = (sinf(breath_phase) + 1.0f) / 2.0f * 0.8f + 0.2f; // 0.2~1.0
            uint8_t val = (uint8_t)(255 * brightness);
            for (int i = 0; i < LED_NUM; i++) {
                led_strip_set_pixel(led_strip, i, val, 0, 0);
            }
            breath_phase += 0.1f;
            if (breath_phase >= 2 * M_PI) breath_phase -= 2 * M_PI;
        }

        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }
}