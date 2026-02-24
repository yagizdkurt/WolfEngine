#include "WE_Comp_Animator.hpp"

Animator::Animator(SpriteRenderer*  owner,
                   const Sprite*    frames,
                   uint8_t          frameCount,
                   uint8_t          frameDuration)
    : m_owner         (owner)
    , m_frames        (frames)
    , m_frameCount    (frameCount)
    , m_frameDuration (frameDuration)
{
    type = COMP_ANIMATOR;
    if (m_owner && m_frames) m_owner->setSprite(&m_frames[0]);
}

void Animator::tick() {
    if (m_paused || !m_owner || !m_frames) return;

    m_tickCounter++;
    if (m_tickCounter >= m_frameDuration) {
        m_tickCounter = 0;
        m_currentFrame = (m_currentFrame + 1) % m_frameCount;
        m_owner->setSprite(&m_frames[m_currentFrame]);
    }
}

void Animator::setFrame(uint8_t frame) {
    if (frame >= m_frameCount) return;
    m_currentFrame = frame;
    m_tickCounter  = 0;
    if (m_owner) m_owner->setSprite(&m_frames[m_currentFrame]);
}

void Animator::setFrames(const Sprite* frames, uint8_t frameCount) {
    m_frames       = frames;
    m_frameCount   = frameCount;
    m_currentFrame = 0;
    m_tickCounter  = 0;
    if (m_owner) m_owner->setSprite(&m_frames[0]);
}
