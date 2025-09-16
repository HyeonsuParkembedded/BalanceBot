#ifndef SERVO_STANDUP_H
#define SERVO_STANDUP_H

#ifndef NATIVE_BUILD
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#endif
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    STANDUP_IDLE,
    STANDUP_EXTENDING,
    STANDUP_PUSHING,
    STANDUP_RETRACTING,
    STANDUP_COMPLETE
} standup_state_t;

typedef struct {
    gpio_num_t servo_pin;
    ledc_channel_t servo_channel;
    int extended_angle;
    int retracted_angle;
    int current_angle;
    standup_state_t state;
    uint32_t state_start_time;
    uint32_t extend_duration;
    uint32_t push_duration;
    uint32_t retract_duration;
    bool standup_requested;
    bool standup_in_progress;
} servo_standup_t;

esp_err_t servo_standup_init(servo_standup_t* servo, gpio_num_t pin, ledc_channel_t channel, int extend_angle, int retract_angle);
void servo_standup_request_standup(servo_standup_t* servo);
void servo_standup_update(servo_standup_t* servo);
bool servo_standup_is_standing_up(servo_standup_t* servo);
bool servo_standup_is_complete(servo_standup_t* servo);
void servo_standup_reset(servo_standup_t* servo);
void servo_standup_set_timings(servo_standup_t* servo, uint32_t extend, uint32_t push, uint32_t retract);
void servo_standup_set_angles(servo_standup_t* servo, int extend, int retract);
standup_state_t servo_standup_get_state(servo_standup_t* servo);

#ifdef __cplusplus
}
#endif

#endif