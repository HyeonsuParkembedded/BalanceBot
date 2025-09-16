#include "mpu6050_sensor.h"
#ifndef NATIVE_BUILD
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

#ifndef NATIVE_BUILD
static const char* TAG = "MPU6050";
#else
#define TAG "MPU6050"
#endif

static esp_err_t mpu6050_write_register(mpu6050_sensor_t* sensor, uint8_t reg, uint8_t value);
static esp_err_t mpu6050_read_register(mpu6050_sensor_t* sensor, uint8_t reg, uint8_t* data, size_t len);
static esp_err_t mpu6050_read_raw_data(mpu6050_sensor_t* sensor, int16_t* accel, int16_t* gyro);

esp_err_t mpu6050_sensor_init(mpu6050_sensor_t* sensor, i2c_port_t port, int sda_pin, int scl_pin) {
    sensor->i2c_port = port;
    sensor->data.accel_x = sensor->data.accel_y = sensor->data.accel_z = 0.0f;
    sensor->data.gyro_x = sensor->data.gyro_y = sensor->data.gyro_z = 0.0f;
    sensor->data.pitch = sensor->data.roll = 0.0f;
    sensor->data.initialized = false;

    esp_err_t ret;
    
    // I2C configuration
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)sda_pin;
    conf.scl_io_num = (gpio_num_t)scl_pin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000; // 400kHz
    
    ret = i2c_param_config(sensor->i2c_port, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C param config failed");
        return ret;
    }
    
    ret = i2c_driver_install(sensor->i2c_port, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed");
        return ret;
    }
    
    // Check device ID
    uint8_t who_am_i;
    ret = mpu6050_read_register(sensor, MPU6050_WHO_AM_I, &who_am_i, 1);
    if (ret != ESP_OK || who_am_i != 0x68) {
        ESP_LOGE(TAG, "MPU6050 not found or wrong ID: 0x%02X", who_am_i);
        return ESP_FAIL;
    }
    
    // Wake up device
    ret = mpu6050_write_register(sensor, MPU6050_PWR_MGMT_1, 0x00);
    if (ret != ESP_OK) return ret;
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Configure gyroscope (±250°/s)
    ret = mpu6050_write_register(sensor, MPU6050_GYRO_CONFIG, 0x00);
    if (ret != ESP_OK) return ret;
    
    // Configure accelerometer (±2g)
    ret = mpu6050_write_register(sensor, MPU6050_ACCEL_CONFIG, 0x00);
    if (ret != ESP_OK) return ret;
    
    sensor->data.initialized = true;
    ESP_LOGI(TAG, "MPU6050 initialized successfully");
    
    return ESP_OK;
}

static esp_err_t mpu6050_write_register(mpu6050_sensor_t* sensor, uint8_t reg, uint8_t value) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(sensor->i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    return ret;
}

static esp_err_t mpu6050_read_register(mpu6050_sensor_t* sensor, uint8_t reg, uint8_t* data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true);
    
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(sensor->i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    return ret;
}

static esp_err_t mpu6050_read_raw_data(mpu6050_sensor_t* sensor, int16_t* accel, int16_t* gyro) {
    uint8_t raw_data[14];
    esp_err_t ret = mpu6050_read_register(sensor, MPU6050_ACCEL_XOUT_H, raw_data, 14);
    
    if (ret == ESP_OK) {
        accel[0] = (int16_t)((raw_data[0] << 8) | raw_data[1]);   // X
        accel[1] = (int16_t)((raw_data[2] << 8) | raw_data[3]);   // Y
        accel[2] = (int16_t)((raw_data[4] << 8) | raw_data[5]);   // Z
        
        gyro[0] = (int16_t)((raw_data[8] << 8) | raw_data[9]);    // X
        gyro[1] = (int16_t)((raw_data[10] << 8) | raw_data[11]);  // Y
        gyro[2] = (int16_t)((raw_data[12] << 8) | raw_data[13]);  // Z
    }
    
    return ret;
}

esp_err_t mpu6050_sensor_update(mpu6050_sensor_t* sensor) {
    if (!sensor->data.initialized) return ESP_FAIL;
    
    int16_t accel_raw[3], gyro_raw[3];
    esp_err_t ret = mpu6050_read_raw_data(sensor, accel_raw, gyro_raw);
    
    if (ret == ESP_OK) {
        // Convert to g and degrees/second
        sensor->data.accel_x = accel_raw[0] / 16384.0f;
        sensor->data.accel_y = accel_raw[1] / 16384.0f;
        sensor->data.accel_z = accel_raw[2] / 16384.0f;
        
        sensor->data.gyro_x = gyro_raw[0] / 131.0f;
        sensor->data.gyro_y = gyro_raw[1] / 131.0f;
        sensor->data.gyro_z = gyro_raw[2] / 131.0f;
        
        // Calculate pitch and roll
        sensor->data.pitch = atan2(sensor->data.accel_y, sqrt(sensor->data.accel_x * sensor->data.accel_x + sensor->data.accel_z * sensor->data.accel_z)) * 180.0f / M_PI;
        sensor->data.roll = atan2(-sensor->data.accel_x, sensor->data.accel_z) * 180.0f / M_PI;
    }
    
    return ret;
}

float mpu6050_sensor_get_pitch(mpu6050_sensor_t* sensor) {
    return sensor->data.pitch;
}

float mpu6050_sensor_get_roll(mpu6050_sensor_t* sensor) {
    return sensor->data.roll;
}

float mpu6050_sensor_get_gyro_x(mpu6050_sensor_t* sensor) {
    return sensor->data.gyro_x;
}

float mpu6050_sensor_get_gyro_y(mpu6050_sensor_t* sensor) {
    return sensor->data.gyro_y;
}

float mpu6050_sensor_get_gyro_z(mpu6050_sensor_t* sensor) {
    return sensor->data.gyro_z;
}

float mpu6050_sensor_get_accel_x(mpu6050_sensor_t* sensor) {
    return sensor->data.accel_x;
}

float mpu6050_sensor_get_accel_y(mpu6050_sensor_t* sensor) {
    return sensor->data.accel_y;
}

float mpu6050_sensor_get_accel_z(mpu6050_sensor_t* sensor) {
    return sensor->data.accel_z;
}

bool mpu6050_sensor_is_ready(mpu6050_sensor_t* sensor) {
    return sensor->data.initialized;
}