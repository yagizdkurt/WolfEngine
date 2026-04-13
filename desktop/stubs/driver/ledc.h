#pragma once
// Desktop stub for driver/ledc.h

#include "esp_err.h"
#include <stdint.h>

typedef enum { LEDC_LOW_SPEED_MODE = 0, LEDC_HIGH_SPEED_MODE = 1 } ledc_mode_t;
typedef enum { LEDC_CHANNEL_0 = 0, LEDC_CHANNEL_1, LEDC_CHANNEL_2, LEDC_CHANNEL_3,
               LEDC_CHANNEL_4, LEDC_CHANNEL_5, LEDC_CHANNEL_6, LEDC_CHANNEL_7 } ledc_channel_t;
typedef enum { LEDC_TIMER_0 = 0, LEDC_TIMER_1, LEDC_TIMER_2, LEDC_TIMER_3 } ledc_timer_t;
typedef enum { LEDC_TIMER_1_BIT = 1, LEDC_TIMER_8_BIT = 8, LEDC_TIMER_10_BIT = 10,
               LEDC_TIMER_12_BIT = 12, LEDC_TIMER_13_BIT = 13 } ledc_timer_bit_t;
typedef enum { LEDC_AUTO_CLK = 0, LEDC_APB_CLK = 1, LEDC_REF_TICK = 2 } ledc_clk_cfg_t;
typedef enum { LEDC_INTR_DISABLE = 0, LEDC_INTR_FADE_END = 1 } ledc_intr_type_t;

typedef struct {
    ledc_mode_t     speed_mode;
    ledc_timer_bit_t duty_resolution;
    ledc_timer_t    timer_num;
    uint32_t        freq_hz;
    ledc_clk_cfg_t  clk_cfg;
} ledc_timer_config_t;

typedef struct {
    int              gpio_num;
    ledc_mode_t      speed_mode;
    ledc_channel_t   channel;
    ledc_intr_type_t intr_type;
    ledc_timer_t     timer_sel;
    uint32_t         duty;
    int              hpoint;
} ledc_channel_config_t;

inline esp_err_t ledc_timer_config(const ledc_timer_config_t*)              { return ESP_OK; }
inline esp_err_t ledc_channel_config(const ledc_channel_config_t*)          { return ESP_OK; }
inline esp_err_t ledc_set_freq(ledc_mode_t, ledc_timer_t, uint32_t)         { return ESP_OK; }
inline esp_err_t ledc_set_duty(ledc_mode_t, ledc_channel_t, uint32_t)       { return ESP_OK; }
inline esp_err_t ledc_update_duty(ledc_mode_t, ledc_channel_t)              { return ESP_OK; }
inline esp_err_t ledc_stop(ledc_mode_t, ledc_channel_t, uint32_t)           { return ESP_OK; }
inline uint32_t  ledc_get_duty(ledc_mode_t, ledc_channel_t)                 { return 0; }
