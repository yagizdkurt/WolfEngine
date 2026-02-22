#pragma once
#include "WE_I2C.hpp"
#include <string.h>
// ─────────────────────────────────────────────────────────────
//  EEPROM24LC512 — 512 Kbit (64 KB) I2C EEPROM
//
//  Address: 0x50–0x57  (A2 A1 A0 pins)
//  Page size: 128 bytes
//  Write cycle time: 5 ms (poll ACK to detect completion)
// ─────────────────────────────────────────────────────────────

class EEPROM24LC512 {
public:
    static constexpr uint16_t PAGE_SIZE   = 128;
    static constexpr uint16_t TOTAL_BYTES = 65536;  // 64 KB
    static constexpr uint32_t WRITE_CYCLE_MS = 5;

    explicit EEPROM24LC512(uint8_t addr = 0x50) : _addr(addr) {}

    // ── Single byte ───────────────────────────────────────────

    esp_err_t writeByte(uint16_t memAddr, uint8_t value) {
        return writeBytes(memAddr, &value, 1);
    }

    esp_err_t readByte(uint16_t memAddr, uint8_t& value) {
        return readBytes(memAddr, &value, 1);
    }

    // ── Burst read (unlimited length) ─────────────────────────

    esp_err_t readBytes(uint16_t memAddr, uint8_t* buf, size_t len) {
        if (memAddr + len > TOTAL_BYTES) return ESP_ERR_INVALID_ARG;

        // Send 16-bit memory address
        uint8_t addrBuf[2] = { uint8_t(memAddr >> 8), uint8_t(memAddr & 0xFF) };

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (_addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, addrBuf, 2, true);
        i2c_master_start(cmd);  // Repeated start
        i2c_master_write_byte(cmd, (_addr << 1) | I2C_MASTER_READ, true);
        if (len > 1)
            i2c_master_read(cmd, buf, len - 1, I2C_MASTER_ACK);
        i2c_master_read_byte(cmd, buf + len - 1, I2C_MASTER_NACK);
        i2c_master_stop(cmd);
        esp_err_t err = i2c_master_cmd_begin(I2CManager::PORT, cmd, I2CManager::TIMEOUT);
        i2c_cmd_link_delete(cmd);
        return err;
    }

    // ── Page-aware burst write ─────────────────────────────────
    //  Automatically splits across 128-byte page boundaries.

    esp_err_t writeBytes(uint16_t memAddr, const uint8_t* buf, size_t len) {
        if (memAddr + len > TOTAL_BYTES) return ESP_ERR_INVALID_ARG;

        size_t written = 0;
        while (written < len) {
            uint16_t currentAddr = memAddr + written;
            // How many bytes fit in the current page from this offset?
            uint16_t pageOffset  = currentAddr % PAGE_SIZE;
            size_t   chunk       = PAGE_SIZE - pageOffset;
            if (chunk > len - written) chunk = len - written;

            esp_err_t err = writePage(currentAddr, buf + written, chunk);
            if (err != ESP_OK) return err;

            written += chunk;
        }
        return ESP_OK;
    }

    // ── Erase (fill with 0xFF) ────────────────────────────────

    esp_err_t eraseAll() {
        uint8_t blank[PAGE_SIZE];
        memset(blank, 0xFF, PAGE_SIZE);
        for (uint16_t addr = 0; addr < TOTAL_BYTES; addr += PAGE_SIZE) {
            esp_err_t err = writeBytes(addr, blank, PAGE_SIZE);
            if (err != ESP_OK) return err;
        }
        return ESP_OK;
    }

private:
    uint8_t _addr;

    // Write up to PAGE_SIZE bytes within a single page; polls for ACK
    esp_err_t writePage(uint16_t memAddr, const uint8_t* buf, size_t len) {
        uint8_t addrBuf[2] = { uint8_t(memAddr >> 8), uint8_t(memAddr & 0xFF) };

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (_addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, addrBuf, 2, true);
        i2c_master_write(cmd, buf, len, true);
        i2c_master_stop(cmd);
        esp_err_t err = i2c_master_cmd_begin(I2CManager::PORT, cmd, I2CManager::TIMEOUT);
        i2c_cmd_link_delete(cmd);
        if (err != ESP_OK) return err;

        return pollAck();
    }

    // Poll ACK to detect write-cycle completion (up to 10 ms)
    esp_err_t pollAck() {
        constexpr int MAX_POLLS = 20;
        for (int i = 0; i < MAX_POLLS; i++) {
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (_addr << 1) | I2C_MASTER_WRITE, true);
            i2c_master_stop(cmd);
            esp_err_t err = i2c_master_cmd_begin(I2CManager::PORT, cmd, I2CManager::TIMEOUT);
            i2c_cmd_link_delete(cmd);
            if (err == ESP_OK) return ESP_OK;
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        return ESP_ERR_TIMEOUT;
    }
};