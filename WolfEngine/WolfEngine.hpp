#pragma once
//============================================================================================
//                        WOLF ENGINE CORE HEADER FILE
//============================================================================================

// =============== ENGINE INCLUDES ================
// ======= Settings and Constants ==========
#include "WolfEngine/Settings/WE_Settings.hpp"
#include "WolfEngine/Utilities/WE_Timer.hpp"

// ======= Low Level Systems ========
#include "WolfEngine/GameObjectSystem/WE_GORegistry.hpp"

// ======= SUBSYSTEMS ========
#include "WolfEngine/InputSystem/WE_InputManager.hpp"

// ======= Component System ========
#include "WolfEngine/ComponentSystem/Components/WE_Components.hpp"

// ======= Graphics System ========
#include "WolfEngine/Graphics/RenderSystem/WE_RenderCore.hpp"
#include "WolfEngine/Graphics/RenderSystem/WE_Camera.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIManager.hpp"
#include "WolfEngine/Graphics/ColorPalettes/WE_Palettes.hpp"

// ======= SOUND SYSTEM ========
#include "WolfEngine/Sound/WE_SoundManager.hpp"

// ======= Input System ========
#include "WolfEngine/InputSystem/WE_InputManager.hpp"

class WolfEngine final {
public:
    static WolfEngine& getInstance() { static WolfEngine instance; return instance; }

    void StartEngine();
    void StartGame();
    Renderer m_renderer;
    Camera m_Camera;
    InputManager m_InputManager;
    UIManager m_UIManager;
    SoundManager m_SoundManager;
    
private:
    GameObjectRegistry m_GameObjectRegistry = {};
    bool m_isRunning = false;
    void gameTick();
    WolfEngine() : 
    m_renderer(GetDriver()),
    m_Camera(),
    m_InputManager(),
    m_SoundManager()
    {}

    friend class GameObject;

};

// Global accessors for convenience.
inline WolfEngine& Engine()                 { return WolfEngine::getInstance(); }
inline Camera& MainCamera()                 { return WolfEngine::getInstance().m_Camera; }
inline InputManager& Input()                { return WolfEngine::getInstance().m_InputManager; }
inline UIManager& UI()                      { return WolfEngine::getInstance().m_UIManager; }
inline SoundManager& Sound()                { return WolfEngine::getInstance().m_SoundManager; }
inline Renderer& RenderSys()                { return WolfEngine::getInstance().m_renderer; }
