#include "error_recovery.h"
#include "config.h"
#include <string.h>
#ifndef NATIVE_BUILD
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#else
// Native build - mock ESP functions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ESP_LOGI(tag, format, ...) printf("[INFO] " tag ": " format "\n", ##__VA_ARGS__)
#define ESP_LOGW(tag, format, ...) printf("[WARN] " tag ": " format "\n", ##__VA_ARGS__)
#define ESP_LOGE(tag, format, ...) printf("[ERROR] " tag ": " format "\n", ##__VA_ARGS__)
#define esp_err_to_name(err) ((err == ESP_OK) ? "ESP_OK" : "ESP_FAIL")
#define vTaskDelay(ticks) // No-op for native
#define pdMS_TO_TICKS(ms) (ms)
#define esp_restart() exit(1)
#endif

static const char* TAG = "ERROR_RECOVERY";

static component_info_t system_components[10];
static int component_count = 0;
static bool safe_mode_active = false;

esp_err_t error_recovery_init(void) {
    ESP_LOGI(TAG, "Error recovery system initialized");
    return ESP_OK;
}

bool initialize_component_with_retry(component_info_t* component) {
    if (component == NULL || component->init_func == NULL) {
        ESP_LOGE(TAG, "Invalid component configuration");
        return false;
    }

    ESP_LOGI(TAG, "Initializing component: %s", component->name);
    
    for (int retry = 0; retry < CONFIG_MAX_INIT_RETRIES; retry++) {
        esp_err_t ret = component->init_func();
        if (ret == ESP_OK) {
            component->initialized = true;
            component->retry_count = retry;
            ESP_LOGI(TAG, "Component %s initialized successfully", component->name);
            
            // Add to system components list
            if (component_count < 10) {
                system_components[component_count++] = *component;
            }
            return true;
        }
        
        ESP_LOGW(TAG, "%s initialization failed (attempt %d/%d): %s", 
                component->name, retry + 1, CONFIG_MAX_INIT_RETRIES, esp_err_to_name(ret));
        
        if (retry < CONFIG_MAX_INIT_RETRIES - 1) {
            vTaskDelay(pdMS_TO_TICKS(CONFIG_ERROR_RECOVERY_DELAY));
        }
    }
    
    // All retries failed
    component->initialized = false;
    component->retry_count = CONFIG_MAX_INIT_RETRIES;
    handle_component_failure(component);
    
    return false;
}

void handle_component_failure(component_info_t* component) {
    ESP_LOGE(TAG, "Component %s failed after %d retries", 
            component->name, component->retry_count);
    
    switch (component->priority) {
    case COMPONENT_CRITICAL:
        ESP_LOGE(TAG, "Critical component %s failed - entering safe mode", component->name);
        enter_safe_mode();
        break;
        
    case COMPONENT_IMPORTANT:
        ESP_LOGW(TAG, "Important component %s failed - continuing with limited functionality", 
                component->name);
        break;
        
    case COMPONENT_OPTIONAL:
        ESP_LOGI(TAG, "Optional component %s failed - continuing normally", component->name);
        break;
    }
}

bool is_component_operational(const char* name) {
    for (int i = 0; i < component_count; i++) {
        if (strcmp(system_components[i].name, name) == 0) {
            return system_components[i].initialized;
        }
    }
    return false;
}

void enter_safe_mode(void) {
    safe_mode_active = true;
    ESP_LOGE(TAG, "ENTERING SAFE MODE - System will restart in %d seconds", 
            CONFIG_ERROR_RECOVERY_DELAY / 1000);
    
    // Log system status before restart
    log_system_health();
    
    // Wait before restart to allow log output
    vTaskDelay(pdMS_TO_TICKS(CONFIG_ERROR_RECOVERY_DELAY));
    
    ESP_LOGE(TAG, "Restarting system...");
    esp_restart();
}

void log_system_health(void) {
    ESP_LOGI(TAG, "=== SYSTEM HEALTH REPORT ===");
    ESP_LOGI(TAG, "Safe mode active: %s", safe_mode_active ? "YES" : "NO");
    ESP_LOGI(TAG, "Total components: %d", component_count);
    
    for (int i = 0; i < component_count; i++) {
        ESP_LOGI(TAG, "Component %s: %s (retries: %d, priority: %d)", 
                system_components[i].name,
                system_components[i].initialized ? "OK" : "FAILED",
                system_components[i].retry_count,
                system_components[i].priority);
    }
    ESP_LOGI(TAG, "========================");
}