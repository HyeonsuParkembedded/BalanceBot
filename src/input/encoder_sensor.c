#include "encoder_sensor.h"
#ifndef NATIVE_BUILD
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif
#include <math.h>

#ifndef NATIVE_BUILD
static const char* ENCODER_TAG = "ENCODER_SENSOR";
#else
#define ENCODER_TAG "ENCODER_SENSOR"
#endif

static void IRAM_ATTR encoder_isr_handler(void* arg) {
    encoder_sensor_t* encoder = (encoder_sensor_t*)arg;

#ifndef NATIVE_BUILD
    int msb = gpio_get_level(encoder->encoder_pin_a);
    int lsb = gpio_get_level(encoder->encoder_pin_b);
#else
    // Mock for native build
    static int mock_counter = 0;
    int msb = (mock_counter >> 1) & 1;
    int lsb = mock_counter & 1;
    mock_counter++;
#endif

    int encoded = (msb << 1) | lsb;
    int sum = (encoder->last_encoded << 2) | encoded;

    if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
        encoder->encoder_count++;
    }
    if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
        encoder->encoder_count--;
    }

    encoder->last_encoded = encoded;
}

esp_err_t encoder_sensor_init(encoder_sensor_t* encoder,
                             gpio_num_t pin_a, gpio_num_t pin_b,
                             int pulses_per_rev, float wheel_diam) {
    encoder->encoder_pin_a = pin_a;
    encoder->encoder_pin_b = pin_b;
    encoder->ppr = pulses_per_rev;
    encoder->wheel_diameter = wheel_diam;
    encoder->encoder_count = 0;
    encoder->last_encoded = 0;
    encoder->last_time = 0;
    encoder->last_position = 0;
    encoder->current_speed = 0.0f;

#ifndef NATIVE_BUILD
    // Configure encoder pins
    gpio_config_t encoder_config = {
        .pin_bit_mask = (1ULL << pin_a) | (1ULL << pin_b),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    esp_err_t ret = gpio_config(&encoder_config);
    if (ret != ESP_OK) {
        ESP_LOGE(ENCODER_TAG, "Failed to configure encoder GPIO");
        return ret;
    }

    // Install interrupt service
    ret = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(ENCODER_TAG, "Failed to install ISR service");
        return ret;
    }

    // Add ISR handlers
    ret = gpio_isr_handler_add(pin_a, encoder_isr_handler, encoder);
    if (ret != ESP_OK) {
        ESP_LOGE(ENCODER_TAG, "Failed to add ISR handler for encoder A");
        return ret;
    }

    ret = gpio_isr_handler_add(pin_b, encoder_isr_handler, encoder);
    if (ret != ESP_OK) {
        ESP_LOGE(ENCODER_TAG, "Failed to add ISR handler for encoder B");
        return ret;
    }

    encoder->last_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    ESP_LOGI(ENCODER_TAG, "Encoder sensor initialized");
#endif

    return ESP_OK;
}

void encoder_sensor_reset(encoder_sensor_t* encoder) {
    encoder->encoder_count = 0;
    encoder->last_position = 0;
    encoder->current_speed = 0.0f;
#ifndef NATIVE_BUILD
    encoder->last_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
#endif
}

int32_t encoder_sensor_get_position(encoder_sensor_t* encoder) {
    return encoder->encoder_count;
}

float encoder_sensor_get_distance(encoder_sensor_t* encoder) {
    float revolutions = (float)encoder->encoder_count / encoder->ppr;
    return revolutions * M_PI * encoder->wheel_diameter;
}

float encoder_sensor_get_speed(encoder_sensor_t* encoder) {
    return encoder->current_speed;
}

esp_err_t encoder_sensor_update_speed(encoder_sensor_t* encoder) {
#ifndef NATIVE_BUILD
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    uint32_t time_diff = current_time - encoder->last_time;

    if (time_diff >= 100) { // Update every 100ms
        int32_t position_diff = encoder->encoder_count - encoder->last_position;
        float distance_diff = (float)position_diff / encoder->ppr * M_PI * encoder->wheel_diameter;
        encoder->current_speed = (distance_diff / (time_diff / 1000.0f)) * 100.0f; // cm/s

        encoder->last_time = current_time;
        encoder->last_position = encoder->encoder_count;
    }
#endif
    return ESP_OK;
}