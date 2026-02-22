#pragma once
#include <stdint.h>

// =============================================================
//  WE_SpriteData.hpp
// =============================================================

// -------------------------------------------------------------
//  Rotation
//  Represents the four supported 90-degree snap rotations.
//  Applied by the renderer during drawing — sprite pixel data
//  itself never changes, the renderer reads it in a different
//  order depending on this value.
//
//  R0   — Normal, no rotation
//  R90  — 90 degrees clockwise
//  R180 — 180 degrees (upside down)
//  R270 — 270 degrees clockwise (or 90 counter-clockwise)
// -------------------------------------------------------------
enum class Rotation : uint8_t { R0   = 0, R90  = 1, R180 = 2, R270 = 3 };

// -------------------------------------------------------------
//  SpriteData
//  A lightweight snapshot of everything the renderer needs to
//  draw one sprite. Filled and returned by the sprite component
//  via getRenderData(), consumed entirely by the renderer.
//
//  The renderer never accesses the sprite component or the
//  GameObject directly — this struct is the only communication
//  between the two systems.
//
//  Fields:
//      pixels   — Pointer to the sprite's pixel index array in
//                 flash. Each byte is a palette index (0-31).
//                 Index 0 is always transparent and skipped.
//
//      palette  — Pointer to the RGB565 color palette in flash.
//                 Must be an array of exactly 32 uint16_t values.
//
//      x, y     — Screen position of the sprite's top-left corner,
//                 sourced from the owning GameObject's position.
//
//      size     — Width and height of the sprite in pixels.
//                 Always a power of two (4, 8, 16, 32, 64).
//                 The pixel array is expected to be size*size bytes.
//
//      rotation — The current rotation to apply during drawing.
//
//      visible  — If false, the renderer skips this sprite entirely
//                 without removing it from the layer.
//
// -------------------------------------------------------------
struct SpriteData {
    const uint8_t*  pixels;
    const uint16_t* palette;
    int             x;
    int             y;
    int             size;
    Rotation        rotation;
    bool            visible;
};