# UILabel

`UILabel` renders text using the built-in 5x7 font.

---

## Declaration

```cpp
#include "WolfEngine/Graphics/UserInterface/UIElements/WE_UIElements.hpp"

// Current field order:
// { x, y, width, height, layer, anchor, marginLeft, marginRight, marginTop, marginBottom }
static const UITransform scoreTf = { 4, 4, 96, 7, 0, UIAnchor::TopLeft, 0, 0, 0, 0 };
static UILabelState scoreState = { "Score: 0" };

static UILabel scoreLabel(&scoreTf, &scoreState);
```

Register with UI manager before `StartGame()`. See [UI Manager](ui-manager.md).

---

## UITransform

`UILabel` uses the common `UITransform` struct:

```cpp
UITransform(x, y, width, height, layer, anchor, marginLeft, marginRight, marginTop, marginBottom)
```

Current renderer behavior:

- `UILabel::draw()` submits one `DrawCommandType::TextRun` command.
- `rect.width` from `resolveLayout()` is passed as `maxWidth`.
- `maxWidth = 0` means no clipping.
- `maxWidth > 0` clips at command execution time.

---

## UILabelState

```cpp
struct UILabelState {
    char            text[32];
    uint8_t         colorIndex = PL_GS_White;
    const uint16_t* palette    = PALETTE_GRAYSCALE;
};
```

Simple default:

```cpp
static UILabelState myState = { "Hello" };
```

Custom color/palette:

```cpp
static UILabelState myState = { "Score: 0", PL_WM_Amber, PALETTE_WARM };
```

---

## Runtime API

```cpp
scoreLabel.setText("Score: 42");
scoreLabel.setColorIndex(PL_GS_White);

scoreLabel.hide();
scoreLabel.show();

const char* t = scoreLabel.getText();
uint8_t c = scoreLabel.getColorIndex();
bool visible = scoreLabel.isVisible();
```

---

## Text Limits

- Max length is 32 bytes including null terminator
- Printable ASCII (32-126) is supported
- Out-of-range characters are skipped
- Font is fixed-size 5x7 with spacing
