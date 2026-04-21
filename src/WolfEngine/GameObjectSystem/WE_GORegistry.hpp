#pragma once
#include "WE_GameObject.hpp"
#include "WolfEngine/Settings/WE_Settings.hpp"

// =============================================================================
//                  GAME OBJECT REGISTRY
// =============================================================================

typedef struct {
    GameObject* gameObjects[Settings.limits.maxGameObjects];
    uint8_t count;
} GameObjectRegistry;