#ifndef FREERTOS_TASK_H
#define FREERTOS_TASK_H

#ifdef NATIVE_BUILD
// Mock FreeRTOS task.h for native testing
#include <stdint.h>

// Mock task functions
#define vTaskDelay(x)

#else
// Include real FreeRTOS task header for ESP32
#include_next "freertos/task.h"
#endif

#endif