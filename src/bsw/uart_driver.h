#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#ifndef NATIVE_BUILD
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_err.h"
#else
typedef int esp_err_t;
typedef int uart_port_t;
typedef int gpio_num_t;
#define ESP_OK 0
#define ESP_FAIL -1
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t uart_driver_init(uart_port_t port, gpio_num_t tx_pin, gpio_num_t rx_pin, int baudrate);
int uart_read_data(uart_port_t port, uint8_t* data, size_t max_len, uint32_t timeout_ms);
esp_err_t uart_write_data(uart_port_t port, const uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif

#endif // UART_DRIVER_H