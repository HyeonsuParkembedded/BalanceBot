#include "motor_control.h"
#include "../bsw/pwm_driver.h"
#ifndef NATIVE_BUILD
#include "esp_log.h"
#endif

#ifndef NATIVE_BUILD
static const char* MOTOR_TAG = "MOTOR_CONTROL";
#else
#define MOTOR_TAG "MOTOR_CONTROL"
#endif

esp_err_t motor_control_init(motor_control_t* motor,
                            gpio_num_t pin_a, gpio_num_t pin_b,
                            gpio_num_t enable_pin, ledc_channel_t enable_ch) {
    motor->motor_pin_a = pin_a;
    motor->motor_pin_b = pin_b;
    motor->enable_pin = enable_pin;
    motor->enable_channel = enable_ch;

    // Initialize PWM driver
    esp_err_t ret = pwm_driver_init();
    if (ret != ESP_OK) {
        return ret;
    }

#ifndef NATIVE_BUILD
    // Configure motor pins
    gpio_config_t motor_config = {
        .pin_bit_mask = (1ULL << pin_a) | (1ULL << pin_b),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ret = gpio_config(&motor_config);
    if (ret != ESP_OK) {
        ESP_LOGE(MOTOR_TAG, "Failed to configure motor GPIO");
        return ret;
    }
#endif

    // Initialize PWM channel
    ret = pwm_channel_init(enable_pin, enable_ch);
    if (ret != ESP_OK) {
        return ret;
    }

#ifndef NATIVE_BUILD
    ESP_LOGI(MOTOR_TAG, "Motor control initialized");
#endif
    return ESP_OK;
}

void motor_control_set_speed(motor_control_t* motor, int speed) {
    // Constrain speed
    if (speed > 255) speed = 255;
    if (speed < -255) speed = -255;

#ifndef NATIVE_BUILD
    if (speed > 0) {
        gpio_set_level(motor->motor_pin_a, 1);
        gpio_set_level(motor->motor_pin_b, 0);
    } else if (speed < 0) {
        gpio_set_level(motor->motor_pin_a, 0);
        gpio_set_level(motor->motor_pin_b, 1);
        speed = -speed;
    } else {
        gpio_set_level(motor->motor_pin_a, 0);
        gpio_set_level(motor->motor_pin_b, 0);
    }
#endif

    pwm_set_duty(motor->enable_channel, speed);
}

void motor_control_stop(motor_control_t* motor) {
    motor_control_set_speed(motor, 0);
}