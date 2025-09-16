#ifndef MOTOR_ENCODER_H
#define MOTOR_ENCODER_H

#ifndef NATIVE_BUILD
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#else
// Native build - types defined in test file
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define IRAM_ATTR
#endif
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    gpio_num_t encoder_pin_a;
    gpio_num_t encoder_pin_b;
    gpio_num_t motor_pin_a;
    gpio_num_t motor_pin_b;
    gpio_num_t enable_pin;
    ledc_channel_t enable_channel;
    
    volatile int32_t encoder_count;
    volatile int last_encoded;
    int ppr; // pulses per revolution
    float wheel_diameter;
    uint32_t last_time;
    int32_t last_position;
    float current_speed;
} motor_encoder_t;

esp_err_t motor_encoder_init(motor_encoder_t* motor, 
                            gpio_num_t enc_a, gpio_num_t enc_b,
                            gpio_num_t mot_a, gpio_num_t mot_b,
                            gpio_num_t enable_pin, ledc_channel_t enable_ch,
                            int pulses_per_rev, float wheel_diam);

void motor_encoder_set_speed(motor_encoder_t* motor, int speed);
void motor_encoder_stop(motor_encoder_t* motor);
int32_t motor_encoder_get_position(motor_encoder_t* motor);
float motor_encoder_get_distance(motor_encoder_t* motor);
float motor_encoder_get_speed(motor_encoder_t* motor);
void motor_encoder_reset_position(motor_encoder_t* motor);
void motor_encoder_update_speed(motor_encoder_t* motor);

#ifdef __cplusplus
}
#endif

#endif