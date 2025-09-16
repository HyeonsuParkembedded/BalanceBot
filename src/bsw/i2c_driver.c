#include "i2c_driver.h"
#ifndef NATIVE_BUILD
#include "esp_log.h"
#endif

#ifndef NATIVE_BUILD
static const char* I2C_TAG = "I2C_DRIVER";
#else
#define I2C_TAG "I2C_DRIVER"
#endif

esp_err_t i2c_driver_init(i2c_port_t port, gpio_num_t sda_pin, gpio_num_t scl_pin) {
#ifndef NATIVE_BUILD
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = scl_pin,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000, // 400kHz
    };

    esp_err_t ret = i2c_param_config(port, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(I2C_TAG, "I2C param config failed");
        return ret;
    }

    ret = i2c_driver_install(port, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(I2C_TAG, "I2C driver install failed");
        return ret;
    }

    ESP_LOGI(I2C_TAG, "I2C driver initialized");
#endif
    return ESP_OK;
}

esp_err_t i2c_write_register(i2c_port_t port, uint8_t device_addr, uint8_t reg_addr, uint8_t value) {
#ifndef NATIVE_BUILD
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
#else
    return ESP_OK;
#endif
}

esp_err_t i2c_read_register(i2c_port_t port, uint8_t device_addr, uint8_t reg_addr, uint8_t* data, size_t len) {
#ifndef NATIVE_BUILD
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_READ, true);

    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
#else
    // Mock data for native build
    for (size_t i = 0; i < len; i++) {
        data[i] = 0x42 + i;
    }
    return ESP_OK;
#endif
}