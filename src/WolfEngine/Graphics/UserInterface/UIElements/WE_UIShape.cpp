#include "WE_UIShape.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIManager.hpp"
#include "WolfEngine/WolfEngine.hpp"

UIShape::UIShape(const UITransform* transform, UIShapeState* state)
    : BaseUIElement(transform)
    , m_state(state) {}

void UIShape::setSize(int16_t width, int16_t height) {
    m_state->width = width;
    m_state->height = height;
    markDirty();
}

void UIShape::setColorIndex(uint8_t index) {
    m_state->colorIndex = index;
    markDirty();
}

void UIShape::setShape(UIShapeType shape) {
    m_state->shape = shape;
    markDirty();
}

void UIShape::setFilled(bool filled) {
    m_state->filled = filled;
    markDirty();
}

void UIShape::setPalette(const uint16_t* palette) {
    m_state->palette = palette;
    markDirty();
}

void UIShape::setLength(int16_t length) {
    switch (m_state->shape) {
    case UIShapeType::HLine:
        m_state->width = length;
        break;
    case UIShapeType::VLine:
        m_state->height = length;
        break;
    default:
        // Rectangle: sets both dimensions to the same value
        m_state->width  = length;
        m_state->height = length;
        break;
    }
    markDirty();
}

int16_t     UIShape::getWidth()      const { return m_state->width; }
int16_t     UIShape::getHeight()     const { return m_state->height; }
uint8_t     UIShape::getColorIndex() const { return m_state->colorIndex; }
bool        UIShape::isFilled()      const { return m_state->filled; }
UIShapeType UIShape::getShape()      const { return m_state->shape; }

void UIShape::draw(UIManager& mgr) {
    if (!m_visible) return;

    UIRect rect = resolveLayout(*m_transform);
    const int16_t  width  = m_state->width;
    const int16_t  height = m_state->height;
    const uint16_t color  = m_state->palette[m_state->colorIndex];

    // Shape-aware early-out: HLine only needs a valid width,
    // VLine only needs a valid height.
    switch (m_state->shape) {
    case UIShapeType::HLine:  if (width  <= 0) return; break;
    case UIShapeType::VLine:  if (height <= 0) return; break;
    default:                  if (width  <= 0 || height <= 0) return; break;
    }

    DrawCommand cmd;

    switch (m_state->shape) {
    case UIShapeType::HLine:
        cmd.type       = DrawCommandType::Line;
        cmd.flags      = 0;
        cmd.x          = rect.x;
        cmd.y          = rect.y;
        cmd.sortKey    = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
        cmd.line.x2    = static_cast<int16_t>(rect.x + width - 1);
        cmd.line.y2    = rect.y;
        cmd.line.color = color;
        RenderSys().submitDrawCommand(cmd);
        break;

    case UIShapeType::VLine:
        cmd.type       = DrawCommandType::Line;
        cmd.flags      = 0;
        cmd.x          = rect.x;
        cmd.y          = rect.y;
        cmd.sortKey    = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
        cmd.line.x2    = rect.x;
        cmd.line.y2    = static_cast<int16_t>(rect.y + height - 1);
        cmd.line.color = color;
        RenderSys().submitDrawCommand(cmd);
        break;

    case UIShapeType::Rectangle:
    default:
        if (m_state->filled) {
            cmd.type           = DrawCommandType::FillRect;
            cmd.flags          = 0;
            cmd.x              = rect.x;
            cmd.y              = rect.y;
            cmd.sortKey        = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
            cmd.fillRect.w     = static_cast<uint8_t>(width);
            cmd.fillRect.h     = static_cast<uint8_t>(height);
            cmd.fillRect.color = color;
            RenderSys().submitDrawCommand(cmd);
        } else {
            // Top edge
            cmd.type       = DrawCommandType::Line;
            cmd.flags      = 0;
            cmd.x          = rect.x;
            cmd.y          = rect.y;
            cmd.sortKey    = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
            cmd.line.x2    = static_cast<int16_t>(rect.x + width - 1);
            cmd.line.y2    = rect.y;
            cmd.line.color = color;
            RenderSys().submitDrawCommand(cmd);
            // Bottom edge
            cmd.type       = DrawCommandType::Line;
            cmd.flags      = 0;
            cmd.x          = rect.x;
            cmd.y          = static_cast<int16_t>(rect.y + height - 1);
            cmd.sortKey    = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
            cmd.line.x2    = static_cast<int16_t>(rect.x + width - 1);
            cmd.line.y2    = static_cast<int16_t>(rect.y + height - 1);
            cmd.line.color = color;
            RenderSys().submitDrawCommand(cmd);
            // Left edge
            cmd.type       = DrawCommandType::Line;
            cmd.flags      = 0;
            cmd.x          = rect.x;
            cmd.y          = rect.y;
            cmd.sortKey    = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
            cmd.line.x2    = rect.x;
            cmd.line.y2    = static_cast<int16_t>(rect.y + height - 1);
            cmd.line.color = color;
            RenderSys().submitDrawCommand(cmd);
            // Right edge
            cmd.type       = DrawCommandType::Line;
            cmd.flags      = 0;
            cmd.x          = static_cast<int16_t>(rect.x + width - 1);
            cmd.y          = rect.y;
            cmd.sortKey    = cmdMakeSortKey(static_cast<RenderLayer>(m_layer), m_drawOrder);
            cmd.line.x2    = static_cast<int16_t>(rect.x + width - 1);
            cmd.line.y2    = static_cast<int16_t>(rect.y + height - 1);
            cmd.line.color = color;
            RenderSys().submitDrawCommand(cmd);
        }
        break;
    }
}
