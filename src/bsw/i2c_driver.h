#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

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

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t i2c_driver_init(i2c_port_t port, gpio_num_t sda_pin, gpio_num_t scl_pin);
esp_err_t i2c_write_register(i2c_port_t port, uint8_t device_addr, uint8_t reg_addr, uint8_t value);
esp_err_t i2c_read_register(i2c_port_t port, uint8_t device_addr, uint8_t reg_addr, uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif

#endif // I2C_DRIVER_H