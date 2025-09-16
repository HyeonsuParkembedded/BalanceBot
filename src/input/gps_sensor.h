#ifndef GPS_SENSOR_H
#define GPS_SENSOR_H

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

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double latitude;
    double longitude;
    float altitude;
    int satellites;
    bool fix_valid;
    bool initialized;
} gps_data_t;

typedef struct {
    uart_port_t uart_port;
    gps_data_t data;
} gps_sensor_t;

esp_err_t gps_sensor_init(gps_sensor_t* gps, uart_port_t port, gpio_num_t tx_pin, gpio_num_t rx_pin, int baudrate);
esp_err_t gps_sensor_update(gps_sensor_t* gps);
double gps_sensor_get_latitude(gps_sensor_t* gps);
double gps_sensor_get_longitude(gps_sensor_t* gps);
float gps_sensor_get_altitude(gps_sensor_t* gps);
int gps_sensor_get_satellites(gps_sensor_t* gps);
bool gps_sensor_has_fix(gps_sensor_t* gps);
bool gps_sensor_is_initialized(gps_sensor_t* gps);

#ifdef __cplusplus
}
#endif

#endif // GPS_SENSOR_H