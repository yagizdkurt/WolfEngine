#pragma once
#include <stdint.h>

// -------------------------------------------------------------
//  constexpr integer square root helper — compile time only
// -------------------------------------------------------------
constexpr int _isqrt(int n) {
    for (int i = 1; i <= n; i++)
        if (i * i == n) return i;
    return -1;  // not a perfect square
}

// =============================================================
//  WE_Sprite.hpp
//  A Sprite is a pure data asset — pixel indices and size.
//
//  USAGE:
//      constexpr uint8_t playerPixels[8 * 8] = { ... };
//      constexpr Sprite SPRITE_PLAYER = Sprite::Create(playerPixels);
//
//  Sprite::Create automatically deduces the size from the array,
//  enforces that it is a perfect square, and enforces that
//  the side length is a valid size (4, 8, 16, 32, or 64).
// =============================================================
struct Sprite {
    const uint8_t* pixels;
    int            size;
    // ---------------------------------------------------------
    //  Safe factory method. Deduces size from the array,
    //  validates it is a perfect square and a supported size.
    //  Use this instead of constructing Sprite directly.
    //
    //  VALID SIZES: 4x4, 8x8, 16x16, 32x32, 64x64
    //
    //  EXAMPLE:
    //      constexpr uint8_t playerPixels[8 * 8] = { ... };
    //      constexpr Sprite SPRITE_PLAYER = Sprite::Create(playerPixels);
    // ---------------------------------------------------------
    template<int ArraySize>
    static constexpr Sprite Create(const uint8_t (&pixels)[ArraySize]) {
        constexpr int side = _isqrt(ArraySize);
        static_assert(side != -1,
            "Sprite pixel array must be a perfect square (e.g. 4*4, 8*8, 16*16)");
        static_assert(side == 4 || side == 8 || side == 16 || side == 32 || side == 64,
            "Sprite side length must be 4, 8, 16, 32, or 64");
        return { pixels, side };
    }
};