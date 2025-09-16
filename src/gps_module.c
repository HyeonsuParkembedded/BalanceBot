#include "gps_module.h"
#ifndef NATIVE_BUILD
#include "esp_log.h"
#endif
#include <string.h>
#include <stdlib.h>

#ifndef NATIVE_BUILD
static const char* TAG = "GPS";
#else
#define TAG "GPS"
#endif

static float convert_deg_min_to_dec_deg(float deg_min);
static bool parse_nmea(gps_module_t* gps, const char* sentence);
static bool parse_gpgga(gps_module_t* gps, const char* sentence);
static bool parse_gprmc(gps_module_t* gps, const char* sentence);

esp_err_t gps_module_init(gps_module_t* gps, uart_port_t port, int tx_pin, int rx_pin, int baudrate) {
    gps->uart_port = port;
    gps->data.valid = false;
    gps->data.latitude = 0.0f;
    gps->data.longitude = 0.0f;
    gps->data.altitude = 0.0f;
    gps->data.speed = 0.0f;
    gps->data.satellites = 0;
    gps->data.hdop = 0.0f;
    gps->buffer_index = 0;
    memset(gps->nmea_buffer, 0, sizeof(gps->nmea_buffer));

    uart_config_t uart_config = {
        .baud_rate = baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    esp_err_t ret = uart_driver_install(port, 1024, 1024, 0, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART driver install failed");
        return ret;
    }

    ret = uart_param_config(port, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART param config failed");
        return ret;
    }

    ret = uart_set_pin(port, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART set pin failed");
        return ret;
    }

    ESP_LOGI(TAG, "GPS module initialized");
    return ESP_OK;
}

void gps_module_update(gps_module_t* gps) {
    uint8_t data[128];
    int length = uart_read_bytes(gps->uart_port, data, sizeof(data), 0);
    
    for (int i = 0; i < length; i++) {
        char c = (char)data[i];
        
        if (c == '\n') {
            if (gps->buffer_index > 0 && gps->nmea_buffer[0] == '$') {
                gps->nmea_buffer[gps->buffer_index] = '\0';
                parse_nmea(gps, gps->nmea_buffer);
            }
            gps->buffer_index = 0;
        } else if (c != '\r') {
            if (gps->buffer_index < sizeof(gps->nmea_buffer) - 1) {
                gps->nmea_buffer[gps->buffer_index++] = c;
            }
        }
    }
}

static bool parse_nmea(gps_module_t* gps, const char* sentence) {
    if (strncmp(sentence, "$GPGGA", 6) == 0 || strncmp(sentence, "$GNGGA", 6) == 0) {
        return parse_gpgga(gps, sentence);
    } else if (strncmp(sentence, "$GPRMC", 6) == 0 || strncmp(sentence, "$GNRMC", 6) == 0) {
        return parse_gprmc(gps, sentence);
    }
    return false;
}

static bool parse_gpgga(gps_module_t* gps, const char* sentence) {
    char* tokens[15];
    char sentence_copy[256];
    strncpy(sentence_copy, sentence, sizeof(sentence_copy) - 1);
    sentence_copy[sizeof(sentence_copy) - 1] = '\0';
    
    int token_count = 0;
    char* token = strtok(sentence_copy, ",");
    while (token != NULL && token_count < 15) {
        tokens[token_count++] = token;
        token = strtok(NULL, ",");
    }
    
    if (token_count < 14) return false;
    
    // Fix quality (6th field)
    int fix_quality = atoi(tokens[6]);
    if (fix_quality == 0) {
        gps->data.valid = false;
        return false;
    }
    
    // Latitude (2nd field) and direction (3rd field)
    if (strlen(tokens[2]) > 0 && strlen(tokens[3]) > 0) {
        float lat = atof(tokens[2]);
        gps->data.latitude = convert_deg_min_to_dec_deg(lat);
        if (tokens[3][0] == 'S') {
            gps->data.latitude = -gps->data.latitude;
        }
    }
    
    // Longitude (4th field) and direction (5th field)
    if (strlen(tokens[4]) > 0 && strlen(tokens[5]) > 0) {
        float lon = atof(tokens[4]);
        gps->data.longitude = convert_deg_min_to_dec_deg(lon);
        if (tokens[5][0] == 'W') {
            gps->data.longitude = -gps->data.longitude;
        }
    }
    
    // Satellites (7th field)
    if (strlen(tokens[7]) > 0) {
        gps->data.satellites = atoi(tokens[7]);
    }
    
    // HDOP (8th field)
    if (strlen(tokens[8]) > 0) {
        gps->data.hdop = atof(tokens[8]);
    }
    
    // Altitude (9th field)
    if (strlen(tokens[9]) > 0) {
        gps->data.altitude = atof(tokens[9]);
    }
    
    gps->data.valid = true;
    return true;
}

static bool parse_gprmc(gps_module_t* gps, const char* sentence) {
    char* tokens[12];
    char sentence_copy[256];
    strncpy(sentence_copy, sentence, sizeof(sentence_copy) - 1);
    sentence_copy[sizeof(sentence_copy) - 1] = '\0';
    
    int token_count = 0;
    char* token = strtok(sentence_copy, ",");
    while (token != NULL && token_count < 12) {
        tokens[token_count++] = token;
        token = strtok(NULL, ",");
    }
    
    if (token_count < 11) return false;
    
    // Status (2nd field)
    if (tokens[2][0] != 'A') return false;
    
    // Speed (7th field) - convert from knots to km/h
    if (strlen(tokens[7]) > 0) {
        gps->data.speed = atof(tokens[7]) * 1.852f;
    }
    
    return true;
}

static float convert_deg_min_to_dec_deg(float deg_min) {
    int degrees = (int)(deg_min / 100);
    float minutes = deg_min - (degrees * 100);
    return degrees + (minutes / 60.0f);
}

bool gps_module_is_valid(gps_module_t* gps) {
    return gps->data.valid;
}

gps_data_t gps_module_get_data(gps_module_t* gps) {
    return gps->data;
}

float gps_module_get_latitude(gps_module_t* gps) {
    return gps->data.latitude;
}

float gps_module_get_longitude(gps_module_t* gps) {
    return gps->data.longitude;
}

float gps_module_get_altitude(gps_module_t* gps) {
    return gps->data.altitude;
}

float gps_module_get_speed(gps_module_t* gps) {
    return gps->data.speed;
}

int gps_module_get_satellites(gps_module_t* gps) {
    return gps->data.satellites;
}