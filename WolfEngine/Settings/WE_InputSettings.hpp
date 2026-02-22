#pragma once
// =================== INPUT SETTINGS =======================
// This file contains all the necessary settings and configurations for the
// Wolf Engine's input system. It defines the GPIO pins for buttons and
// joystick, as well as settings for using a PCF8574 I/O expander.
//
// QUICK SETUP:
//   1. If buttons are wired directly to the ESP32, fill in INPUT_PIN_* below.
//   2. If buttons are routed through a PCF8574, set INPUT_PCF8574_ADDR to the
//      chip's 7-bit address and fill in the PCF8574 pin map instead.
//   3. Set unused pins/axes to -1 to disable them entirely.
//   4. Joystick axes are always direct GPIO (ADC) — the PCF8574 is digital only.
// ──────────────────────────────────────────────────────────────────────────────


// =================== DIRECT GPIO BUTTONS ==================
// Assign ESP32 GPIO numbers. Set to -1 if the pin is unused or
// routed through the PCF8574 expander instead.
//
//   Button layout reference:
//   Button layout reference has been deprecated because every controller has a diffrent layout. Just label them A-F and K and wire them as you like.
//   
//   Important note: Comments on the right side of the button definitions are just suggestions based on a common gamepad layout. 
//   You can wire the buttons in any configuration you like, and label them as you see fit in your game code. 
//   The engine only recognizes them as Button A, B, C, D, E, F, and K — their logic is entirely determined by how you wire them 
//   and read them in your game, not by any predefined role.
//
#define INPUT_PIN_BUTTON_A    27    // Face button — right
#define INPUT_PIN_BUTTON_B    -1    // Face button — bottom
#define INPUT_PIN_BUTTON_C    -1    // Face button — left
#define INPUT_PIN_BUTTON_D    -1    // Face button — top
#define INPUT_PIN_BUTTON_E    -1    // Options / select
#define INPUT_PIN_BUTTON_F    -1    // Pause / start
#define INPUT_PIN_BUTTON_K    -1    // Joystick push (digital)

// GPIO 4, 5, 13, 14, 18, 19, 21, 22, 23, 25, 26, 27 — these are input/output capable with no major caveats and work well for buttons.

// =================== JOYSTICK (ANALOG) ====================
// Joystick axes must always be connected directly to ESP32 ADC-capable pins.
// PCF8574 is a digital-only device and cannot read analog signals.
//
// ADC1 channel that corresponds to INPUT_PIN_JOY_X and INPUT_PIN_JOY_Y.
// Compatible ADC1 pins on ESP32: 32–39. Avoid ADC2 (pins 0,2,4,12–15,25–27)
// as it is unavailable while Wi-Fi is active.
// The GPIO pin number and ADC channel are NOT the same thing on ESP32.
// Use the table below to find your channel:
//
//   GPIO  │  ADC1 Channel
//   ──────┼───────────────
//    36   │  ADC1_CHANNEL_0
//    37   │  ADC1_CHANNEL_1
//    38   │  ADC1_CHANNEL_2
//    39   │  ADC1_CHANNEL_3
//    32   │  ADC1_CHANNEL_4
//    33   │  ADC1_CHANNEL_5
//    34   │  ADC1_CHANNEL_6
//    35   │  ADC1_CHANNEL_7
//
#define INPUT_JOY_X_ADC_CH    ADC1_CHANNEL_0   // ADC1 channel for X axis
#define INPUT_JOY_Y_ADC_CH    ADC1_CHANNEL_1   // ADC1 channel for Y axis

// Raw ADC range the joystick produces at full deflection (0–4095 for 12-bit).
// Calibrate these by printing raw ADC values and moving the stick to each extreme.
#define INPUT_JOY_X_MIN        0
#define INPUT_JOY_X_MAX     4095
#define INPUT_JOY_Y_MIN        0
#define INPUT_JOY_Y_MAX     4095

// Dead zone as a normalized fraction (0.0–1.0) applied around centre.
// Inputs within this fraction of the centre point are returned as 0.
// Increase if your stick drifts when untouched.
#define INPUT_JOY_DEADZONE    0.1f

// Joystick centre resting values in raw ADC units.
// Read these with the stick untouched and paste the values here.
#define INPUT_JOY_X_CENTER  2048
#define INPUT_JOY_Y_CENTER  2048


// ============= PCF8574 I/O EXPANDER SETTINGS ==============
// Set INPUT_PCF8574_ADDR to the chip's 7-bit I2C address to enable expander
// support. The address is determined by the A0/A1/A2 pins on the PCF8574:
//
//   A2  A1  A0  │  Address
//   ────────────┼──────────
//    0   0   0  │  0x20   (default, all pins tied low)
//    0   0   1  │  0x21
//    0   1   0  │  0x22
//    0   1   1  │  0x23
//    1   0   0  │  0x24
//    1   0   1  │  0x25
//    1   1   0  │  0x26
//    1   1   1  │  0x27
//
// Set to -1 to disable expander support entirely.
#define INPUT_PCF8574_ADDR    -1

// PCF8574 pin assignments (P0–P7).
// Map each button to the expander pin it is wired to.
// Set to -1 if that button is not connected through the expander.
//
// NOTE: A button may be on GPIO or on the expander, but not both.
//       If INPUT_PIN_BUTTON_* is set, that takes priority over the expander pin.
//
// NOTE: Any PCF8574 pin not assigned to a button should remain high (1) in
//       firmware so it does not float and generate false inputs.
#define INPUT_PCF8574_PIN_BUTTON_A   -1   // Expander pin for face button A
#define INPUT_PCF8574_PIN_BUTTON_B   -1   // Expander pin for face button B
#define INPUT_PCF8574_PIN_BUTTON_C   -1   // Expander pin for face button C
#define INPUT_PCF8574_PIN_BUTTON_D   -1   // Expander pin for face button D
#define INPUT_PCF8574_PIN_BUTTON_E   -1   // Expander pin for options button
#define INPUT_PCF8574_PIN_BUTTON_F   -1   // Expander pin for pause button
#define INPUT_PCF8574_PIN_BUTTON_K   -1   // Expander pin for joystick push


// =================== ELECTRICAL SETTINGS ==================
// ACTIVE_LOW  (1): buttons connect the pin to GND when pressed (most common).
//   - PCF8574 pins are written HIGH to enable quasi-bidirectional input mode.
//   - Direct GPIO pins use the ESP32's internal pull-up resistor.
// ACTIVE_HIGH (0): buttons connect the pin to VCC when pressed.
//   - Requires an external pull-down resistor on each pin.
#define INPUT_BUTTON_ACTIVE_LOW   1   // 1 = active low (default), 0 = active high


// =================== POLLING SETTINGS =====================
// How often (in milliseconds) the input system polls all button states.
// 10 ms = 100 Hz, which is more than sufficient for a game controller.
#define INPUT_POLL_INTERVAL_MS    10

// Software debounce window in milliseconds.
// A state change is only committed once the new level holds stable for this long.
// Eliminates mechanical contact bounce. Typical range: 10–50 ms.
// Increase if phantom presses appear; decrease if fast inputs feel missed.
#define INPUT_DEBOUNCE_MS   20