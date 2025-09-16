#include "ble_controller.h"
#ifndef NATIVE_BUILD
#include "esp_log.h"
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char* TAG = "BLE_CONTROLLER";

static ble_controller_t* ble_instance = NULL;

// Simplified BLE implementation for basic functionality
esp_err_t ble_controller_init(ble_controller_t* ble, const char* device_name) {
    ble_instance = ble;
    
    ble->device_connected = false;
    ble->current_command.direction = 0;
    ble->current_command.turn = 0;
    ble->current_command.speed = 0;
    ble->current_command.balance = true;
    ble->current_command.standup = false;
    ble->gatts_if = 0;
    ble->conn_id = 0;
    ble->command_handle = 0;
    ble->status_handle = 0;
    memset(ble->last_command, 0, sizeof(ble->last_command));

    // Initialize Bluetooth controller
#ifndef NATIVE_BUILD
    esp_err_t ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth controller release classic bt memory failed: %s", esp_err_to_name(ret));
        return ret;
    }

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth controller initialize failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth controller enable failed: %s", esp_err_to_name(ret));
        return ret;
    }
#else
    esp_err_t ret = ESP_OK;
#endif

#ifndef NATIVE_BUILD
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth enable failed: %s", esp_err_to_name(ret));
        return ret;
    }
#endif

    ESP_LOGI(TAG, "BLE Controller initialized (basic mode)");
    return ESP_OK;
}

void ble_controller_update(ble_controller_t* ble) {
    // Placeholder for BLE updates
}

remote_command_t ble_controller_get_command(ble_controller_t* ble) {
    return ble->current_command;
}

bool ble_controller_is_connected(ble_controller_t* ble) {
    return ble->device_connected;
}

esp_err_t ble_controller_send_status(ble_controller_t* ble, const char* status) {
    if (!ble->device_connected) {
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Status: %s", status);
    return ESP_OK;
}

void ble_controller_parse_command(ble_controller_t* ble, const char* command) {
    if (strncmp(command, "MOVE:", 5) == 0) {
        int dir, turn, speed;
        if (sscanf(command + 5, "%d,%d,%d", &dir, &turn, &speed) == 3) {
            ble->current_command.direction = (dir > 1) ? 1 : ((dir < -1) ? -1 : dir);
            ble->current_command.turn = (turn > 100) ? 100 : ((turn < -100) ? -100 : turn);
            ble->current_command.speed = (speed > 100) ? 100 : ((speed < 0) ? 0 : speed);
        }
    } else if (strcmp(command, "STOP") == 0) {
        ble->current_command.direction = 0;
        ble->current_command.turn = 0;
        ble->current_command.speed = 0;
    } else if (strcmp(command, "BALANCE_ON") == 0) {
        ble->current_command.balance = true;
    } else if (strcmp(command, "BALANCE_OFF") == 0) {
        ble->current_command.balance = false;
    } else if (strcmp(command, "STANDUP") == 0) {
        ble->current_command.standup = true;
    } else if (strcmp(command, "STANDUP_DONE") == 0) {
        ble->current_command.standup = false;
    }
    
    strncpy(ble->last_command, command, sizeof(ble->last_command) - 1);
    ble->last_command[sizeof(ble->last_command) - 1] = '\0';
}