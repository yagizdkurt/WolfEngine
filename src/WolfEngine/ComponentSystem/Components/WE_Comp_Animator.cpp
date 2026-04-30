#include "WE_Comp_Animator.hpp"
#include "WolfEngine/GameObjectSystem/WE_GameObject.hpp"

Animator::Animator(SpriteRenderer* mySpriteRenderer, const WE_Animation* animation)
    : m_mySpriteRenderer(mySpriteRenderer)
    , m_raw            (animation ? animation->raw           : nullptr)
    , m_frameDuration  (animation ? animation->frameDuration : 8)
    , m_looping        (animation ? animation->looping       : true)
{
    type        = COMP_ANIMATOR;
    tickEnabled = true;
    if (m_mySpriteRenderer && m_raw)
        m_mySpriteRenderer->setSprite(m_raw->sprites[m_raw->sequence[0]]);
    if (m_mySpriteRenderer)
        m_mySpriteRenderer->m_owner->registerComponent(this);
}

void Animator::tick() {
    if (m_paused || !m_mySpriteRenderer || !m_raw) return;

    if (++m_tickCounter < m_frameDuration) return;
    m_tickCounter = 0;

    uint8_t next = m_currentFrame + 1;
    if (m_raw->sequence[next] == 0xFF) {
        if (!m_looping) { m_finished = true; return; }
        next = 0;
    }
    m_currentFrame = next;
    m_mySpriteRenderer->setSprite(m_raw->sprites[m_raw->sequence[m_currentFrame]]);
}

void Animator::setFrame(uint8_t frame) {
    m_currentFrame = frame;
    m_tickCounter  = 0;
    m_finished     = false;
    if (m_mySpriteRenderer && m_raw)
        m_mySpriteRenderer->setSprite(m_raw->sprites[m_raw->sequence[m_currentFrame]]);
}

void Animator::setAnimation(const WE_Animation* animation) {
    if (!animation) return;
    m_raw          = animation->raw;
    m_frameDuration = animation->frameDuration;
    m_looping      = animation->looping;
    m_currentFrame = 0;
    m_tickCounter  = 0;
    m_finished     = false;
    if (m_mySpriteRenderer && m_raw)
        m_mySpriteRenderer->setSprite(m_raw->sprites[m_raw->sequence[0]]);
}
