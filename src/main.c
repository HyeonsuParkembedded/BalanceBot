#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "config.h"

#include "input/imu_sensor.h"
#include "logic/kalman_filter.h"
#include "input/gps_sensor.h"
#include "input/encoder_sensor.h"
#include "output/motor_control.h"
#include "output/ble_controller.h"
#include "logic/pid_controller.h"
#include "output/servo_standup.h"

// Pin definitions are now in config.h

static const char* TAG = "BALANCE_ROBOT";

// Robot state machine
typedef enum {
    ROBOT_STATE_INIT,
    ROBOT_STATE_IDLE,
    ROBOT_STATE_BALANCING,
    ROBOT_STATE_STANDING_UP,
    ROBOT_STATE_FALLEN,
    ROBOT_STATE_ERROR
} robot_state_t;

static robot_state_t current_state = ROBOT_STATE_INIT;
static SemaphoreHandle_t state_mutex = NULL;

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

// Robot state variables (protected by mutex)
static float filtered_angle = 0.0f;
static float robot_velocity = 0.0f;
static bool balancing_enabled = true;

// Mutex for protecting shared data
static SemaphoreHandle_t data_mutex = NULL;

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

// Thread-safe data access functions
static float get_filtered_angle(void);
static void set_filtered_angle(float angle);
static float get_robot_velocity(void);
static void set_robot_velocity(float velocity);
static bool get_balancing_enabled(void);
static void set_balancing_enabled(bool enabled);

// State machine functions
static robot_state_t get_robot_state(void);
static void set_robot_state(robot_state_t new_state);
static const char* get_state_name(robot_state_t state);
static void state_machine_update(void);

void app_main(void) {
    ESP_LOGI(TAG, "Balance Robot Starting...");

    // Create mutexes for data protection
    data_mutex = xSemaphoreCreateMutex();
    state_mutex = xSemaphoreCreateMutex();
    if (data_mutex == NULL || state_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutexes!");
        while(1) vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ESP_LOGI(TAG, "Mutexes created");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize robot components
    initialize_robot();

    // Set initial state to idle after successful initialization
    set_robot_state(ROBOT_STATE_IDLE);
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
    ret = imu_sensor_init(&imu, CONFIG_MPU6050_I2C_PORT, CONFIG_MPU6050_SDA_PIN, CONFIG_MPU6050_SCL_PIN);
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
    ret = gps_sensor_init(&gps, CONFIG_GPS_UART_PORT, CONFIG_GPS_TX_PIN, CONFIG_GPS_RX_PIN, CONFIG_GPS_BAUDRATE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize GPS!");
    } else {
        ESP_LOGI(TAG, "GPS initialized");
    }
    
    // Initialize motors
    ret = encoder_sensor_init(&left_encoder, CONFIG_LEFT_ENC_A_PIN, CONFIG_LEFT_ENC_B_PIN, CONFIG_ENCODER_PPR, CONFIG_WHEEL_DIAMETER_CM);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize left encoder!");
        while(1) vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ret = motor_control_init(&left_motor, CONFIG_LEFT_MOTOR_A_PIN, CONFIG_LEFT_MOTOR_B_PIN, CONFIG_LEFT_MOTOR_EN_PIN, CONFIG_LEFT_MOTOR_CHANNEL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize left motor!");
    } else {
        ESP_LOGI(TAG, "Left motor initialized");
    }
    
    ret = encoder_sensor_init(&right_encoder, CONFIG_RIGHT_ENC_A_PIN, CONFIG_RIGHT_ENC_B_PIN, CONFIG_ENCODER_PPR, CONFIG_WHEEL_DIAMETER_CM);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize right encoder!");
        while(1) vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ret = motor_control_init(&right_motor, CONFIG_RIGHT_MOTOR_A_PIN, CONFIG_RIGHT_MOTOR_B_PIN, CONFIG_RIGHT_MOTOR_EN_PIN, CONFIG_RIGHT_MOTOR_CHANNEL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize right motor!");
    } else {
        ESP_LOGI(TAG, "Right motor initialized");
    }
    
    // Initialize BLE
    ret = ble_controller_init(&ble_controller, CONFIG_BLE_DEVICE_NAME);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize BLE!");
    } else {
        ESP_LOGI(TAG, "BLE initialized");
    }
    
    // Initialize servo standup
    ret = servo_standup_init(&servo_standup, CONFIG_SERVO_PIN, CONFIG_SERVO_CHANNEL, CONFIG_SERVO_EXTENDED_ANGLE, CONFIG_SERVO_RETRACTED_ANGLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize servo standup!");
    } else {
        ESP_LOGI(TAG, "Servo standup initialized");
    }
    
    // Initialize PID controllers
    pid_controller_init(&balance_pid, CONFIG_BALANCE_PID_KP, CONFIG_BALANCE_PID_KI, CONFIG_BALANCE_PID_KD);
    pid_controller_set_output_limits(&balance_pid, CONFIG_PID_OUTPUT_MIN, CONFIG_PID_OUTPUT_MAX);
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
            set_filtered_angle(kalman_filter_get_angle(&kalman_pitch, 
                                                   imu_sensor_get_pitch(&imu),
                                                   imu_sensor_get_gyro_y(&imu), 
                                                   dt));
        }
        
        // Update GPS
        gps_sensor_update(&gps);
        
        // Update motor speeds
        encoder_sensor_update_speed(&left_encoder);
        encoder_sensor_update_speed(&right_encoder);
        
        // Calculate robot velocity (average of both wheels)
        set_robot_velocity((encoder_sensor_get_speed(&left_encoder) + encoder_sensor_get_speed(&right_encoder)) / 2.0f);
        
        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz
    }
}

static void balance_task(void *pvParameters) {
    ESP_LOGI(TAG, "Balance task started");

    while (1) {
        // Update state machine first
        state_machine_update();

        remote_command_t cmd = ble_controller_get_command(&ble_controller);
        robot_state_t state = get_robot_state();

        // Handle different robot states
        switch (state) {
        case ROBOT_STATE_IDLE:
            // Stop motors and reset PID
            motor_control_stop(&left_motor);
            motor_control_stop(&right_motor);
            pid_controller_reset(&balance_pid);
            break;

        case ROBOT_STATE_BALANCING:
            // Set PID setpoint to maintain balance (0 degrees)
            pid_controller_set_setpoint(&balance_pid, CONFIG_BALANCE_ANGLE_TARGET);

            // Compute balance control
            float motor_output = pid_controller_compute(&balance_pid, get_filtered_angle());

            // Apply motor commands
            update_motors(motor_output, cmd);
            break;

        case ROBOT_STATE_STANDING_UP:
            // Motors stopped during standup
            motor_control_stop(&left_motor);
            motor_control_stop(&right_motor);
            pid_controller_reset(&balance_pid);
            break;

        case ROBOT_STATE_FALLEN:
        case ROBOT_STATE_ERROR:
        default:
            // Emergency stop
            motor_control_stop(&left_motor);
            motor_control_stop(&right_motor);
            pid_controller_reset(&balance_pid);
            break;
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
                    get_filtered_angle(), get_robot_velocity(), 
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
                get_filtered_angle(), get_robot_velocity(), 
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
    set_balancing_enabled(cmd.balance);
}

// Thread-safe data access functions
static float get_filtered_angle(void) {
    float angle = 0.0f;
    if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
        angle = filtered_angle;
        xSemaphoreGive(data_mutex);
    }
    return angle;
}

static void set_filtered_angle(float angle) {
    if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
        filtered_angle = angle;
        xSemaphoreGive(data_mutex);
    }
}

static float get_robot_velocity(void) {
    float velocity = 0.0f;
    if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
        velocity = robot_velocity;
        xSemaphoreGive(data_mutex);
    }
    return velocity;
}

static void set_robot_velocity(float velocity) {
    if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
        robot_velocity = velocity;
        xSemaphoreGive(data_mutex);
    }
}

static bool get_balancing_enabled(void) {
    bool enabled = false;
    if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
        enabled = balancing_enabled;
        xSemaphoreGive(data_mutex);
    }
    return enabled;
}

static void set_balancing_enabled(bool enabled) {
    if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
        balancing_enabled = enabled;
        xSemaphoreGive(data_mutex);
    }
}

// State machine functions
static robot_state_t get_robot_state(void) {
    robot_state_t state = ROBOT_STATE_ERROR;
    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        state = current_state;
        xSemaphoreGive(state_mutex);
    }
    return state;
}

static void set_robot_state(robot_state_t new_state) {
    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        if (current_state != new_state) {
            ESP_LOGI(TAG, "State change: %s -> %s",
                    get_state_name(current_state), get_state_name(new_state));
            current_state = new_state;
        }
        xSemaphoreGive(state_mutex);
    }
}

static const char* get_state_name(robot_state_t state) {
    switch (state) {
        case ROBOT_STATE_INIT: return "INIT";
        case ROBOT_STATE_IDLE: return "IDLE";
        case ROBOT_STATE_BALANCING: return "BALANCING";
        case ROBOT_STATE_STANDING_UP: return "STANDING_UP";
        case ROBOT_STATE_FALLEN: return "FALLEN";
        case ROBOT_STATE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

static void state_machine_update(void) {
    robot_state_t current = get_robot_state();
    float angle = get_filtered_angle();
    remote_command_t cmd = ble_controller_get_command(&ble_controller);

    // State transitions based on conditions
    switch (current) {
    case ROBOT_STATE_IDLE:
        if (cmd.balance && !servo_standup_is_standing_up(&servo_standup)) {
            set_robot_state(ROBOT_STATE_BALANCING);
        } else if (cmd.standup) {
            set_robot_state(ROBOT_STATE_STANDING_UP);
        }
        // Check if fallen (angle too large)
        if (fabsf(angle) > CONFIG_FALLEN_ANGLE_THRESHOLD) {
            set_robot_state(ROBOT_STATE_FALLEN);
        }
        break;

    case ROBOT_STATE_BALANCING:
        if (!cmd.balance) {
            set_robot_state(ROBOT_STATE_IDLE);
        } else if (cmd.standup) {
            set_robot_state(ROBOT_STATE_STANDING_UP);
        } else if (fabsf(angle) > 45.0f) {
            set_robot_state(ROBOT_STATE_FALLEN);
        }
        break;

    case ROBOT_STATE_STANDING_UP:
        if (servo_standup_is_complete(&servo_standup)) {
            set_robot_state(ROBOT_STATE_IDLE);
        } else if (!servo_standup_is_standing_up(&servo_standup)) {
            // Standup failed or cancelled
            set_robot_state(ROBOT_STATE_IDLE);
        }
        break;

    case ROBOT_STATE_FALLEN:
        // Can only recover through standup
        if (cmd.standup) {
            set_robot_state(ROBOT_STATE_STANDING_UP);
        }
        break;

    case ROBOT_STATE_ERROR:
        // Manual recovery required - could add auto-recovery logic here
        break;

    default:
        set_robot_state(ROBOT_STATE_ERROR);
        break;
    }
}