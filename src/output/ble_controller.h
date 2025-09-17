#ifndef BLE_CONTROLLER_H
#define BLE_CONTROLLER_H

#ifndef NATIVE_BUILD
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_err.h"
#else
// Native build - types defined in test file
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#endif
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_SERVICE_UUID        0x00FF
#define BLE_COMMAND_CHAR_UUID   0xFF01
#define BLE_STATUS_CHAR_UUID    0xFF02

typedef struct {
    int direction;    // 0: stop, 1: forward, -1: backward
    int turn;         // -100 to 100 (left to right)
    int speed;        // 0 to 100
    bool balance;     // enable/disable balancing
    bool standup;     // command to stand up
} remote_command_t;

typedef struct {
    bool device_connected;
    remote_command_t current_command;
    char last_command[64];
    uint16_t gatts_if;
    uint16_t conn_id;
    uint16_t command_handle;
    uint16_t status_handle;
} ble_controller_t;

esp_err_t ble_controller_init(ble_controller_t* ble, const char* device_name);
void ble_controller_update(ble_controller_t* ble);
remote_command_t ble_controller_get_command(ble_controller_t* ble);
bool ble_controller_is_connected(ble_controller_t* ble);
esp_err_t ble_controller_send_status(ble_controller_t* ble, float angle, float velocity, float battery_voltage);
esp_err_t ble_controller_process_packet(ble_controller_t* ble, const uint8_t* data, size_t length);
void ble_controller_parse_command(ble_controller_t* ble, const char* command); // Legacy function

#ifdef __cplusplus
}
#endif

#endif