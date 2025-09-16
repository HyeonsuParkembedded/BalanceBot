#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#ifndef NATIVE_BUILD
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#else
typedef int esp_err_t;
typedef int gpio_num_t;
typedef int ledc_channel_t;
#define ESP_OK 0
#define ESP_FAIL -1
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    gpio_num_t motor_pin_a;
    gpio_num_t motor_pin_b;
    gpio_num_t enable_pin;
    ledc_channel_t enable_channel;
} motor_control_t;

esp_err_t motor_control_init(motor_control_t* motor,
                            gpio_num_t pin_a, gpio_num_t pin_b,
                            gpio_num_t enable_pin, ledc_channel_t enable_ch);
void motor_control_set_speed(motor_control_t* motor, int speed);
void motor_control_stop(motor_control_t* motor);

#ifdef __cplusplus
}
#endif

#endif // MOTOR_CONTROL_H