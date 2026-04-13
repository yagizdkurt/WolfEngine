#pragma once
// Desktop stub for freertos/semphr.h

#include "FreeRTOS.h"
#include <stddef.h>

typedef void* SemaphoreHandle_t;
typedef void* QueueHandle_t;

inline SemaphoreHandle_t xSemaphoreCreateBinary()               { return nullptr; }
inline SemaphoreHandle_t xSemaphoreCreateMutex()                { return nullptr; }
inline void              vSemaphoreDelete(SemaphoreHandle_t)    {}
inline BaseType_t        xSemaphoreGive(SemaphoreHandle_t)      { return pdTRUE;  }
inline BaseType_t        xSemaphoreGiveFromISR(SemaphoreHandle_t, BaseType_t* pw) {
    if (pw) *pw = pdFALSE;
    return pdTRUE;
}
inline BaseType_t xSemaphoreTake(SemaphoreHandle_t, TickType_t) { return pdTRUE;  }
