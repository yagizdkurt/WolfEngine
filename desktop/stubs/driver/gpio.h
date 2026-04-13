#pragma once
// Desktop stub for driver/gpio.h

#include "esp_err.h"
#include <stdint.h>

typedef int gpio_num_t;

typedef enum {
    GPIO_MODE_INPUT         = 1,
    GPIO_MODE_OUTPUT        = 2,
    GPIO_MODE_INPUT_OUTPUT  = 3,
} gpio_mode_t;

typedef enum {
    GPIO_PULLUP_DISABLE = 0,
    GPIO_PULLUP_ENABLE  = 1,
} gpio_pullup_t;

typedef enum {
    GPIO_PULLDOWN_DISABLE = 0,
    GPIO_PULLDOWN_ENABLE  = 1,
} gpio_pulldown_t;

typedef enum {
    GPIO_INTR_DISABLE   = 0,
    GPIO_INTR_POSEDGE   = 1,
    GPIO_INTR_NEGEDGE   = 2,
    GPIO_INTR_ANYEDGE   = 3,
    GPIO_INTR_LOW_LEVEL = 4,
    GPIO_INTR_HIGH_LEVEL= 5,
} gpio_int_type_t;

typedef struct {
    uint64_t       pin_bit_mask;
    gpio_mode_t    mode;
    gpio_pullup_t  pull_up_en;
    gpio_pulldown_t pull_down_en;
    gpio_int_type_t intr_type;
} gpio_config_t;

inline esp_err_t gpio_config(const gpio_config_t*) { return ESP_OK; }
inline int       gpio_get_level(gpio_num_t)         { return 0; }
inline esp_err_t gpio_set_level(gpio_num_t, uint32_t) { return ESP_OK; }
inline esp_err_t gpio_set_direction(gpio_num_t, gpio_mode_t) { return ESP_OK; }
inline esp_err_t gpio_set_pull_mode(gpio_num_t, int) { return ESP_OK; }
