#ifndef MPU6050_SENSOR_H
#define MPU6050_SENSOR_H

#ifndef NATIVE_BUILD
#include "driver/i2c.h"
#include "esp_err.h"
#else
typedef int esp_err_t;
typedef int i2c_port_t;
#define ESP_OK 0
#define ESP_FAIL -1
#endif
#include <math.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MPU6050_ADDR            0x68
#define MPU6050_WHO_AM_I        0x75
#define MPU6050_PWR_MGMT_1      0x6B
#define MPU6050_GYRO_CONFIG     0x1B
#define MPU6050_ACCEL_CONFIG    0x1C
#define MPU6050_ACCEL_XOUT_H    0x3B
#define MPU6050_GYRO_XOUT_H     0x43

typedef struct {
    float accel_x, accel_y, accel_z;
    float gyro_x, gyro_y, gyro_z;
    float pitch, roll;
    bool initialized;
} mpu6050_data_t;

typedef struct {
    i2c_port_t i2c_port;
    mpu6050_data_t data;
} mpu6050_sensor_t;

esp_err_t mpu6050_sensor_init(mpu6050_sensor_t* sensor, i2c_port_t port, int sda_pin, int scl_pin);
esp_err_t mpu6050_sensor_update(mpu6050_sensor_t* sensor);
float mpu6050_sensor_get_pitch(mpu6050_sensor_t* sensor);
float mpu6050_sensor_get_roll(mpu6050_sensor_t* sensor);
float mpu6050_sensor_get_gyro_x(mpu6050_sensor_t* sensor);
float mpu6050_sensor_get_gyro_y(mpu6050_sensor_t* sensor);
float mpu6050_sensor_get_gyro_z(mpu6050_sensor_t* sensor);
float mpu6050_sensor_get_accel_x(mpu6050_sensor_t* sensor);
float mpu6050_sensor_get_accel_y(mpu6050_sensor_t* sensor);
float mpu6050_sensor_get_accel_z(mpu6050_sensor_t* sensor);
bool mpu6050_sensor_is_ready(mpu6050_sensor_t* sensor);

#ifdef __cplusplus
}
#endif

#endif