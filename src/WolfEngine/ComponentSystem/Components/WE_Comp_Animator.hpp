#pragma once
#include <stdint.h>
#include "WolfEngine/Graphics/SpriteSystem/WE_Sprite.hpp"
#include "WolfEngine/Graphics/AnimationSystem/WE_Animation.hpp"
#include "WE_Comp_SpriteRenderer.hpp"
#include "WE_BaseComp.hpp"


/**
 * # Animator  Component
 *
 * Drives sprite frames by sampling animation sequences each tick.
 *
 * Runs during the engine tick phase and mutates the owned `SpriteRenderer`'s
 * sprite pointer based on the current animation frame. Consumes a `WE_Animation`
 * runtime wrapper (tagline, frame duration, loop flag) and the underlying
 * `WE_AnimationRaw` asset (frame sequence and sprite indices).
 *
 * Supports playback control: pause/resume, seek to frame, swap animation,
 * and override frame duration per instance. Distinguishes looping and
 * non-looping animations; signals completion via `isFinished()`.
 *
 * ### Example Usage:
 *
 * ~~~cpp
 * Animator animator(&spriteRenderer, &Assets::PLAYER_WALK);
 * animator.setFrameDuration(6);
 * ~~~
 *
 * ### Notes:
 * - Requires a `SpriteRenderer` component on the owner GameObject.
 * - Runs during ticks; pause() stops frame advancement but keeps sprite visible.
 */
class Animator : public Component {
 public:
    /** ## Constructs an Animator attached to a SpriteRenderer and sets initial frame.
     *
     *  @param owner  Pointer to the `SpriteRenderer` this component drives; non-owning.
     *  @param animation  Pointer to a `WE_Animation` wrapper; non-owning, may be null.
     */
     Animator(SpriteRenderer* owner, const WE_Animation* animation);

    /** ## Immediately jump to a specific position in the animation sequence.
     *
     *  Resets the internal tick counter so the frame persists for a full frame duration.
     *
     *  @param frame  Zero-based sequence index; out-of-range calls are ignored.
     */
     void setFrame(uint8_t frame);

    /** ## Swaps the active animation and resets playback to frame 0.
     *
     *  @param animation  Pointer to a `WE_Animation` wrapper; non-owning, may be null.
     */
     void setAnimation(const WE_Animation* animation);

    /** ## Overrides the frame duration for this instance only.
     *
     *  @param duration  Ticks per frame; must be > 0.
     */
     void setFrameDuration(uint8_t duration) { m_frameDuration = duration; }

    /** ## Returns the current sequence position (frame index) being displayed.
     *
     *  @return Zero-based index within the active animation sequence.
     */
     uint8_t getCurrentFrame() const { return m_currentFrame; }

    /** ## Pauses playback; tick() will not advance frames while paused.
     */
     void pause()          { m_paused =  true;  }

    /** ## Resumes playback after pause().
     */
     void resume()         { m_paused = false;  }

    /** ## Returns whether playback is currently paused.
     *
     *  @return True if paused, false otherwise.
     */
     bool isPaused() const { return  m_paused;  }

    /** ## Returns true if a non-looping animation has completed.
     *
     *  Always false for looping animations. Resets when setAnimation() or setFrame() is called.
     *
     *  @return True if non-looping animation has exhausted its sequence.
     */
     bool isFinished() const { return m_finished; }

private:
    void tick() override;

    SpriteRenderer*       m_mySpriteRenderer;
    const WE_AnimationRaw* m_raw;
    uint8_t               m_frameDuration;
    bool                  m_looping;
    uint8_t               m_currentFrame = 0;
    uint8_t               m_tickCounter  = 0;
    bool                  m_paused       = false;
    bool                  m_finished     = false;
};
