#pragma once
#include "WolfEngine/Settings/WE_Settings.hpp"
#include "WolfEngine/Drivers/IODrivers/WE_ExpanderDrivers.hpp"
#include "WolfEngine/Drivers/IODrivers/WE_IExpander.hpp"
#include "WolfEngine/Drivers/IODrivers/WE_PCF8574.hpp"
#include "WolfEngine/Drivers/IODrivers/WE_PCF8575.hpp"
#include "WolfEngine/Drivers/IODrivers/WE_MCP23017.hpp"
#include "esp_adc/adc_oneshot.h"
#include <stdint.h>

enum class Button  { Any = -1, A, B, C, D, E, F, G, H, I, J };
enum class JoyAxis { X, Y };

// =============================================================
//  Controller
//  Represents a single physical controller.
//  Owns button state, expander, and joystick data.
//  Constructed and ticked by InputManager.
// =============================================================
class Controller {
public:

    // Returns true every frame the button is held down.
    // Use Button::Any to return true if any button is held.
    template<Button Btn>
    bool getButton() const {
        if constexpr (Btn == Button::Any) {
            for (int i = 0; i < Settings.input.buttonCount; ++i)
                if (m_currState[i]) return true;
            return false;
        } else {
            static_assert(static_cast<int>(Btn) >= 0 && static_cast<int>(Btn) < Settings.input.buttonCount,
                          "Button index out of range — use a valid Button enum value");
            constexpr int i = static_cast<int>(Btn);
            return m_currState[i];
        }
    }

    // Returns true only on the frame the button transitions to pressed.
    // Use Button::Any to return true if any button was just pressed.
    template<Button Btn>
    bool getButtonDown() const {
        if constexpr (Btn == Button::Any) {
            for (int i = 0; i < Settings.input.buttonCount; ++i)
                if (m_currState[i] && !m_prevState[i]) return true;
            return false;
        } else {
            static_assert(static_cast<int>(Btn) >= 0 && static_cast<int>(Btn) < Settings.input.buttonCount,
                          "Button index out of range — use a valid Button enum value");
            constexpr int i = static_cast<int>(Btn);
            return m_currState[i] && !m_prevState[i];
        }
    }

    // Returns true only on the frame the button transitions to released.
    // Use Button::Any to return true if any button was just released.
    template<Button Btn>
    bool getButtonUp() const {
        if constexpr (Btn == Button::Any) {
            for (int i = 0; i < Settings.input.buttonCount; ++i)
                if (!m_currState[i] && m_prevState[i]) return true;
            return false;
        } else {
            static_assert(static_cast<int>(Btn) >= 0 && static_cast<int>(Btn) < Settings.input.buttonCount,
                          "Button index out of range — use a valid Button enum value");
            constexpr int i = static_cast<int>(Btn);
            return !m_currState[i] && m_prevState[i];
        }
    }

    // Returns normalized joystick position [-1.0, 1.0].
    // Returns 0.0 if axis is disabled or inside dead zone.
    float getAxis(JoyAxis axis) const;

    // Bypass hardware polling and directly set button/axis state.
    // Must be called every frame for every button so m_prevState stays
    // in sync for getButtonDown() / getButtonUp() edge detection.
    void simulateButton(Button btn, bool pressed);
    void simulateJoystick(JoyAxis axis, float value);


private:
    const ControllerSettings* m_settings = nullptr;

    // ── Button state ───────────────────────────────────────────
    bool    m_currState[Settings.input.buttonCount]          = {};
    bool    m_prevState[Settings.input.buttonCount]          = {};
    bool    m_rawState[Settings.input.buttonCount]           = {};
    int64_t m_debounceTimestamp[Settings.input.buttonCount]  = {};

    // ── Joystick ───────────────────────────────────────────────
    adc_oneshot_unit_handle_t m_adcHandle = nullptr;
    float m_axisX = 0.0f;
    float m_axisY = 0.0f;

    // ── Expander ───────────────────────────────────────────────
    // Buffer sized and aligned to fit the largest expander type.
    // The correct driver is constructed into this buffer at init.
    alignas(alignof(MCP23017)) uint8_t m_expanderBuf[sizeof(MCP23017)];
    IExpander* m_expander = nullptr;

    // ── Internal ───────────────────────────────────────────────
    bool  readRaw(Button btn) const;
    float normalizeAxis(int raw, int centre, int minVal, int maxVal) const;
    void init(const ControllerSettings& settings, adc_oneshot_unit_handle_t adcHandle, int64_t now);
    void tick(int64_t now, int64_t debounceUs);
    friend class InputManager;
};