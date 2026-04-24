#pragma once
#include "WE_GameObject.hpp"
#include "WolfEngine/Settings/WE_Settings.hpp"

// =============================================================================
//                  GAME OBJECT REGISTRY
// =============================================================================

typedef struct {
    GameObject* gameObjects[Settings.limits.maxGameObjects];
    uint16_t    count;
    GameObject* pendingStart[Settings.limits.maxGameObjects];
    uint16_t    pendingStartCount;
    GameObject* pendingDestroy[Settings.limits.maxGameObjects];
    uint16_t    pendingDestroyCount;
} GameObjectRegistry;