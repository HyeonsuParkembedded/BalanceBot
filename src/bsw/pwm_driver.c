#include "pwm_driver.h"
#ifndef NATIVE_BUILD
#include "esp_log.h"
#endif

#ifndef NATIVE_BUILD
static const char* PWM_TAG = "PWM_DRIVER";
#else
#define PWM_TAG "PWM_DRIVER"
#endif

static bool pwm_timer_initialized = false;

esp_err_t pwm_driver_init(void) {
    if (pwm_timer_initialized) {
        return ESP_OK;
    }

#ifndef NATIVE_BUILD
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    esp_err_t ret = ledc_timer_config(&ledc_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(PWM_TAG, "Failed to configure LEDC timer");
        return ret;
    }
#endif

    pwm_timer_initialized = true;
    return ESP_OK;
}

esp_err_t pwm_channel_init(gpio_num_t gpio, ledc_channel_t channel) {
#ifndef NATIVE_BUILD
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = channel,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = gpio,
        .duty = 0,
        .hpoint = 0
    };
    esp_err_t ret = ledc_channel_config(&ledc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(PWM_TAG, "Failed to configure LEDC channel");
        return ret;
    }
#endif
    return ESP_OK;
}

esp_err_t pwm_set_duty(ledc_channel_t channel, uint32_t duty) {
#ifndef NATIVE_BUILD
    esp_err_t ret = ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty);
    if (ret != ESP_OK) {
        return ret;
    }
    return ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
#else
    return ESP_OK;
#endif
}