#pragma once
// Desktop stub for driver/i2c.h

#include "esp_err.h"
#include "driver/gpio.h"
#include "../freertos/FreeRTOS.h"
#include <stdint.h>
#include <stddef.h>

typedef int i2c_port_t;
typedef int i2c_mode_t;
typedef uint8_t i2c_ack_type_t;
typedef void*  i2c_cmd_handle_t;

#define I2C_NUM_0          0
#define I2C_NUM_1          1
#define I2C_MODE_MASTER    1
#define I2C_MODE_SLAVE     0
#define I2C_MASTER_WRITE   0
#define I2C_MASTER_READ    1
#define I2C_MASTER_ACK     0
#define I2C_MASTER_NACK    1
#define I2C_MASTER_LAST_NACK 2

typedef struct {
    i2c_mode_t mode;
    gpio_num_t sda_io_num;
    gpio_num_t scl_io_num;
    bool       sda_pullup_en;
    bool       scl_pullup_en;
    union {
        struct { uint32_t clk_speed; } master;
        struct { uint8_t addr_10bit_en; uint16_t slave_addr; uint32_t maximum_speed; } slave;
    };
    int clk_flags;
} i2c_config_t;

inline esp_err_t       i2c_param_config(i2c_port_t, const i2c_config_t*)          { return ESP_OK; }
inline esp_err_t       i2c_driver_install(i2c_port_t, i2c_mode_t, size_t, size_t, int) { return ESP_OK; }
inline esp_err_t       i2c_driver_delete(i2c_port_t)                               { return ESP_OK; }
inline i2c_cmd_handle_t i2c_cmd_link_create()                                      { return nullptr; }
inline esp_err_t       i2c_master_start(i2c_cmd_handle_t)                          { return ESP_OK; }
inline esp_err_t       i2c_master_write_byte(i2c_cmd_handle_t, uint8_t, bool)      { return ESP_OK; }
inline esp_err_t       i2c_master_write(i2c_cmd_handle_t, const uint8_t*, size_t, bool) { return ESP_OK; }
inline esp_err_t       i2c_master_read(i2c_cmd_handle_t, uint8_t*, size_t, i2c_ack_type_t) { return ESP_OK; }
inline esp_err_t       i2c_master_read_byte(i2c_cmd_handle_t, uint8_t*, i2c_ack_type_t) { return ESP_OK; }
inline esp_err_t       i2c_master_stop(i2c_cmd_handle_t)                           { return ESP_OK; }
inline esp_err_t       i2c_master_cmd_begin(i2c_port_t, i2c_cmd_handle_t, uint32_t) { return ESP_OK; }
inline void            i2c_cmd_link_delete(i2c_cmd_handle_t)                       {}
