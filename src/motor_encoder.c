#include "motor_encoder.h"
#ifndef NATIVE_BUILD
#include "esp_log.h"
#endif
#include <math.h>

#ifndef NATIVE_BUILD
static const char* MOTOR_TAG = "MOTOR_ENCODER";
#else
#define MOTOR_TAG "MOTOR_ENCODER"
#endif

static motor_encoder_t* motor_instances[2] = {NULL, NULL};
static bool ledc_timer_initialized = false;

static void IRAM_ATTR encoder_isr_handler(void* arg) {
    motor_encoder_t* motor = (motor_encoder_t*)arg;
    
    int msb = gpio_get_level(motor->encoder_pin_a);
    int lsb = gpio_get_level(motor->encoder_pin_b);
    
    int encoded = (msb << 1) | lsb;
    int sum = (motor->last_encoded << 2) | encoded;
    
    if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
        motor->encoder_count++;
    }
    if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
        motor->encoder_count--;
    }
    
    motor->last_encoded = encoded;
}

esp_err_t motor_encoder_init(motor_encoder_t* motor, 
                            gpio_num_t enc_a, gpio_num_t enc_b,
                            gpio_num_t mot_a, gpio_num_t mot_b,
                            gpio_num_t enable_pin, ledc_channel_t enable_ch,
                            int pulses_per_rev, float wheel_diam) {
    
    motor->encoder_pin_a = enc_a;
    motor->encoder_pin_b = enc_b;
    motor->motor_pin_a = mot_a;
    motor->motor_pin_b = mot_b;
    motor->enable_pin = enable_pin;
    motor->enable_channel = enable_ch;
    motor->ppr = pulses_per_rev;
    motor->wheel_diameter = wheel_diam;
    motor->encoder_count = 0;
    motor->last_encoded = 0;
    motor->last_time = 0;
    motor->last_position = 0;
    motor->current_speed = 0.0f;
    
    // Configure encoder pins
    gpio_config_t encoder_config = {
        .pin_bit_mask = (1ULL << enc_a) | (1ULL << enc_b),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    esp_err_t ret = gpio_config(&encoder_config);
    if (ret != ESP_OK) {
ESP_LOGE(MOTOR_TAG, "Failed to configure encoder GPIO");
        return ret;
    }
    
    // Configure motor pins
    gpio_config_t motor_config = {
        .pin_bit_mask = (1ULL << mot_a) | (1ULL << mot_b),
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
    
    // Initialize LEDC timer only once
    if (!ledc_timer_initialized) {
        ledc_timer_config_t ledc_timer = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .timer_num = LEDC_TIMER_0,
            .duty_resolution = LEDC_TIMER_8_BIT,
            .freq_hz = 5000,
            .clk_cfg = LEDC_AUTO_CLK
        };
        ret = ledc_timer_config(&ledc_timer);
        if (ret != ESP_OK) {
            ESP_LOGE(MOTOR_TAG, "Failed to configure LEDC timer");
            return ret;
        }
        ledc_timer_initialized = true;
    }
    
    // Configure LEDC channel for motor enable (PWM)
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = enable_ch,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = enable_pin,
        .duty = 0,
        .hpoint = 0
    };
    ret = ledc_channel_config(&ledc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(MOTOR_TAG, "Failed to configure LEDC channel");
        return ret;
    }
    
    // Install interrupt service only once
    ret = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(MOTOR_TAG, "Failed to install ISR service");
        return ret;
    }
    
    // Add ISR handlers
    ret = gpio_isr_handler_add(enc_a, encoder_isr_handler, motor);
    if (ret != ESP_OK) {
        ESP_LOGE(MOTOR_TAG, "Failed to add ISR handler for encoder A");
        return ret;
    }
    
    ret = gpio_isr_handler_add(enc_b, encoder_isr_handler, motor);
    if (ret != ESP_OK) {
        ESP_LOGE(MOTOR_TAG, "Failed to add ISR handler for encoder B");
        return ret;
    }
    
    motor->last_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    ESP_LOGI(MOTOR_TAG, "Motor encoder initialized");
    return ESP_OK;
}

void motor_encoder_set_speed(motor_encoder_t* motor, int speed) {
    // Constrain speed
    if (speed > 255) speed = 255;
    if (speed < -255) speed = -255;
    
    if (speed > 0) {
        gpio_set_level(motor->motor_pin_a, 1);
        gpio_set_level(motor->motor_pin_b, 0);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, motor->enable_channel, speed);
    } else if (speed < 0) {
        gpio_set_level(motor->motor_pin_a, 0);
        gpio_set_level(motor->motor_pin_b, 1);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, motor->enable_channel, -speed);
    } else {
        motor_encoder_stop(motor);
        return;
    }
    
    ledc_update_duty(LEDC_LOW_SPEED_MODE, motor->enable_channel);
}

void motor_encoder_stop(motor_encoder_t* motor) {
    gpio_set_level(motor->motor_pin_a, 0);
    gpio_set_level(motor->motor_pin_b, 0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, motor->enable_channel, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, motor->enable_channel);
}

int32_t motor_encoder_get_position(motor_encoder_t* motor) {
    return motor->encoder_count;
}

float motor_encoder_get_distance(motor_encoder_t* motor) {
    float revolutions = (float)motor->encoder_count / (float)motor->ppr;
    float circumference = motor->wheel_diameter * M_PI;
    return revolutions * circumference;
}

float motor_encoder_get_speed(motor_encoder_t* motor) {
    return motor->current_speed;
}

void motor_encoder_reset_position(motor_encoder_t* motor) {
    motor->encoder_count = 0;
}

void motor_encoder_update_speed(motor_encoder_t* motor) {
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    int32_t current_position = motor->encoder_count;
    
    if (current_time - motor->last_time >= 50) { // Update every 50ms
        int32_t delta_position = current_position - motor->last_position;
        float delta_time = (current_time - motor->last_time) / 1000.0f; // Convert to seconds
        
        float delta_distance = (delta_position / (float)motor->ppr) * (motor->wheel_diameter * M_PI);
        motor->current_speed = delta_distance / delta_time; // cm/s
        
        motor->last_time = current_time;
        motor->last_position = current_position;
    }
}