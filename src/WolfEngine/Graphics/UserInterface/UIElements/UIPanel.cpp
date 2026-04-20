#include "UIPanel.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIManager.hpp"
#include "WolfEngine/WolfEngine.hpp"

UIPanel::UIPanel(const UITransform* transform, UIPanelState* state, BaseUIElement** children)
    : BaseUIElement(transform), m_state(state), m_children(children) {}

void UIPanel::setSize(int16_t width, int16_t height) {
    m_state->width  = width;
    m_state->height = height;
    markDirty();
    propagateDirtyToChildren();
}

void UIPanel::setBackgroundEnabled(bool enabled) {
    m_state->backgroundEnabled = enabled;
    markDirty();
    propagateDirtyToChildren();
}

void UIPanel::setBackgroundColor(uint16_t color) {
    m_state->backgroundColor = color;
    markDirty();
    propagateDirtyToChildren();
}

BaseUIElement** UIPanel::getChildren() const { return m_children; }


void UIPanel::syncChildManagers() {
    if (!m_children) return;

    for (int16_t i = 0; m_children[i] != nullptr; ++i) {
        BaseUIElement* child = m_children[i];
        if (!child) continue;

        child->m_manager = m_manager;
    }
}

void UIPanel::propagateDirtyToChildren() {
    // There is only manager-level dirty tracking in this UI system.
    // Sync manager linkage so child updates can mark the UI dirty.
    syncChildManagers();
}

void UIPanel::draw(UIManager& mgr) {
    if (!m_visible) return;

    const int16_t panelX = getX();
    const int16_t panelY = getY();

    syncChildManagers();
    if (m_state->backgroundEnabled && m_state->width > 0 && m_state->height > 0) {
        DrawCommand bg;
        bg.type           = DrawCommandType::FillRect;
        bg.flags          = 0;
        bg.x              = panelX;
        bg.y              = panelY;
        bg.sortKey        = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
        bg.fillRect.w     = static_cast<uint8_t>(m_state->width);
        bg.fillRect.h     = static_cast<uint8_t>(m_state->height);
        bg.fillRect.color = m_state->backgroundColor;
        RenderSys().submitDrawCommand(bg);
    }

    if (!m_children) return;

    for (int16_t i = 0; m_children[i] != nullptr; ++i) {
        BaseUIElement* child = m_children[i];
        if (!child || child == this) continue;

        const UITransform* original = child->m_transform;
        if (!original) continue;

        UITransform adjusted = *original;
        adjusted.x = static_cast<int16_t>(adjusted.x + panelX);
        adjusted.y = static_cast<int16_t>(adjusted.y + panelY);

        child->m_transform = &adjusted;
        uint8_t savedLayer = child->m_layer;
        child->m_layer = m_transform->layer + 1;
        child->draw(mgr);
        child->m_layer = savedLayer;
        child->m_transform = original;
    }
}
