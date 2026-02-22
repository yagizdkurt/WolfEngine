#include "WE_UILabel.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIManager.hpp"

void UILabel::draw(UIManager& mgr) {
    if (!m_visible) return;

    const char*    text  = m_state->text;
    const int16_t  x     = getX();
    const int16_t  y     = getY();
    const uint16_t color = m_state ->palette[m_state->colorIndex];
    const int16_t  step  = FONT_5x7_WIDTH + FONT_5x7_SPACING;

    for (int i = 0; text[i] != '\0' && i < WE_UI_LABEL_MAX_LEN; i++) {
        mgr.drawChar(x + i * step, y, text[i], color);
    }
}
