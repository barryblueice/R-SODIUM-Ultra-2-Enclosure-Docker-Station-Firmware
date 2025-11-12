#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/mcpwm_prelude.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define TAG "FAN_CTRL"

#include "init_func/init_func.h"

#include "fan_ctl/pwm_ctl.h"

#include "i2c_1/lvgl_ui/lvgl_init.h"

#define FAN_PWM_GPIO     33
#define FAN_RPM_GPIO     34
#define MCPWM_CLK_HZ     1000000UL
#define FAN_PWM_FREQ_HZ  25000UL
#define PERIOD_TICKS     (MCPWM_CLK_HZ / FAN_PWM_FREQ_HZ)
#define PULSES_PER_REV   2
#define RPM_MEASURE_INTERVAL_MS 1000

#define FAN_MIN_DUTY     18.0f    // % 最小有效占空比
#define FAN_MAX_DUTY     100.0f  // % 最大占空比
#define FAN_START_TEMP   40.0f   // °C 起转温度
#define FAN_FULL_TEMP    70.0f   // °C 全速温度

#include "i2c_0/TMP117/TMP117.h"

static mcpwm_cmpr_handle_t comparator = NULL;
static volatile uint32_t pulse_count = 0;
static SemaphoreHandle_t rpm_mutex;

static void IRAM_ATTR rpm_isr_handler(void* arg) {
    pulse_count++;
}

static void fan_set_speed(float duty_percent) {
    if (duty_percent < 0.0f) duty_percent = 0.0f;
    if (duty_percent > 100.0f) duty_percent = 100.0f;

    uint32_t cmp = (uint32_t)(duty_percent / 100.0f * PERIOD_TICKS + 0.5f);
    if (cmp >= PERIOD_TICKS) cmp = PERIOD_TICKS - 1;

    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, cmp));
    ESP_LOGI(TAG, "Set duty: %.1f%% (cmp=%u)", duty_percent, cmp);
}

void fan_pwm_init(void) {
    mcpwm_timer_handle_t timer = NULL;
    mcpwm_oper_handle_t oper = NULL;
    mcpwm_gen_handle_t generator = NULL;

    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = MCPWM_CLK_HZ,
        .period_ticks = PERIOD_TICKS,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

    mcpwm_operator_config_t operator_config = { .group_id = 0 };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer));

    mcpwm_comparator_config_t comparator_config = { .flags.update_cmp_on_tez = true };
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &comparator_config, &comparator));

    mcpwm_generator_config_t generator_config = { .gen_gpio_num = FAN_PWM_GPIO };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &generator_config, &generator));

    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator,
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,
        MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));

    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator,
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,
        comparator, MCPWM_GEN_ACTION_LOW)));

    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));

    fan_set_speed(0.0f);
}

void fan_rpm_init(void) {
    rpm_mutex = xSemaphoreCreateMutex();

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << FAN_RPM_GPIO),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(FAN_RPM_GPIO, rpm_isr_handler, NULL);
}

void fan_rpm_task(void *arg) {
    char buf[32];
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(RPM_MEASURE_INTERVAL_MS));

        uint32_t count;
        xSemaphoreTake(rpm_mutex, portMAX_DELAY);
        count = pulse_count;
        pulse_count = 0;
        xSemaphoreGive(rpm_mutex);

        uint32_t rpm = (count * 60000) / (PULSES_PER_REV * RPM_MEASURE_INTERVAL_MS);
        snprintf(buf, sizeof(buf), "Fan RPM: %lu", rpm);
        update_label_text(1, buf);
        ESP_LOGI(TAG, "%s", buf);
    }
}

void fan_temp_task(void *arg) {
    while (1) {
        float t1 = 0, t2 = 0;
        TMP117_read_temp(tmp117_front, &t1);
        TMP117_read_temp(tmp117_back, &t2);

        float avg = (t1 + t2) / 2.0f;
        float duty = 0.0f;

        if (avg < FAN_START_TEMP) {
            duty = 0.0f;
        } else {
            duty = FAN_MIN_DUTY + (avg - FAN_START_TEMP) * (FAN_MAX_DUTY - FAN_MIN_DUTY) / (FAN_FULL_TEMP - FAN_START_TEMP);
            if (duty > FAN_MAX_DUTY) duty = FAN_MAX_DUTY;
        }

        fan_set_speed(duty);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
