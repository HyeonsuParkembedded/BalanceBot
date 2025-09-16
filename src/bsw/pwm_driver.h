#ifndef PWM_DRIVER_H
#define PWM_DRIVER_H

#ifndef NATIVE_BUILD
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"
#else
typedef int esp_err_t;
typedef int ledc_channel_t;
typedef int gpio_num_t;
#define ESP_OK 0
#define ESP_FAIL -1
#endif

#ifdef __cplusplus
extern "C" {
#endif

// PWM driver initialization
esp_err_t pwm_driver_init(void);
esp_err_t pwm_channel_init(gpio_num_t gpio, ledc_channel_t channel);
esp_err_t pwm_set_duty(ledc_channel_t channel, uint32_t duty);

#ifdef __cplusplus
}
#endif

#endif // PWM_DRIVER_H