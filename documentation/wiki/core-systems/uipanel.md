# UIPanel

`UIPanel` is a container UI element. It can draw a background rectangle and then draw child `BaseUIElement`s relative to the panel position.

---

## Declaration

```cpp
#include "WolfEngine/Graphics/UserInterface/UIElements/WE_UIElements.hpp"

static const UITransform panelTf = { 0, -20, 0, 0, 0, 0, 0, 0, UIAnchor::BotLeft };

static UIPanelState panelState = { 128, 20, true, 0x0000 };

static BaseUIElement* panelChildren[] = { &myLabel, &myShape, nullptr };

static UIPanel panel(&panelTf, &panelState, panelChildren);
```

Register only the panel in the top-level UI list:

```cpp
static BaseUIElement* uiElements[] = { &panel, nullptr };
UI().setElements(uiElements);
```

Do not register panel children separately in `uiElements`.

---

## UIPanelState

```cpp
struct UIPanelState {
    int16_t  width;
    int16_t  height;
    bool     backgroundEnabled;
    uint16_t backgroundColor;
};
```

---

## Child List Rules

1. `m_children` must be null-terminated.
2. Children are rendered in array order.
3. Child transforms are interpreted relative to panel position during draw.

---

## Runtime API

```cpp
panel.setSize(160, 24);
panel.setBackgroundEnabled(true);
panel.setBackgroundColor(0xF800);

BaseUIElement** children = panel.getChildren();
```

Current public getter support is `getChildren()`.

---

## Draw Behavior

When `UIPanel::draw()` runs:

1. Resolve panel position from its transform.
2. Draw panel background if enabled.
3. Temporarily offset each child transform by panel `(x, y)`.
4. Draw child.
5. Restore the original child transform pointer.

This gives panel-local positioning while still using normal element draw logic.
