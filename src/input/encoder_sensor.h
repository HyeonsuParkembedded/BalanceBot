#ifndef ENCODER_SENSOR_H
#define ENCODER_SENSOR_H

#ifndef NATIVE_BUILD
#include "driver/gpio.h"
#include "esp_err.h"
#else
typedef int esp_err_t;
typedef int gpio_num_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define IRAM_ATTR
#endif

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    gpio_num_t encoder_pin_a;
    gpio_num_t encoder_pin_b;
    volatile int32_t encoder_count;
    volatile int last_encoded;
    int ppr; // pulses per revolution
    float wheel_diameter;
    uint32_t last_time;
    int32_t last_position;
    float current_speed;
} encoder_sensor_t;

esp_err_t encoder_sensor_init(encoder_sensor_t* encoder,
                             gpio_num_t pin_a, gpio_num_t pin_b,
                             int pulses_per_rev, float wheel_diam);
void encoder_sensor_reset(encoder_sensor_t* encoder);
int32_t encoder_sensor_get_position(encoder_sensor_t* encoder);
float encoder_sensor_get_distance(encoder_sensor_t* encoder);
float encoder_sensor_get_speed(encoder_sensor_t* encoder);
esp_err_t encoder_sensor_update_speed(encoder_sensor_t* encoder);

#ifdef __cplusplus
}
#endif

#endif // ENCODER_SENSOR_H