#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "input/imu_sensor.h"
#include "logic/kalman_filter.h"
#include "input/gps_sensor.h"
#include "input/encoder_sensor.h"
#include "output/motor_control.h"
#include "output/ble_controller.h"
#include "logic/pid_controller.h"
#include "output/servo_standup.h"

// Pin definitions for ESP32-S3
#define MPU6050_SDA         GPIO_NUM_21
#define MPU6050_SCL         GPIO_NUM_20

#define GPS_RX              GPIO_NUM_16
#define GPS_TX              GPIO_NUM_17

#define LEFT_MOTOR_A        GPIO_NUM_4
#define LEFT_MOTOR_B        GPIO_NUM_5
#define LEFT_MOTOR_EN       GPIO_NUM_6
#define LEFT_ENC_A          GPIO_NUM_7
#define LEFT_ENC_B          GPIO_NUM_15

#define RIGHT_MOTOR_A       GPIO_NUM_8
#define RIGHT_MOTOR_B       GPIO_NUM_9
#define RIGHT_MOTOR_EN      GPIO_NUM_10
#define RIGHT_ENC_A         GPIO_NUM_11
#define RIGHT_ENC_B         GPIO_NUM_12

#define SERVO_PIN           GPIO_NUM_18
#define SERVO_CHANNEL       LEDC_CHANNEL_2

static const char* TAG = "BALANCE_ROBOT";

// Robot components
static imu_sensor_t imu;
static kalman_filter_t kalman_pitch;
static gps_sensor_t gps;
static encoder_sensor_t left_encoder;
static motor_control_t left_motor;
static encoder_sensor_t right_encoder;
static motor_control_t right_motor;
static ble_controller_t ble_controller;
static pid_controller_t balance_pid;
static servo_standup_t servo_standup;

// Robot state variables
static float filtered_angle = 0.0f;
static float robot_velocity = 0.0f;
static bool balancing_enabled = true;

// Task handles
static TaskHandle_t balance_task_handle = NULL;
static TaskHandle_t sensor_task_handle = NULL;
static TaskHandle_t status_task_handle = NULL;

// Function prototypes
static void initialize_robot(void);
static void balance_task(void *pvParameters);
static void sensor_task(void *pvParameters);
static void status_task(void *pvParameters);
static void update_motors(float motor_output, remote_command_t cmd);
static void handle_remote_commands(void);

void app_main(void) {
    ESP_LOGI(TAG, "Balance Robot Starting...");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize robot components
    initialize_robot();
    
    ESP_LOGI(TAG, "Robot initialized successfully!");
    
    // Create tasks
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, &sensor_task_handle);
    xTaskCreate(balance_task, "balance_task", 4096, NULL, 4, &balance_task_handle);
    xTaskCreate(status_task, "status_task", 4096, NULL, 3, &status_task_handle);
    
    ESP_LOGI(TAG, "Tasks created, starting main loop...");
    
    // Main loop
    while (1) {
        // Update servo standup mechanism
        servo_standup_update(&servo_standup);
        
        // Update BLE
        ble_controller_update(&ble_controller);
        
        // Handle remote commands
        handle_remote_commands();
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void initialize_robot(void) {
    esp_err_t ret;
    
    // Initialize MPU6050
    ret = imu_sensor_init(&imu, I2C_NUM_0, MPU6050_SDA, MPU6050_SCL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MPU6050!");
        while(1) vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ESP_LOGI(TAG, "MPU6050 initialized");
    
    // Initialize Kalman filter
    kalman_filter_init(&kalman_pitch);
    kalman_filter_set_angle(&kalman_pitch, 0.0f);
    ESP_LOGI(TAG, "Kalman filter initialized");
    
    // Initialize GPS
    ret = gps_sensor_init(&gps, UART_NUM_2, GPS_TX, GPS_RX, 9600);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize GPS!");
    } else {
        ESP_LOGI(TAG, "GPS initialized");
    }
    
    // Initialize motors
    ret = encoder_sensor_init(&left_encoder, LEFT_ENC_A, LEFT_ENC_B, 360, 6.5f);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize left encoder!");
        while(1) vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ret = motor_control_init(&left_motor, LEFT_MOTOR_A, LEFT_MOTOR_B, LEFT_MOTOR_EN, LEDC_CHANNEL_0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize left motor!");
    } else {
        ESP_LOGI(TAG, "Left motor initialized");
    }
    
    ret = encoder_sensor_init(&right_encoder, RIGHT_ENC_A, RIGHT_ENC_B, 360, 6.5f);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize right encoder!");
        while(1) vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ret = motor_control_init(&right_motor, RIGHT_MOTOR_A, RIGHT_MOTOR_B, RIGHT_MOTOR_EN, LEDC_CHANNEL_1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize right motor!");
    } else {
        ESP_LOGI(TAG, "Right motor initialized");
    }
    
    // Initialize BLE
    ret = ble_controller_init(&ble_controller, "BalanceBot");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize BLE!");
    } else {
        ESP_LOGI(TAG, "BLE initialized");
    }
    
    // Initialize servo standup
    ret = servo_standup_init(&servo_standup, SERVO_PIN, SERVO_CHANNEL, 90, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize servo standup!");
    } else {
        ESP_LOGI(TAG, "Servo standup initialized");
    }
    
    // Initialize PID controllers
    pid_controller_init(&balance_pid, 50.0f, 0.5f, 2.0f);
    pid_controller_set_output_limits(&balance_pid, -255.0f, 255.0f);
    ESP_LOGI(TAG, "PID controllers initialized");
}

static void sensor_task(void *pvParameters) {
    ESP_LOGI(TAG, "Sensor task started");
    
    while (1) {
        // Update IMU
        esp_err_t ret = imu_sensor_update(&imu);
        if (ret == ESP_OK) {
            // Apply Kalman filter to pitch angle
            float dt = 0.02f; // 50Hz update rate
            filtered_angle = kalman_filter_get_angle(&kalman_pitch, 
                                                   imu_sensor_get_pitch(&imu),
                                                   imu_sensor_get_gyro_y(&imu), 
                                                   dt);
        }
        
        // Update GPS
        gps_sensor_update(&gps);
        
        // Update motor speeds
        encoder_sensor_update_speed(&left_encoder);
        encoder_sensor_update_speed(&right_encoder);
        
        // Calculate robot velocity (average of both wheels)
        robot_velocity = (encoder_sensor_get_speed(&left_encoder) + encoder_sensor_get_speed(&right_encoder)) / 2.0f;
        
        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz
    }
}

static void balance_task(void *pvParameters) {
    ESP_LOGI(TAG, "Balance task started");
    
    while (1) {
        remote_command_t cmd = ble_controller_get_command(&ble_controller);
        
        if (cmd.balance && balancing_enabled && !servo_standup_is_standing_up(&servo_standup)) {
            // Set PID setpoint to maintain balance (0 degrees)
            pid_controller_set_setpoint(&balance_pid, 0.0f);
            
            // Compute balance control
            float motor_output = pid_controller_compute(&balance_pid, filtered_angle);
            
            // Apply motor commands
            update_motors(motor_output, cmd);
        } else {
            // Stop motors if balancing is disabled or standing up
            motor_control_stop(&left_motor);
            motor_control_stop(&right_motor);
            pid_controller_reset(&balance_pid);
        }
        
        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz control loop
    }
}

static void status_task(void *pvParameters) {
    ESP_LOGI(TAG, "Status task started");
    
    while (1) {
        // Send BLE status
        if (ble_controller_is_connected(&ble_controller)) {
            char status[128];
            snprintf(status, sizeof(status), "Angle:%.2f Vel:%.1f GPS:%s", 
                    filtered_angle, robot_velocity, 
                    gps_sensor_has_fix(&gps) ? "OK" : "NO");
            
            if (gps_sensor_has_fix(&gps)) {
                char gps_info[64];
                snprintf(gps_info, sizeof(gps_info), " Lat:%.6f Lon:%.6f", 
                        gps_sensor_get_latitude(&gps), gps_sensor_get_longitude(&gps));
                strncat(status, gps_info, sizeof(status) - strlen(status) - 1);
            }
            
            ble_controller_send_status(&ble_controller, status);
        }
        
        // Print debug info to serial
        ESP_LOGI(TAG, "Angle: %.2f | Velocity: %.2f | GPS: %s", 
                filtered_angle, robot_velocity, 
                gps_sensor_has_fix(&gps) ? "Valid" : "Invalid");
        
        if (gps_sensor_has_fix(&gps)) {
            ESP_LOGI(TAG, "GPS - Lat: %.6f | Lon: %.6f | Sats: %d", 
                    gps_sensor_get_latitude(&gps), 
                    gps_sensor_get_longitude(&gps),
                    gps_sensor_get_satellites(&gps));
        }
        
        ESP_LOGI(TAG, "Standup: %s", servo_standup_is_standing_up(&servo_standup) ? "Active" : "Idle");
        
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1Hz status updates
    }
}

static void update_motors(float motor_output, remote_command_t cmd) {
    // Apply turn adjustment
    float turn_adjustment = cmd.turn * 0.5f; // Scale turn command
    
    float left_motor_speed = motor_output - turn_adjustment;
    float right_motor_speed = motor_output + turn_adjustment;
    
    // Constrain motor speeds
    if (left_motor_speed > 255.0f) left_motor_speed = 255.0f;
    if (left_motor_speed < -255.0f) left_motor_speed = -255.0f;
    if (right_motor_speed > 255.0f) right_motor_speed = 255.0f;
    if (right_motor_speed < -255.0f) right_motor_speed = -255.0f;
    
    // Apply to motors
    motor_control_set_speed(&left_motor, (int)left_motor_speed);
    motor_control_set_speed(&right_motor, (int)right_motor_speed);
}

static void handle_remote_commands(void) {
    remote_command_t cmd = ble_controller_get_command(&ble_controller);
    
    // Handle standup command
    if (cmd.standup && !servo_standup_is_standing_up(&servo_standup)) {
        servo_standup_request_standup(&servo_standup);
        ble_controller_send_status(&ble_controller, "Standing up...");
    }
    
    // Update balancing state
    balancing_enabled = cmd.balance;
}