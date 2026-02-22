#include "WolfEngine/ComponentSystem/Components/WE_Comp_Sprite.hpp"
#include "WolfEngine/GameObjectSystem/WE_GameObject.hpp"
#include "WolfEngine/WolfEngine.hpp"

// =============================================================
//  WE_Comp_Sprite.cpp
//  Sprite component implementation.
//
//  Handles registration/unregistration with the renderer and
//  packages owner position + sprite state into a SpriteData
//  struct for the renderer to consume each frame.
// =============================================================


// -------------------------------------------------------------
//  Constructor
//  Initializes all sprite state and registers with the renderer.
//  Called automatically when the owning GameObject is created.
// -------------------------------------------------------------
Sprite::Sprite(GameObject*      owner,
               const uint8_t*   pixels,
               int              size,
               const uint16_t*  palette,
               int              layer)
    : m_owner   (owner)
    , m_pixels  (pixels)
    , m_palette (palette)
    , m_size    (size)
    , m_layer   (layer)
{
    type = COMP_SPRITE;
    Engine().renderer.registerSprite(this, layer);
}


// -------------------------------------------------------------
//  Destructor
//  Unregisters from the renderer before the object is freed.
//  This ensures the renderer never holds a dangling pointer.
// -------------------------------------------------------------
Sprite::~Sprite() {
    Engine().renderer.unregisterSprite(this, m_layer);
}

// -------------------------------------------------------------
//  getRenderData
//  Packages everything the renderer needs into a SpriteData.
//  Called once per frame per sprite by the renderer.
//  Reads the owner's current transform position so the sprite
//  is always drawn at the correct location even if the object
//  has moved since the last frame.
// -------------------------------------------------------------
SpriteData Sprite::getRenderData() const {
    SpriteData data;
    data.pixels   = m_pixels;
    data.palette  = m_palette;
    data.x        = m_owner->transform.position.x;
    data.y        = m_owner->transform.position.y;
    data.size     = m_size;
    data.rotation = m_rotation;
    data.visible  = m_visible;
    return data;
}