#include "gps_sensor.h"
#include "../bsw/uart_driver.h"
#ifndef NATIVE_BUILD
#include "esp_log.h"
#endif
#include <string.h>
#include <stdlib.h>

// Mock functions for native build
#ifndef NATIVE_BUILD
#else
char* strdup(const char* s) {
    size_t len = strlen(s) + 1;
    char* dup = malloc(len);
    if (dup) {
        memcpy(dup, s, len);
    }
    return dup;
}
#endif

#ifndef NATIVE_BUILD
static const char* GPS_TAG = "GPS_SENSOR";
#else
#define GPS_TAG "GPS_SENSOR"
#endif

static float convert_deg_min_to_dec_deg(float deg_min);
static bool parse_nmea(gps_sensor_t* gps, const char* sentence);
static bool parse_gpgga(gps_sensor_t* gps, const char* sentence);
static bool parse_gprmc(gps_sensor_t* gps, const char* sentence);

esp_err_t gps_sensor_init(gps_sensor_t* gps, uart_port_t port, gpio_num_t tx_pin, gpio_num_t rx_pin, int baudrate) {
    gps->uart_port = port;
    gps->data.latitude = 0.0;
    gps->data.longitude = 0.0;
    gps->data.altitude = 0.0f;
    gps->data.satellites = 0;
    gps->data.fix_valid = false;
    gps->data.initialized = false;

    esp_err_t ret = uart_driver_init(port, tx_pin, rx_pin, baudrate);
    if (ret != ESP_OK) {
        return ret;
    }

    gps->data.initialized = true;

#ifndef NATIVE_BUILD
    ESP_LOGI(GPS_TAG, "GPS sensor initialized");
#endif
    return ESP_OK;
}

esp_err_t gps_sensor_update(gps_sensor_t* gps) {
    if (!gps->data.initialized) {
        return ESP_FAIL;
    }

    uint8_t buffer[256];
    int len = uart_read_data(gps->uart_port, buffer, sizeof(buffer) - 1, 100);

    if (len > 0) {
        buffer[len] = '\0';
        char* sentence = strtok((char*)buffer, "\r\n");

        while (sentence != NULL) {
            if (parse_nmea(gps, sentence)) {
                return ESP_OK;
            }
            sentence = strtok(NULL, "\r\n");
        }
    }

    return ESP_OK;
}

double gps_sensor_get_latitude(gps_sensor_t* gps) {
    return gps->data.latitude;
}

double gps_sensor_get_longitude(gps_sensor_t* gps) {
    return gps->data.longitude;
}

float gps_sensor_get_altitude(gps_sensor_t* gps) {
    return gps->data.altitude;
}

int gps_sensor_get_satellites(gps_sensor_t* gps) {
    return gps->data.satellites;
}

bool gps_sensor_has_fix(gps_sensor_t* gps) {
    return gps->data.fix_valid;
}

bool gps_sensor_is_initialized(gps_sensor_t* gps) {
    return gps->data.initialized;
}

static float convert_deg_min_to_dec_deg(float deg_min) {
    int degrees = (int)(deg_min / 100);
    float minutes = deg_min - (degrees * 100);
    return degrees + (minutes / 60.0f);
}

static bool parse_nmea(gps_sensor_t* gps, const char* sentence) {
    if (strncmp(sentence, "$GPGGA", 6) == 0) {
        return parse_gpgga(gps, sentence);
    } else if (strncmp(sentence, "$GPRMC", 6) == 0) {
        return parse_gprmc(gps, sentence);
    }
    return false;
}

static bool parse_gpgga(gps_sensor_t* gps, const char* sentence) {
    char* copy = strdup(sentence);
    char* token = strtok(copy, ",");
    int field = 0;

    float lat_raw = 0, lon_raw = 0;
    char lat_dir = 'N', lon_dir = 'E';
    int quality = 0;

    while (token != NULL && field < 15) {
        switch (field) {
            case 2: lat_raw = atof(token); break;
            case 3: lat_dir = token[0]; break;
            case 4: lon_raw = atof(token); break;
            case 5: lon_dir = token[0]; break;
            case 6: quality = atoi(token); break;
            case 7: gps->data.satellites = atoi(token); break;
            case 9: gps->data.altitude = atof(token); break;
        }
        token = strtok(NULL, ",");
        field++;
    }

    if (quality > 0 && lat_raw != 0 && lon_raw != 0) {
        gps->data.latitude = convert_deg_min_to_dec_deg(lat_raw);
        gps->data.longitude = convert_deg_min_to_dec_deg(lon_raw);

        if (lat_dir == 'S') gps->data.latitude = -gps->data.latitude;
        if (lon_dir == 'W') gps->data.longitude = -gps->data.longitude;

        gps->data.fix_valid = true;
    } else {
        gps->data.fix_valid = false;
    }

    free(copy);
    return gps->data.fix_valid;
}

static bool parse_gprmc(gps_sensor_t* gps, const char* sentence) {
    // Simplified GPRMC parsing - mainly for fix validity
    char* copy = strdup(sentence);
    char* token = strtok(copy, ",");
    int field = 0;

    while (token != NULL && field < 3) {
        if (field == 2) {
            gps->data.fix_valid = (token[0] == 'A');
            break;
        }
        token = strtok(NULL, ",");
        field++;
    }

    free(copy);
    return false; // Don't update position from RMC
}