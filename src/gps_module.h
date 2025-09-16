#ifndef GPS_MODULE_H
#define GPS_MODULE_H

#ifndef NATIVE_BUILD
#include "driver/uart.h"
#include "esp_err.h"
#else
typedef int esp_err_t;
typedef int uart_port_t;
#define ESP_OK 0
#define ESP_FAIL -1
#endif
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool valid;
    float latitude;
    float longitude;
    float altitude;
    float speed;
    int satellites;
    float hdop;
} gps_data_t;

typedef struct {
    uart_port_t uart_port;
    gps_data_t data;
    char nmea_buffer[256];
    int buffer_index;
} gps_module_t;

esp_err_t gps_module_init(gps_module_t* gps, uart_port_t port, int tx_pin, int rx_pin, int baudrate);
void gps_module_update(gps_module_t* gps);
bool gps_module_is_valid(gps_module_t* gps);
gps_data_t gps_module_get_data(gps_module_t* gps);
float gps_module_get_latitude(gps_module_t* gps);
float gps_module_get_longitude(gps_module_t* gps);
float gps_module_get_altitude(gps_module_t* gps);
float gps_module_get_speed(gps_module_t* gps);
int gps_module_get_satellites(gps_module_t* gps);

#ifdef __cplusplus
}
#endif

#endif