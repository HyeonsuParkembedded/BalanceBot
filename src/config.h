#ifndef CONFIG_H
#define CONFIG_H

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/i2c.h"
#include "driver/uart.h"

#ifdef __cplusplus
extern "C" {
#endif

// ========================================
// Hardware Pin Configuration
// ========================================

// MPU6050 I2C pins
#define CONFIG_MPU6050_SDA_PIN          GPIO_NUM_21
#define CONFIG_MPU6050_SCL_PIN          GPIO_NUM_20
#define CONFIG_MPU6050_I2C_PORT         I2C_NUM_0

// GPS UART pins
#define CONFIG_GPS_RX_PIN               GPIO_NUM_16
#define CONFIG_GPS_TX_PIN               GPIO_NUM_17
#define CONFIG_GPS_UART_PORT            UART_NUM_2
#define CONFIG_GPS_BAUDRATE             9600

// Left motor pins
#define CONFIG_LEFT_MOTOR_A_PIN         GPIO_NUM_4
#define CONFIG_LEFT_MOTOR_B_PIN         GPIO_NUM_5
#define CONFIG_LEFT_MOTOR_EN_PIN        GPIO_NUM_6
#define CONFIG_LEFT_MOTOR_CHANNEL       LEDC_CHANNEL_0

// Left encoder pins
#define CONFIG_LEFT_ENC_A_PIN           GPIO_NUM_7
#define CONFIG_LEFT_ENC_B_PIN           GPIO_NUM_15

// Right motor pins
#define CONFIG_RIGHT_MOTOR_A_PIN        GPIO_NUM_8
#define CONFIG_RIGHT_MOTOR_B_PIN        GPIO_NUM_9
#define CONFIG_RIGHT_MOTOR_EN_PIN       GPIO_NUM_10
#define CONFIG_RIGHT_MOTOR_CHANNEL      LEDC_CHANNEL_1

// Right encoder pins
#define CONFIG_RIGHT_ENC_A_PIN          GPIO_NUM_11
#define CONFIG_RIGHT_ENC_B_PIN          GPIO_NUM_12

// Servo pins
#define CONFIG_SERVO_PIN                GPIO_NUM_18
#define CONFIG_SERVO_CHANNEL            LEDC_CHANNEL_2

// ========================================
// Control System Parameters
// ========================================

// PID Controller tuning
#define CONFIG_BALANCE_PID_KP           50.0f
#define CONFIG_BALANCE_PID_KI           0.5f
#define CONFIG_BALANCE_PID_KD           2.0f
#define CONFIG_PID_OUTPUT_MIN           -255.0f
#define CONFIG_PID_OUTPUT_MAX           255.0f

// Kalman Filter parameters
#define CONFIG_KALMAN_Q_ANGLE           0.001f
#define CONFIG_KALMAN_Q_BIAS            0.003f
#define CONFIG_KALMAN_R_MEASURE         0.03f

// Robot physical parameters
#define CONFIG_WHEEL_DIAMETER_CM        6.5f
#define CONFIG_ENCODER_PPR              360

// State machine thresholds
#define CONFIG_FALLEN_ANGLE_THRESHOLD   45.0f
#define CONFIG_BALANCE_ANGLE_TARGET     0.0f

// Servo parameters
#define CONFIG_SERVO_EXTENDED_ANGLE     90
#define CONFIG_SERVO_RETRACTED_ANGLE    0

// ========================================
// Task Configuration
// ========================================

// Task stack sizes
#define CONFIG_SENSOR_TASK_STACK        4096
#define CONFIG_BALANCE_TASK_STACK       4096
#define CONFIG_STATUS_TASK_STACK        4096

// Task priorities
#define CONFIG_SENSOR_TASK_PRIORITY     5
#define CONFIG_BALANCE_TASK_PRIORITY    4
#define CONFIG_STATUS_TASK_PRIORITY     3

// Task update rates (ms)
#define CONFIG_SENSOR_UPDATE_RATE       20    // 50Hz
#define CONFIG_BALANCE_UPDATE_RATE      20    // 50Hz
#define CONFIG_STATUS_UPDATE_RATE       1000  // 1Hz

// ========================================
// Communication Configuration
// ========================================

// BLE device name
#define CONFIG_BLE_DEVICE_NAME          "BalanceBot"

// Status message buffer size
#define CONFIG_STATUS_BUFFER_SIZE       128

// ========================================
// System Configuration
// ========================================

// Watchdog timeout (ms)
#define CONFIG_WATCHDOG_TIMEOUT_MS      5000

// Maximum initialization retries
#define CONFIG_MAX_INIT_RETRIES         3

// Error recovery delay (ms)
#define CONFIG_ERROR_RECOVERY_DELAY     1000

#ifdef __cplusplus
}
#endif

#endif // CONFIG_H