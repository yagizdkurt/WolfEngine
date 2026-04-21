# Pin Definitions

GPIO assignments are configured in `WolfEngine/Settings/WE_Settings.hpp` under `Settings.hardware`.

---

## Display (SPI)

```cpp
Settings.hardware.display.cs  = 17
Settings.hardware.display.rst = 4
Settings.hardware.display.dc  = 16
```

These are screen control pins. CS, RST, and DC are required — without them the display won't initialize. Keep in mind that different screens use different pins but mostly SPI communication holds.

---

## SPI Bus

```cpp
Settings.hardware.spi.mosi = 23
Settings.hardware.spi.miso = 19
Settings.hardware.spi.sclk = 18
```

Standard SPI bus pins shared by the display.

---

## I2C Bus

```cpp
Settings.hardware.i2c.sda = 21
Settings.hardware.i2c.scl = 22
```

Used by I2C devices such as the PCF8574 expander. These are the ESP32 default I2C pins and work well in most cases.

---

## Input Pins

Button GPIO assignments have their own file — see [Input Settings](input.md).

---

## Suggestions

- Pins 21 and 22 are the ESP32 hardware I2C pins — stick with these unless you have a reason to remap.
- Avoid GPIO 6–11 — these are connected to the internal flash on most ESP32 modules and will cause crashes if used.
- Avoid GPIO 34–39 for SPI and I2C — they are input-only and cannot be driven as outputs.
- If you change SPI pins, make sure they are not already claimed by another peripheral.
