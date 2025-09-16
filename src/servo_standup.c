#include "servo_standup.h"
#ifndef NATIVE_BUILD
#include "esp_log.h"
#endif

#ifndef NATIVE_BUILD
static const char* TAG = "SERVO_STANDUP";
#else
#define TAG "SERVO_STANDUP"
#endif

// Servo PWM constants
#define SERVO_MIN_PULSEWIDTH_US 500   // Minimum pulse width in microseconds
#define SERVO_MAX_PULSEWIDTH_US 2500  // Maximum pulse width in microseconds
#define SERVO_MAX_DEGREE        180   // Maximum angle in degrees
#define SERVO_FREQ              50    // Servo frequency in Hz

static uint32_t servo_per_degree_init(uint32_t degree_of_rotation) {
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH_US + (((SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
    return cal_pulsewidth;
}

static void servo_set_angle(servo_standup_t* servo, int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    
    uint32_t pulse_width = servo_per_degree_init(angle);
    uint32_t duty = (pulse_width * ((1 << LEDC_TIMER_14_BIT) - 1)) / (1000000 / SERVO_FREQ);
    
    ledc_set_duty(LEDC_LOW_SPEED_MODE, servo->servo_channel, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, servo->servo_channel);
    
    servo->current_angle = angle;
}

esp_err_t servo_standup_init(servo_standup_t* servo, gpio_num_t pin, ledc_channel_t channel, int extend_angle, int retract_angle) {
    servo->servo_pin = pin;
    servo->servo_channel = channel;
    servo->extended_angle = extend_angle;
    servo->retracted_angle = retract_angle;
    servo->current_angle = retract_angle;
    servo->state = STANDUP_IDLE;
    servo->state_start_time = 0;
    servo->extend_duration = 1000;   // 1 second to extend
    servo->push_duration = 2000;     // 2 seconds to push/hold
    servo->retract_duration = 1000;  // 1 second to retract
    servo->standup_requested = false;
    servo->standup_in_progress = false;

    // Configure LEDC timer for servo
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_1,
        .duty_resolution = LEDC_TIMER_14_BIT,
        .freq_hz = SERVO_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    esp_err_t ret = ledc_timer_config(&ledc_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC timer");
        return ret;
    }

    // Configure LEDC channel for servo
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = channel,
        .timer_sel = LEDC_TIMER_1,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = pin,
        .duty = 0,
        .hpoint = 0
    };
    ret = ledc_channel_config(&ledc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC channel");
        return ret;
    }

    // Set initial position
    servo_set_angle(servo, retract_angle);
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "Servo standup initialized");
    return ESP_OK;
}

void servo_standup_request_standup(servo_standup_t* servo) {
    if (!servo->standup_in_progress) {
        servo->standup_requested = true;
    }
}

void servo_standup_update(servo_standup_t* servo) {
    if (servo->standup_requested && !servo->standup_in_progress) {
        servo->standup_requested = false;
        servo->standup_in_progress = true;
        servo->state = STANDUP_EXTENDING;
        servo->state_start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        ESP_LOGI(TAG, "Starting standup sequence");
    }
    
    if (!servo->standup_in_progress) return;
    
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    uint32_t elapsed_time = current_time - servo->state_start_time;
    
    switch (servo->state) {
        case STANDUP_EXTENDING:
            if (elapsed_time == 0 || servo->current_angle != servo->extended_angle) {
                servo_set_angle(servo, servo->extended_angle);
                ESP_LOGI(TAG, "Extending servo to %d degrees", servo->extended_angle);
            }
            if (elapsed_time >= servo->extend_duration) {
                servo->state = STANDUP_PUSHING;
                servo->state_start_time = current_time;
                ESP_LOGI(TAG, "Holding position for push");
            }
            break;
            
        case STANDUP_PUSHING:
            if (elapsed_time >= servo->push_duration) {
                servo->state = STANDUP_RETRACTING;
                servo->state_start_time = current_time;
                servo_set_angle(servo, servo->retracted_angle);
                ESP_LOGI(TAG, "Retracting servo to %d degrees", servo->retracted_angle);
            }
            break;
            
        case STANDUP_RETRACTING:
            if (elapsed_time >= servo->retract_duration) {
                servo->state = STANDUP_COMPLETE;
                servo->state_start_time = current_time;
                ESP_LOGI(TAG, "Standup sequence complete");
            }
            break;
            
        case STANDUP_COMPLETE:
            if (elapsed_time >= 500) { // Wait 500ms before allowing next standup
                servo->state = STANDUP_IDLE;
                servo->standup_in_progress = false;
                ESP_LOGI(TAG, "Ready for next standup");
            }
            break;
            
        default:
            break;
    }
}

bool servo_standup_is_standing_up(servo_standup_t* servo) {
    return servo->standup_in_progress;
}

bool servo_standup_is_complete(servo_standup_t* servo) {
    return servo->state == STANDUP_COMPLETE;
}

void servo_standup_reset(servo_standup_t* servo) {
    servo->state = STANDUP_IDLE;
    servo->standup_in_progress = false;
    servo->standup_requested = false;
    servo_set_angle(servo, servo->retracted_angle);
    ESP_LOGI(TAG, "Servo standup reset");
}

void servo_standup_set_timings(servo_standup_t* servo, uint32_t extend, uint32_t push, uint32_t retract) {
    servo->extend_duration = extend;
    servo->push_duration = push;
    servo->retract_duration = retract;
}

void servo_standup_set_angles(servo_standup_t* servo, int extend, int retract) {
    servo->extended_angle = extend;
    servo->retracted_angle = retract;
    if (!servo->standup_in_progress) {
        servo_set_angle(servo, retract);
    }
}

standup_state_t servo_standup_get_state(servo_standup_t* servo) {
    return servo->state;
}