#ifndef WS2812_H
#define WS2812_H
#include "led_strip.h"

typedef enum {
    LED_MODE_RGB,
    LED_MODE_RED_BREATH
} led_mode_t;

extern volatile led_mode_t led_mode;
extern const float TEMP_THRESHOLD;
extern led_strip_handle_t strip; 

extern const float TEMP_THRESHOLD;

void ws2812_thread(void *arg);

#endif
