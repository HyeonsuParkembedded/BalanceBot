#ifndef IMU_SENSOR_H
#define IMU_SENSOR_H

#ifndef NATIVE_BUILD
#include "driver/i2c.h"
#include "esp_err.h"
#else
typedef int esp_err_t;
typedef int i2c_port_t;
typedef int gpio_num_t;
#define ESP_OK 0
#define ESP_FAIL -1
#endif

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float accel_x, accel_y, accel_z;
    float gyro_x, gyro_y, gyro_z;
    float pitch, roll;
    bool initialized;
} imu_data_t;

typedef struct {
    i2c_port_t i2c_port;
    imu_data_t data;
} imu_sensor_t;

esp_err_t imu_sensor_init(imu_sensor_t* sensor, i2c_port_t port, gpio_num_t sda_pin, gpio_num_t scl_pin);
esp_err_t imu_sensor_update(imu_sensor_t* sensor);
float imu_sensor_get_pitch(imu_sensor_t* sensor);
float imu_sensor_get_roll(imu_sensor_t* sensor);
float imu_sensor_get_gyro_x(imu_sensor_t* sensor);
float imu_sensor_get_gyro_y(imu_sensor_t* sensor);
float imu_sensor_get_gyro_z(imu_sensor_t* sensor);
float imu_sensor_get_accel_x(imu_sensor_t* sensor);
float imu_sensor_get_accel_y(imu_sensor_t* sensor);
float imu_sensor_get_accel_z(imu_sensor_t* sensor);
bool imu_sensor_is_initialized(imu_sensor_t* sensor);

#ifdef __cplusplus
}
#endif

#endif // IMU_SENSOR_H