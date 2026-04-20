#include "WE_UILabel.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIManager.hpp"
#include "WolfEngine/WolfEngine.hpp"

#include <string.h>

UILabel::UILabel(const UITransform* transform, UILabelState* state)
    : BaseUIElement(transform)
    , m_state(state) {}

void UILabel::draw(UIManager& mgr) {
    if (!m_visible) return;

    UIRect rect = resolveLayout(*m_transform);
    uint16_t color = m_state->palette[m_state->colorIndex];

    DrawCommand cmd;
    cmd.type             = DrawCommandType::TextRun;
    cmd.flags            = 0;
    cmd.x                = rect.x;
    cmd.y                = rect.y;
    cmd.sortKey          = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
    cmd.textRun.text     = m_state->text;
    cmd.textRun.color    = color;
    cmd.textRun.maxWidth = static_cast<uint8_t>(rect.width > 0 ? rect.width : 0);
    RenderSys().submitDrawCommand(cmd);
}

void UILabel::setText(const char* text) {
    strncpy(m_state->text, text, WE_UI_LABEL_MAX_LEN - 1);
    m_state->text[WE_UI_LABEL_MAX_LEN - 1] = '\0';
    markDirty();
}

void UILabel::setColorIndex(uint8_t index) {
    m_state->colorIndex = index;
    markDirty();
}

const char* UILabel::getText() const {
    return m_state->text;
}

uint8_t UILabel::getColorIndex() const {
    return m_state->colorIndex;
}
