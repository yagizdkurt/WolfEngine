#pragma once
// Desktop stub for esp_adc/adc_oneshot.h

#include "esp_err.h"
#include <stdint.h>

typedef enum {
    ADC_CHANNEL_0 = 0,
    ADC_CHANNEL_1,
    ADC_CHANNEL_2,
    ADC_CHANNEL_3,
    ADC_CHANNEL_4,
    ADC_CHANNEL_5,
    ADC_CHANNEL_6,
    ADC_CHANNEL_7,
    ADC_CHANNEL_8,
    ADC_CHANNEL_9,
} adc_channel_t;

typedef enum {
    ADC_UNIT_1 = 0,
    ADC_UNIT_2,
} adc_unit_t;

typedef enum {
    ADC_BITWIDTH_DEFAULT = 0,
    ADC_BITWIDTH_9  = 9,
    ADC_BITWIDTH_10 = 10,
    ADC_BITWIDTH_11 = 11,
    ADC_BITWIDTH_12 = 12,
    ADC_BITWIDTH_13 = 13,
} adc_bitwidth_t;

typedef enum {
    ADC_ATTEN_DB_0   = 0,
    ADC_ATTEN_DB_2_5 = 1,
    ADC_ATTEN_DB_6   = 2,
    ADC_ATTEN_DB_12  = 3,
    ADC_ATTEN_DB_11  = 3,  // alias
} adc_atten_t;

typedef void* adc_oneshot_unit_handle_t;

typedef struct {
    adc_unit_t unit_id;
    int        ulp_mode;
} adc_oneshot_unit_init_cfg_t;

typedef struct {
    adc_atten_t    atten;
    adc_bitwidth_t bitwidth;
} adc_oneshot_chan_cfg_t;

inline esp_err_t adc_oneshot_new_unit(const adc_oneshot_unit_init_cfg_t*, adc_oneshot_unit_handle_t* out) {
    if (out) *out = nullptr;
    return ESP_OK;
}
inline esp_err_t adc_oneshot_config_channel(adc_oneshot_unit_handle_t, adc_channel_t, const adc_oneshot_chan_cfg_t*) {
    return ESP_OK;
}
inline esp_err_t adc_oneshot_read(adc_oneshot_unit_handle_t, adc_channel_t, int* out) {
    if (out) *out = 2048;  // mid-range neutral value
    return ESP_OK;
}
inline esp_err_t adc_oneshot_del_unit(adc_oneshot_unit_handle_t) { return ESP_OK; }
