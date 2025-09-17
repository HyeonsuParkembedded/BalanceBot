#ifndef ERROR_RECOVERY_H
#define ERROR_RECOVERY_H

#ifndef NATIVE_BUILD
#include "esp_err.h"
#else
// Native build - define ESP types
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#endif
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    COMPONENT_CRITICAL,    // System must halt if this fails
    COMPONENT_IMPORTANT,   // Continue with limited functionality
    COMPONENT_OPTIONAL     // Can be ignored if fails
} component_priority_t;

typedef esp_err_t (*component_init_func_t)(void);

typedef struct {
    const char* name;
    component_init_func_t init_func;
    component_priority_t priority;
    bool initialized;
    int retry_count;
} component_info_t;

esp_err_t error_recovery_init(void);
bool initialize_component_with_retry(component_info_t* component);
void handle_component_failure(component_info_t* component);
bool is_component_operational(const char* name);
void enter_safe_mode(void);
void log_system_health(void);

#ifdef __cplusplus
}
#endif

#endif // ERROR_RECOVERY_H