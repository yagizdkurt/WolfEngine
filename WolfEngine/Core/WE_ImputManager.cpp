#include "WolfEngine/Core/WE_InputManager.hpp"
#include "esp_timer.h"

// ─────────────────────────────────────────────────────────────
//  init
// ─────────────────────────────────────────────────────────────

void InputManager::init() {

    uint64_t pinMask = 0;
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (GPIO_PINS[i] != -1)
            pinMask |= (1ULL << GPIO_PINS[i]);
    }

    if (pinMask != 0) {
        gpio_config_t cfg   = {};
        cfg.mode            = GPIO_MODE_INPUT;
        cfg.pull_up_en      = INPUT_BUTTON_ACTIVE_LOW ? GPIO_PULLUP_ENABLE  : GPIO_PULLUP_DISABLE;
        cfg.pull_down_en    = INPUT_BUTTON_ACTIVE_LOW ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE;
        cfg.intr_type       = GPIO_INTR_DISABLE;
        cfg.pin_bit_mask    = pinMask;
        gpio_config(&cfg);
    }

#if INPUT_PCF8574_ADDR != -1
    m_expander.write(0xFF);
#endif

#if defined(INPUT_JOY_X_ADC_CH) || defined(INPUT_JOY_Y_ADC_CH)
    adc1_config_width(ADC_WIDTH_BIT_12);

    #if defined(INPUT_JOY_X_ADC_CH)
        adc1_config_channel_atten(INPUT_JOY_X_ADC_CH, ADC_ATTEN_DB_11);
    #endif

    #if defined(INPUT_JOY_Y_ADC_CH)
        adc1_config_channel_atten(INPUT_JOY_Y_ADC_CH, ADC_ATTEN_DB_11);
    #endif
#endif

    int64_t now = esp_timer_get_time();
    for (int i = 0; i < BUTTON_COUNT; i++)
        m_debounceTimestamp[i] = now;
}

// ─────────────────────────────────────────────────────────────
//  tick  (called every frame by WolfEngine)
// ─────────────────────────────────────────────────────────────

void InputManager::tick() {
    int64_t now = esp_timer_get_time();
    const int64_t debounceUs = static_cast<int64_t>(INPUT_DEBOUNCE_MS) * 1000LL;

    for (int i = 0; i < BUTTON_COUNT; i++) {
        bool raw = readRaw(static_cast<Button>(i));

        if (raw != m_rawState[i]) {
            m_rawState[i]          = raw;
            m_debounceTimestamp[i] = now;
        } else if ((now - m_debounceTimestamp[i]) >= debounceUs) {
            m_prevState[i] = m_currState[i];
            m_currState[i] = raw;
        }
    }

#if defined(INPUT_JOY_X_ADC_CH)
    m_axisX = normalizeAxis(
        adc1_get_raw(INPUT_JOY_X_ADC_CH),
        INPUT_JOY_X_CENTER,
        INPUT_JOY_X_MIN,
        INPUT_JOY_X_MAX
    );
#endif

#if defined(INPUT_JOY_Y_ADC_CH)
    m_axisY = normalizeAxis(
        adc1_get_raw(INPUT_JOY_Y_ADC_CH),
        INPUT_JOY_Y_CENTER,
        INPUT_JOY_Y_MIN,
        INPUT_JOY_Y_MAX
    );
#endif
}

// ─────────────────────────────────────────────────────────────
//  readRaw — read the physical level of one button
// ─────────────────────────────────────────────────────────────

bool InputManager::readRaw(Button btn) const {
    int idx = static_cast<int>(btn);

    // GPIO takes priority over the expander when both are configured.
    if (GPIO_PINS[idx] != -1) {
        int level = gpio_get_level(static_cast<gpio_num_t>(GPIO_PINS[idx]));
#if INPUT_BUTTON_ACTIVE_LOW
        return level == 0;   // Pressed = pin pulled to GND
#else
        return level == 1;   // Pressed = pin driven HIGH
#endif
    }

#if INPUT_PCF8574_ADDR != -1
    if (EXP_PINS[idx] != -1) {
        int level = m_expander.pinRead(static_cast<uint8_t>(EXP_PINS[idx]));
        if (level < 0) return false;  // I2C read failed — treat as not pressed
#if INPUT_BUTTON_ACTIVE_LOW
        return level == 0;
#else
        return level == 1;
#endif
    }
#endif

    // No source configured for this button.
    return false;
}

// ─────────────────────────────────────────────────────────────
//  normalizeAxis
// ─────────────────────────────────────────────────────────────

float InputManager::normalizeAxis(int raw, int centre, int minVal, int maxVal) const {
    float normalized;

    if (raw >= centre) {
        // Upper half: map [centre, maxVal] → [0.0, 1.0]
        int range = maxVal - centre;
        normalized = (range > 0) ? static_cast<float>(raw - centre) / range : 0.0f;
    } else {
        // Lower half: map [minVal, centre] → [-1.0, 0.0]
        int range = centre - minVal;
        normalized = (range > 0) ? static_cast<float>(raw - centre) / range : 0.0f;
    }

    // Dead zone — clamp small values to exactly 0 to prevent stick drift.
    if (normalized > -INPUT_JOY_DEADZONE && normalized < INPUT_JOY_DEADZONE)
        return 0.0f;

    // Clamp to [-1, 1] in case ADC slightly exceeds calibration range.
    if (normalized >  1.0f) return  1.0f;
    if (normalized < -1.0f) return -1.0f;

    return normalized;
}

// ─────────────────────────────────────────────────────────────
//  Public query methods
// ─────────────────────────────────────────────────────────────

bool InputManager::getButton(Button btn) const {
    return m_currState[static_cast<int>(btn)];
}

bool InputManager::getButtonDown(Button btn) const {
    int i = static_cast<int>(btn);
    return m_currState[i] && !m_prevState[i];
}

bool InputManager::getButtonUp(Button btn) const {
    int i = static_cast<int>(btn);
    return !m_currState[i] && m_prevState[i];
}

float InputManager::getAxis(JoyAxis axis) const {
    return (axis == JoyAxis::X) ? m_axisX : m_axisY;
}