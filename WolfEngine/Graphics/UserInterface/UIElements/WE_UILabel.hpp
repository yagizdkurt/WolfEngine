#pragma once
#include <stdint.h>
#include <string.h>
#include "WE_BaseUIElement.hpp"

// =============================================================
//  WE_UILabel
//  A text label UI element. Inherits position, visibility,
//  dirty tracking, and palette from BaseUIElement.
//
//  USAGE:
//
//  // Flash — never changes
//  static const UITransform scoreTf = { 52, 12, true };  // anchor=true → y=128+12=140
//
//  // RAM — changes at runtime
//  static UILabelState scoreState = { "Score: 0", 1 };   // text, color index
//
//  // Element
//  static UILabel scoreLabel(&scoreTf, &scoreState, PALETTE_GRAYSCALE);
//
//  // Update at runtime
//  scoreLabel.setText("Score: 42");
//  scoreLabel.setColorIndex(3);
// =============================================================

#define WE_UI_LABEL_MAX_LEN 32

// --- Stored in RAM (mutable) ---
struct UILabelState {
    char    text[WE_UI_LABEL_MAX_LEN];
    uint8_t colorIndex = PL_GS_White; // default to white
    const uint16_t* palette = PALETTE_GRAYSCALE; //default
};

// =============================================================
class UILabel : public BaseUIElement {
public:
    UILabel(const UITransform* transform, UILabelState* state) : BaseUIElement(transform) , m_state(state) {}

    void draw(UIManager& mgr) override;

    void setText(const char* text) {
        strncpy(m_state->text, text, WE_UI_LABEL_MAX_LEN - 1);
        m_state->text[WE_UI_LABEL_MAX_LEN - 1] = '\0';
        markDirty();
    }

    void setColorIndex(uint8_t index) {
        m_state->colorIndex = index;
        markDirty();
    }

    const char* getText()       const { return m_state->text;       }
    uint8_t     getColorIndex() const { return m_state->colorIndex; }

private:
    UILabelState* m_state;
};