# UIManager

`UIManager` owns the top-level UI element list and draws those elements when the UI is marked dirty.

You access it through:

```cpp
UI().setElements(uiElements);
```

The element list must be null-terminated.

```cpp
static BaseUIElement* uiElements[] = { &scoreLabel, &healthLabel, nullptr };
```

---

## Setup

Register UI elements after `StartEngine()` and before `StartGame()`.

```cpp
#include "WolfEngine/WolfEngine.hpp"
#include "WolfEngine/Graphics/UserInterface/UIElements/WE_UIElements.hpp"

static const UITransform scoreTf  = { 4,  4, 0, 0, 0, 0, 0, 0, UIAnchor::TopLeft };
static const UITransform healthTf = { 4, 16, 0, 0, 0, 0, 0, 0, UIAnchor::TopLeft };

static UILabelState scoreState  = { "Score: 0" };
static UILabelState healthState = { "HP: 100" };

static UILabel scoreLabel(&scoreTf, &scoreState);
static UILabel healthLabel(&healthTf, &healthState);

static BaseUIElement* uiElements[] = { &scoreLabel, &healthLabel, nullptr };

extern "C" void app_main() {
    Engine().StartEngine();
    UI().setElements(uiElements);
    Engine().StartGame();
}
```

---

## setElements

```cpp
UI().setElements(BaseUIElement** elements);
```

What it does:

1. Stores the list and counts entries until `nullptr`
2. Wires each element to the manager pointer
3. Assigns framebuffer pointers if available
4. Marks UI dirty so it will render

---

## Dirty Behavior

`BaseUIElement::markDirty()` sets a manager-level dirty flag.

At render time:

- If UI is dirty: UI manager draws the registered elements and renderer performs a full-screen flush.
- If UI is clean: UI draw is skipped and renderer can flush only the game region.

This keeps static UI cheap in SPI bandwidth.

---

## Layout Notes

UI position is resolved from `UITransform` + `UIAnchor` (not `RENDER_UI_START_ROW`).

Use anchors (`TopLeft`, `BotCenter`, `Center`, etc.) to place elements relative to screen edges or center.
