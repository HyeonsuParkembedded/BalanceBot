#include "imu_sensor.h"
#include "../bsw/i2c_driver.h"
#ifndef NATIVE_BUILD
#include "esp_log.h"
#endif
#include <math.h>

#ifndef NATIVE_BUILD
static const char* IMU_TAG = "IMU_SENSOR";
#else
#define IMU_TAG "IMU_SENSOR"
#endif

// MPU6050 register definitions
#define MPU6050_ADDR            0x68
#define MPU6050_WHO_AM_I        0x75
#define MPU6050_PWR_MGMT_1      0x6B
#define MPU6050_GYRO_CONFIG     0x1B
#define MPU6050_ACCEL_CONFIG    0x1C
#define MPU6050_ACCEL_XOUT_H    0x3B
#define MPU6050_GYRO_XOUT_H     0x43

esp_err_t imu_sensor_init(imu_sensor_t* sensor, i2c_port_t port, gpio_num_t sda_pin, gpio_num_t scl_pin) {
    sensor->i2c_port = port;
    sensor->data.accel_x = sensor->data.accel_y = sensor->data.accel_z = 0.0f;
    sensor->data.gyro_x = sensor->data.gyro_y = sensor->data.gyro_z = 0.0f;
    sensor->data.pitch = sensor->data.roll = 0.0f;
    sensor->data.initialized = false;

    // Initialize I2C driver
    esp_err_t ret = i2c_driver_init(port, sda_pin, scl_pin);
    if (ret != ESP_OK) {
        return ret;
    }

    // Check WHO_AM_I register
    uint8_t who_am_i;
    ret = i2c_read_register(port, MPU6050_ADDR, MPU6050_WHO_AM_I, &who_am_i, 1);
    if (ret != ESP_OK) {
        return ret;
    }

    if (who_am_i != MPU6050_ADDR) {
#ifndef NATIVE_BUILD
        ESP_LOGE(IMU_TAG, "MPU6050 not found or wrong ID: 0x%02X", who_am_i);
#endif
        return ESP_FAIL;
    }

    // Wake up MPU6050
    ret = i2c_write_register(port, MPU6050_ADDR, MPU6050_PWR_MGMT_1, 0x00);
    if (ret != ESP_OK) {
        return ret;
    }

    // Configure gyroscope (±250 degrees/s)
    ret = i2c_write_register(port, MPU6050_ADDR, MPU6050_GYRO_CONFIG, 0x00);
    if (ret != ESP_OK) {
        return ret;
    }

    // Configure accelerometer (±2g)
    ret = i2c_write_register(port, MPU6050_ADDR, MPU6050_ACCEL_CONFIG, 0x00);
    if (ret != ESP_OK) {
        return ret;
    }

    sensor->data.initialized = true;

#ifndef NATIVE_BUILD
    ESP_LOGI(IMU_TAG, "IMU sensor initialized successfully");
#endif
    return ESP_OK;
}

esp_err_t imu_sensor_update(imu_sensor_t* sensor) {
    if (!sensor->data.initialized) {
        return ESP_FAIL;
    }

    uint8_t raw_data[14];
    esp_err_t ret = i2c_read_register(sensor->i2c_port, MPU6050_ADDR, MPU6050_ACCEL_XOUT_H, raw_data, 14);
    if (ret != ESP_OK) {
        return ret;
    }

    // Parse accelerometer data
    int16_t accel_x = (int16_t)((raw_data[0] << 8) | raw_data[1]);
    int16_t accel_y = (int16_t)((raw_data[2] << 8) | raw_data[3]);
    int16_t accel_z = (int16_t)((raw_data[4] << 8) | raw_data[5]);

    // Parse gyroscope data
    int16_t gyro_x = (int16_t)((raw_data[8] << 8) | raw_data[9]);
    int16_t gyro_y = (int16_t)((raw_data[10] << 8) | raw_data[11]);
    int16_t gyro_z = (int16_t)((raw_data[12] << 8) | raw_data[13]);

    // Convert to physical units
    sensor->data.accel_x = accel_x / 16384.0f;  // ±2g range
    sensor->data.accel_y = accel_y / 16384.0f;
    sensor->data.accel_z = accel_z / 16384.0f;

    sensor->data.gyro_x = gyro_x / 131.0f;      // ±250°/s range
    sensor->data.gyro_y = gyro_y / 131.0f;
    sensor->data.gyro_z = gyro_z / 131.0f;

    // Calculate pitch and roll from accelerometer
    sensor->data.pitch = atan2(-sensor->data.accel_x, sqrt(sensor->data.accel_y * sensor->data.accel_y + sensor->data.accel_z * sensor->data.accel_z)) * 180.0f / M_PI;
    sensor->data.roll = atan2(sensor->data.accel_y, sensor->data.accel_z) * 180.0f / M_PI;

    return ESP_OK;
}

float imu_sensor_get_pitch(imu_sensor_t* sensor) {
    return sensor->data.pitch;
}

float imu_sensor_get_roll(imu_sensor_t* sensor) {
    return sensor->data.roll;
}

float imu_sensor_get_gyro_x(imu_sensor_t* sensor) {
    return sensor->data.gyro_x;
}

float imu_sensor_get_gyro_y(imu_sensor_t* sensor) {
    return sensor->data.gyro_y;
}

float imu_sensor_get_gyro_z(imu_sensor_t* sensor) {
    return sensor->data.gyro_z;
}

float imu_sensor_get_accel_x(imu_sensor_t* sensor) {
    return sensor->data.accel_x;
}

float imu_sensor_get_accel_y(imu_sensor_t* sensor) {
    return sensor->data.accel_y;
}

float imu_sensor_get_accel_z(imu_sensor_t* sensor) {
    return sensor->data.accel_z;
}

bool imu_sensor_is_initialized(imu_sensor_t* sensor) {
    return sensor->data.initialized;
}