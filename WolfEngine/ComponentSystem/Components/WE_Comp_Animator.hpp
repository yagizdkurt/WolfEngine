#pragma once
#include <stdint.h>
#include "WolfEngine/Graphics/SpriteSystem/WE_Sprite.hpp"
#include "WE_Comp_SpriteRenderer.hpp"
#include "WE_BaseComp.hpp"

// =============================================================
//  WE_Comp_Animator.hpp
//  The Animator is a controller component that drives a
//  SpriteRenderer by cycling through an array of Sprite frames.
//
//  It holds a pointer to the SpriteRenderer it controls and
//  advances frames based on a configurable frame duration.
//
//  USAGE:
//      constexpr uint8_t walkPixels0[8 * 8] = { ... };
//      constexpr uint8_t walkPixels1[8 * 8] = { ... };
//      constexpr uint8_t walkPixels2[8 * 8] = { ... };
//      constexpr uint8_t walkPixels3[8 * 8] = { ... };
//
//      constexpr Sprite WALK_FRAMES[] = {
//          Sprite::Create(walkPixels0),
//          Sprite::Create(walkPixels1),
//          Sprite::Create(walkPixels2),
//          Sprite::Create(walkPixels3),
//      };
//
//      class Player : public GameObject {
//      public:
//          SpriteRenderer spriteRenderer = SpriteRenderer(this, &WALK_FRAMES[0], PALETTE_WARM, LAYER_PLAYER);
//          Animator       animator       = Animator(&spriteRenderer, WALK_FRAMES, 4, 8);
//
//          void Update() override {
//              animator.tick();
//          }
//      };
// =============================================================

class Animator : public Component {
public:

    // owner          — pointer to the SpriteRenderer this animator controls
    // frames         — array of Sprite assets representing animation frames
    // frameCount     — number of frames in the array
    // frameDuration  — how many game ticks each frame is shown for
    Animator(SpriteRenderer*  owner,
             const Sprite*    frames,
             uint8_t          frameCount,
             uint8_t          frameDuration = 8);

    // Advances the animation by one tick — call this in Update()
    void tick();

    // ---------------------------------------------------------
    //  Controls
    // ---------------------------------------------------------

    // Jump to a specific frame immediately
    void setFrame(uint8_t frame);

    // Change the frame array at runtime — for switching animations
    void setFrames(const Sprite* frames, uint8_t frameCount);

    // Change how many ticks each frame is shown for
    void setFrameDuration(uint8_t duration) { m_frameDuration = duration; }

    // Pause / resume the animation
    void pause()  { m_paused = true;  }
    void resume() { m_paused = false; }

    // ---------------------------------------------------------
    //  Getters
    // ---------------------------------------------------------
    uint8_t getCurrentFrame() const { return m_currentFrame; }
    bool    isPaused()        const { return m_paused;       }

private:
    SpriteRenderer* m_owner;
    const Sprite*   m_frames;
    uint8_t         m_frameCount;
    uint8_t         m_frameDuration;
    uint8_t         m_currentFrame = 0;
    uint8_t         m_tickCounter  = 0;
    bool            m_paused       = false;
};