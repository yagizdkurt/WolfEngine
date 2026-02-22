#include "WE_BaseUIElement.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIManager.hpp"

void BaseUIElement::markDirty() {
    m_dirty = true;
    if (m_manager) m_manager->m_dirty = true;
}