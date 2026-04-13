#pragma once
// Desktop stub for esp_err.h

#include <stdint.h>

typedef int esp_err_t;

#define ESP_OK              0
#define ESP_FAIL           -1
#define ESP_ERR_INVALID_ARG -2
#define ESP_ERR_TIMEOUT    -3
#define ESP_ERR_NOT_FOUND  -4
#define ESP_ERR_NO_MEM     -5

#define ESP_ERROR_CHECK(x) (x)
