#pragma once
#include <driver/i2c.h>
#include "WolfEngine/Settings/WE_PINDEFS.hpp"

// ─────────────────────────────────────────────────────────────
//  I2CManager — static singleton-style manager
//  Supports: PCF8574 (I/O expander) and 24LC512 (EEPROM)
//  Not for user code; provides low-level read/write primitives for drivers.
//
//  Access is intentionally restricted via friend declarations.
//  Only approved driver classes can call
//  the private primitives, preventing accidental raw bus access
//  from unrelated engine systems.
// ─────────────────────────────────────────────────────────────

class I2CManager {
public:
    // I2C peripheral port used by the engine. All driver communication
    // goes through this single port — do not instantiate a second master
    // on the same port elsewhere.
    static constexpr i2c_port_t PORT    = I2C_NUM_0;

    // SDA and SCL pin assignments sourced from the engine's central
    // pin definition header (WE_PINDEFS.hpp) so board-specific changes
    // only need to be made in one place.
    static constexpr gpio_num_t PIN_SDA = gpio_num_t(I2C_PIN_SDA);
    static constexpr gpio_num_t PIN_SCL = gpio_num_t(I2C_PIN_SCL);

    // Bus clock frequency. 400 kHz (fast-mode) is supported by both
    // the PCF8574 and 24LC512, and is the recommended default.
    // Drop to 100000 if you experience instability on long cable runs.
    static constexpr uint32_t   FREQ_HZ = 400000;

    // Maximum time any single bus transaction is allowed to take
    // before it is considered failed. 10 ms is generous enough to
    // handle clock-stretching devices while still catching hung buses.
    static constexpr uint32_t   TIMEOUT = pdMS_TO_TICKS(10);

private:
    // Installs the ESP-IDF I2C master driver and applies the pin/frequency
    // configuration defined above. Must be called once before any driver
    // attempts to communicate. Called internally by the engine's init sequence.
    static esp_err_t begin();

    // Uninstalls the I2C driver and releases the peripheral.
    // Call during engine shutdown or before reconfiguring the bus.
    static void end();

    // Sends `len` bytes from `data` to the device at 7-bit address `addr`.
    // Generates: START → address+W → data bytes → STOP.
    // Returns ESP_OK on ACK from device, ESP_FAIL or ESP_ERR_TIMEOUT otherwise.
    static esp_err_t write(uint8_t addr, const uint8_t* data, size_t len);

    // Reads `len` bytes from the device at 7-bit address `addr` into `data`.
    // Generates: START → address+R → data bytes (ACK each) → NACK → STOP.
    // The device must already have its internal pointer set (e.g. via a prior write).
    static esp_err_t read(uint8_t addr, uint8_t* data, size_t len);

    // Writes `len` bytes to a specific 8-bit register address on the device.
    // Generates: START → address+W → reg → data bytes → STOP.
    // Suitable for devices that use a single-byte register/command prefix.
    static esp_err_t writeReg(uint8_t addr, uint8_t reg, const uint8_t* data, size_t len);

    // Reads `len` bytes from a specific 8-bit register address on the device.
    // Generates: START → address+W → reg → REPEATED START → address+R → data → STOP.
    // The repeated start (vs stop+start) keeps bus ownership during the turnaround,
    // which is required by some devices to guarantee atomic register reads.
    static esp_err_t readReg(uint8_t addr, uint8_t reg, uint8_t* data, size_t len);

    // Scans all 127 valid 7-bit addresses and logs any device that ACKs.
    // Useful during hardware bring-up to verify wiring and address configuration.
    // Should not be called in production/release builds.
    static void scan();

    // Pure static class — construction is meaningless and disabled.
    I2CManager() = delete;

    // Only these driver classes are permitted to call the private bus primitives.
    // To grant access to a new driver, add its friend declaration here rather
    // than making the methods public.
    friend class PCF8574;
    friend class EEPROM24LC512;
};