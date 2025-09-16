#include "uart_driver.h"
#ifndef NATIVE_BUILD
#include "esp_log.h"
#endif

#ifndef NATIVE_BUILD
static const char* UART_TAG = "UART_DRIVER";
#else
#define UART_TAG "UART_DRIVER"
#endif

esp_err_t uart_driver_init(uart_port_t port, gpio_num_t tx_pin, gpio_num_t rx_pin, int baudrate) {
#ifndef NATIVE_BUILD
    uart_config_t uart_config = {
        .baud_rate = baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    esp_err_t ret = uart_driver_install(port, 1024, 0, 0, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(UART_TAG, "UART driver install failed");
        return ret;
    }

    ret = uart_param_config(port, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(UART_TAG, "UART param config failed");
        return ret;
    }

    ret = uart_set_pin(port, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(UART_TAG, "UART set pin failed");
        return ret;
    }

    ESP_LOGI(UART_TAG, "UART driver initialized");
#endif
    return ESP_OK;
}

int uart_read_data(uart_port_t port, uint8_t* data, size_t max_len, uint32_t timeout_ms) {
#ifndef NATIVE_BUILD
    return uart_read_bytes(port, data, max_len, timeout_ms / portTICK_PERIOD_MS);
#else
    // Mock GPS data for native build
    const char* mock_gps = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";
    int len = strlen(mock_gps);
    if (len > max_len) len = max_len;
    memcpy(data, mock_gps, len);
    return len;
#endif
}

esp_err_t uart_write_data(uart_port_t port, const uint8_t* data, size_t len) {
#ifndef NATIVE_BUILD
    int written = uart_write_bytes(port, (const char*)data, len);
    return (written == len) ? ESP_OK : ESP_FAIL;
#else
    return ESP_OK;
#endif
}