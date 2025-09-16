#ifndef FREERTOS_H
#define FREERTOS_H

#ifdef NATIVE_BUILD
// Mock FreeRTOS for native testing
#include <stdint.h>

#define portTICK_PERIOD_MS 1

typedef uint32_t TickType_t;

uint32_t xTaskGetTickCount(void);

#else
// Include real FreeRTOS header for ESP32
#include_next "freertos/FreeRTOS.h"
#endif

#endif