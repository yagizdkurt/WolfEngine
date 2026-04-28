# WolfEngine

WolfEngine is a 2D game engine for the ESP32. It runs on a microcontroller, talks to a small TFT display, and gives you a familiar, component-based workflow for building games on hardware that fits in your hand.

The design draws heavily from Unity — you work with GameObjects, attach Components, override `Start()` and `Update()`, and let the engine handle the loop. If you've written a Unity game before, a lot of this will feel like home. If you haven't, it's a clean model that's easy to learn.

---

## What you're working with

The target platform is an **ESP32** driving a **TFT display**. Everything in the engine is designed around that constraint.

A minimal game looks like this:

```cpp
#include "WolfEngine/WolfEngine.hpp"

class Player : public GameObject {
public:
    // Image itself
    static constexpr uint8_t playerSpriteData[6][6] = {
        {0, 0, 1, 1, 0, 0},
        {0, 1, 2, 2, 1, 0},
        {1, 2, 3, 3, 2, 1},
        {1, 2, 3, 3, 2, 1},
        {0, 1, 2, 2, 1, 0},
        {0, 0, 1, 1, 0, 0}
    };

    // Sprite
    static constexpr Sprite Splayer = Sprite::Create(playerSpriteData, PALETTE_WARM);

    // Renderer
    SpriteRenderer spriteRenderer = SpriteRenderer(this, &Splayer);
    Controller* controller = nullptr;

    void Start() override {
        transform.position = { 64, 64 };
        controller = Input().getController(0);
    }

    void Update() override {
        if (!controller) return;
        transform.position.x += controller->getAxis(JoyAxis::X) * 2.0f;
        transform.position.y += controller->getAxis(JoyAxis::Y) * 2.0f;
    }
};

extern "C" void app_main() {
    Engine().StartEngine();
    GameObject::Create<Player>();
    Engine().StartGame(); // never returns
}
```

`StartGame()` takes over and runs the loop. Your code lives in `Start()`, `Update()`, and the other lifecycle hooks. The engine handles input polling, frame pacing, rendering, and the display flush every frame.

---

## What's inside

The engine ships with a set of systems that cover the common needs of a game:

- **Input** — button events and analog joystick axes, per-controller
- **Camera** — world-space viewport with follow support
- **Audio** — tone playback on the onboard buzzer
- **Renderer** — layered draw command queue with a world pass and a UI pass
- **UI System** — retained-mode panels and labels rendered in a separate pass after the world
- **Animator** — frame-based sprite animation with state transitions
- **Time** — delta time and frame-rate-aware timers

Systems are globally accessible by name — `Input()`, `UI()`, `Sound()`, `MainCamera()`, `RenderSys()` — so you can reach them from anywhere without passing references around.

---

## Where to start

If you haven't set up the hardware yet, go to [Installation and Setup](getting-started/installation-and-setup.md) first.

If you want the project structure and startup flow first, read [Engine Basics](getting-started/Engine-Basics.md) for the folder layout, `main.cpp`, and object creation pattern.

Everything else  is in the sidebar.