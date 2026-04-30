#pragma once
#include "WolfEngine/Graphics/SpriteSystem/WE_Sprite.hpp"

// =============================================================
//  WE_Animation
//  A lightweight animation asset — an array of Sprite frames
//  and a frame count. Consumed by the Animator component.
//
//  USAGE:
//      animator.setAnimation(&Assets::WALK);
// =============================================================
struct WE_AnimationRaw {
    const Sprite** sprites;    // doesnt need sprite count because the sequence is terminated by 0xFF
    const uint8_t* sequence;   // terminated by 0xFF
};

struct WE_Animation {
    const WE_AnimationRaw* raw;
    uint8_t frameDuration; // how many game ticks each frame is shown for
    bool looping;          // whether the animation should loop back to the start after the last frame
};