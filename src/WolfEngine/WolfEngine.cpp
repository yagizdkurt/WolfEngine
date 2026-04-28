#include "WolfEngine.hpp"
#include "esp_timer.h"
#include "WolfEngine/Utilities/WE_I2C.hpp"
#include "WolfEngine/Utilities/Debug/WE_Debug.hpp"

void WolfEngine::Shutdown() {
#if WE_DUAL_CORE_RENDER
    m_renderer.renderShutDown();
#endif
}

void WolfEngine::StartEngine() {
    // Driver initialization
    I2CManager::begin();

    // Engine CORE subsystem initialization
    m_InputManager.init();
    m_renderer.initialize();
    m_Camera.initialize();
    m_UIManager.initialize(m_renderer.m_framebuffer);
    m_SoundManager.Initialize();

    // Module initialization
    ModuleSystem::InitAll();

    // default UI state: no registered elements
    UI().clearElements();
}

void WolfEngine::StartGame() {
    m_isRunning = true;
    lastFrameTime = esp_timer_get_time();
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj) obj->callStart();

    // Main game loop
    while (IsRunning()) gameLoop();

    // Shutdown sequence
    Shutdown();
}

void WolfEngine::gameLoop() {
    int64_t now     = esp_timer_get_time();
    int64_t elapsed = now - lastFrameTime;

    m_SoundManager.update();
    ModuleSystem::FreeUpdate();

    if (elapsed >= Settings.render.targetFrameTimeUs) { //30fps frame
        lastFrameTime = now;
        gameTick();
    }
}

// Main Game Loop Tick - called every frame
void WolfEngine::gameTick() {
    auto tickStart = WE_DiagBegin();

    // ---- Pending Start Drain ----
    for (uint16_t i = 0; i < m_GameObjectRegistry.pendingStartCount; i++)
        if (m_GameObjectRegistry.pendingStart[i])
            m_GameObjectRegistry.pendingStart[i]->callStart();
    m_GameObjectRegistry.pendingStartCount = 0;

    // ---- Early Phase ----
    m_InputManager.tick(); // Update input states first
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isUpdatable()) obj->EarlyUpdate(); // EarlyUpdate for game objects before main logic
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isUpdatable()) obj->earlyComponentTick(); // Early tick for components before game logic
    ModuleSystem::EarlyUpdate(); // Early update for modules before game logic

    // ---- Main Phase ----
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isUpdatable()) obj->Update(); // Main update for game objects
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isUpdatable()) obj->componentTick(); // Main tick for components
    ModuleSystem::Update(); // Main update for modules after game logic

    // ---- Late Phase ----
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isUpdatable()) obj->LateUpdate(); // LateUpdate for game objects after main logic
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isUpdatable()) obj->lateComponentTick(); // Late tick for components after main logic
    ModuleSystem::LateUpdate(); // Late update for modules after game logic
    m_Camera.followTick(); // Update camera after game logic so follow targets are at their new position

    // ---- End Phase ----
    for (GameObject* obj : m_GameObjectRegistry.gameObjects) if (obj && obj->isUpdatable()) obj->preRenderComponentTick(); // PreRender tick for components before rendering
    ModuleSystem::PreRender(); // PreRender for modules before rendering

    auto renderStart = WE_DiagBegin();
    m_engineDiag.updatePhaseUs = WE_DiagElapsedUs(tickStart);

    m_renderer.render(); // Render the scene

    m_engineDiag.renderPhaseUs = WE_DiagElapsedUs(renderStart);
    m_engineDiag.frameTotalUs  = WE_DiagElapsedUs(tickStart);

    uint32_t fpsElapsed = static_cast<uint32_t>(
        esp_timer_get_time() - m_fpsWindowStart);
    m_fpsFrameCount++;
    if (fpsElapsed >= 1000000u) {
        m_engineDiag.fpsAvg1s = static_cast<uint16_t>(
            (m_fpsFrameCount * 1000000u) / fpsElapsed);
        m_fpsFrameCount  = 0;
        m_fpsWindowStart = esp_timer_get_time();
    }

    if (++m_diagFrameCount >= WE_DIAG_LOG_INTERVAL_FRAMES) {
        WE_LOGI("DIAG", "frame=%luus update=%luus render=%luus fps=%u",
            m_engineDiag.frameTotalUs,
            m_engineDiag.updatePhaseUs,
            m_engineDiag.renderPhaseUs,
            m_engineDiag.fpsAvg1s);
        m_diagFrameCount = 0;
    }

    WETime::incrementFrameCount(); // Increment frame count at the end of the tick so it reflects completed frames

    // ---- Destroy Drain ----
    for (uint16_t i = 0; i < m_GameObjectRegistry.pendingDestroyCount; i++)
        delete m_GameObjectRegistry.pendingDestroy[i];
    m_GameObjectRegistry.pendingDestroyCount = 0;
}
