#pragma once
// =============================================================
//                          RENDER LAYERS
// =============================================================

// -------------------------------------------------------------
//  RenderLayer
//  Edit these names to match your game's rendering needs.
//  RENDER_POOL_LAYERS is automatic — it always equals the count.
//  Layers are drawn in ascending order — layer 0 is drawn first
//  (bottom/background), the highest layer is drawn last (top/UI).
//
//  RAM cost: RENDER_POOL_LAYERS * MAX_GAME_OBJECTS * 4 bytes
//
//  EXAMPLE SETUP for a typical 2D game:
//      LAYER_BACKGROUND — static backdrop, sky, floor tiles
//      LAYER_WORLD      — general world objects, platforms, terrain
//      LAYER_ENTITIES   — enemies, NPCs, items
//      LAYER_PLAYER     — player sprite, always above enemies
//      LAYER_FX         — particle effects, explosions
//      LAYER_UI         — menus, overlays, pause screen
// -------------------------------------------------------------
enum RenderLayer {
    LAYER_BACKGROUND = 0,
    LAYER_WORLD      = 1,
    LAYER_ENTITIES   = 2,
    LAYER_PLAYER     = 3,
    LAYER_FX         = 4,
    LAYER_UI         = 5,
    RENDER_POOL_LAYERS   // always last — automatically equals layer count
    // RENDER_POOL_LAYERS is used by the renderer to know how many layers to manage. It is not an actual layer and should always be last in the enum.
    // Do not remove or reorder RENDER_POOL_LAYERS — it must always be last and will automatically equal the number of layers defined above it.
};