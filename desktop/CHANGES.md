# Phase 1 Desktop Build — Complete Change Log

Phase 1 adds a standalone SDL3 desktop CMake target alongside the existing ESP-IDF/PlatformIO
build. The ESP32 build is completely unaffected. The desktop executable opens a 128×160 window,
initialises every WolfEngine subsystem against no-op hardware stubs, and enters the 30 fps game
loop — confirming the engine compiles and initialises cleanly on a host machine before any real
rendering or input work begins.

---

## How It Works

### The core problem
WolfEngine's source is written for ESP-IDF. Every subsystem pulls in hardware headers:
`esp_timer.h`, `driver/gpio.h`, `driver/i2c.h`, `driver/ledc.h`, `esp_adc/adc_oneshot.h`,
`freertos/*.h`, `esp_log.h`, `esp_err.h`. None of these exist on a desktop host. Additionally,
`WE_Settings.hpp` hardcodes `#define DISPLAY_ST7735`, which causes `WE_RenderCore.hpp` to
include `esp_lcd_st7735.h` — a header that can't exist on desktop.

### How the desktop build solves it

**1. Fake ESP-IDF headers (`desktop/stubs/`)**
A directory of stub headers mirrors the ESP-IDF include structure. Each stub provides the
types, constants, and function signatures the engine expects, with all function bodies as
inline no-ops (or simple return values like `ESP_OK` / `2048` for ADC). The stubs directory
is placed first in `target_include_directories`, so the compiler finds these before any real
system headers.

**2. Display driver bypass via `DISPLAY_CUSTOM`**
`WE_RenderCore.hpp` already has a conditional block:
```cpp
#if defined(DISPLAY_ST7735)
    #include "esp_lcd_st7735.h"        // ← skipped on desktop
    ...
#elif defined(DISPLAY_CUSTOM)
    #include "Display_Custom.h"        // ← our stub
#endif
```
The desktop build activates the `DISPLAY_CUSTOM` branch, which pulls in our
`stubs/Display_Custom.h`. That stub defines `RENDER_SCREEN_WIDTH = 128`,
`RENDER_SCREEN_HEIGHT = 160`, and forward-declares `GetDriver()`.

**3. Patching `WE_Settings.hpp` at configure time**
`WE_Settings.hpp` contains `#define DISPLAY_ST7735` as an unconditional in-source directive.
A CMake `-DDISPLAY_CUSTOM` flag cannot suppress it — a command-line define cannot prevent a
`#define` inside a header from executing. The desktop CMakeLists reads the real
`WE_Settings.hpp` at configure time, replaces the offending line with `#define DISPLAY_CUSTOM`,
and writes the patched copy to `${CMAKE_CURRENT_BINARY_DIR}/WolfEngine/Settings/`. The build
directory is the first include path entry, so the patched copy shadows the original for the
desktop build only. The source file is never modified.

**4. SDL3 display driver stub**
`WE_Display_ST7735.cpp` is excluded from the desktop compile (it defines the real `GetDriver()`
and has unresolvable ESP-LCD/SPI dependencies). `desktop/WE_Display_SDL2.cpp` replaces it,
providing a `GetDriver()` that returns a no-op `SDL2DisplayDriver`. The flush path does nothing
in Phase 1.

**5. Engine entry point**
`main_desktop.cpp` mirrors `src/main.cpp` exactly:
```cpp
Engine().StartEngine();   // initialises all subsystems — hits no-op stubs
Engine().StartGame();     // enters the 30 fps timing loop — never returns
```
`StartGame()` runs on a detached `std::thread` so SDL3's event loop can own the main thread
(required on most platforms). The SDL3 window stays responsive; ESC or window-close calls
`std::exit(0)` to end the process.

---

## Files Created

### `desktop/CMakeLists.txt`
Standalone CMake project (`WolfEngine_Desktop`). Key points:
- `find_package(SDL3 REQUIRED)`
- Patches `WE_Settings.hpp` into the build directory at configure time (see above)
- `file(GLOB_RECURSE … CONFIGURE_DEPENDS)` — re-globs automatically when engine `.cpp` files are added or removed
- Excludes `WE_Display_ST7735.cpp` via `list(FILTER … EXCLUDE REGEX)`
- Include path order: build dir → stubs/ → src/ → src/WolfEngine/Settings → SDL3
- Compile definition: `WE_DESKTOP_BUILD=1`

### `desktop/main_desktop.cpp`
SDL3 entry point. Creates a 128×160 window, calls `StartEngine()` + `StartGame()` on a
background thread, runs the SDL3 event loop on the main thread. Error paths call `getchar()`
before returning so the console stays open. SDL3 API used throughout:
`SDL_Init` returns `bool`, `SDL_CreateWindow` takes 4 args, `SDL_EVENT_QUIT`,
`SDL_EVENT_KEY_DOWN`, `e.key.key`.

### `desktop/WE_Display_SDL2.cpp`
Defines `GetDriver()` — returns a static `SDL2DisplayDriver` (no-op). Satisfies the call in
`WolfEngine`'s constructor. Named `SDL2` because the driver class itself is display-library
agnostic at this stage; it will be renamed when Phase 2 wires up real rendering.

### `desktop/stubs/Display_Custom.h`
Activated by the `DISPLAY_CUSTOM` branch in `WE_RenderCore.hpp`. Defines screen dimensions
and forward-declares `GetDriver()`. The linchpin of the display bypass strategy.

### `desktop/stubs/WE_Display_SDL2.hpp`
`SDL2DisplayDriver : public DisplayDriver` with empty `initialize()` and `flush()`.

### `desktop/stubs/esp_err.h`
`esp_err_t`, `ESP_OK`, `ESP_FAIL`, `ESP_ERR_INVALID_ARG`, `ESP_ERR_TIMEOUT`, etc.
Used by nearly every driver header.

### `desktop/stubs/esp_log.h`
`ESP_LOGI`/`ESP_LOGE`/`ESP_LOGW` → `printf`. Debug/verbose levels silenced.

### `desktop/stubs/esp_timer.h`
`esp_timer_get_time()` → `std::chrono::steady_clock` microseconds since program start.
Matches the ESP-IDF int64_t µs monotonic contract. Used by the engine game loop,
time utilities, controller, and sound manager.

### `desktop/stubs/driver/gpio.h`
`gpio_num_t`, `gpio_config_t`, mode/pullup/pulldown/interrupt enums.
All functions inline no-ops. Used by input and I2C systems.

### `desktop/stubs/driver/i2c.h`
All `i2c_*` types, constants (`I2C_NUM_0`, `I2C_MASTER_WRITE/READ/ACK/NACK`), and functions
as inline no-ops. Transitively includes `freertos/FreeRTOS.h` (mirroring real ESP-IDF),
which is how `pdMS_TO_TICKS` reaches `WE_I2C.hpp`.

### `desktop/stubs/driver/ledc.h`
All LEDC types and functions as inline no-ops. Used by the sound manager.

### `desktop/stubs/esp_adc/adc_oneshot.h`
`adc_channel_t` enum (`ADC_CHANNEL_0`…`9`), handle types, and all `adc_oneshot_*` functions.
`adc_oneshot_read()` writes `2048` (12-bit midpoint) so joystick axes normalise to 0.

### `desktop/stubs/freertos/FreeRTOS.h`
`TickType_t`, `portMAX_DELAY`, `pdMS_TO_TICKS`, `vTaskDelay` (no-op), and related macros.

### `desktop/stubs/freertos/semphr.h`
`SemaphoreHandle_t` and all `xSemaphore*` / `vSemaphoreDelete` functions as inline no-ops.

---

## Engine File Modified

### `src/WolfEngine/GameObjectSystem/WE_GameObject.hpp`
Added `#include <type_traits>` (line 4).

`Create<T>()` uses `std::is_base_of` at line 158. GCC's standard library provides
`<type_traits>` transitively through other headers, so GCC compiled this silently. MSVC does
not, causing an undefined symbol error. Adding the explicit include is the correct portable
fix. The ESP-IDF build is unaffected — the include is redundant there but harmless.

---

## What Was Intentionally Left Out

| Item | Reason |
|---|---|
| `WE_Display_ST7735.cpp` | Excluded from glob. Defines `GetDriver()` and has unresolvable `esp_lcd_panel_*` / SPI / FreeRTOS dependencies. |
| `esp_lcd_st7735.h` stub | Not needed — the only file that includes it is excluded. |
| `freertos/task.h` stub | `vTaskDelay` is inlined directly into `freertos/FreeRTOS.h`. No compiled engine file includes `task.h` separately. |
| All other engine source files | Zero modifications. The constraint was absolute. |
| `CMakeLists.txt` / `platformio.ini` / ESP-IDF config | Zero modifications. ESP32 build unchanged. |

---

## Build Commands

**Desktop:**
```bash
cd Game1/desktop
cmake -B build -S .
# Windows — point CMake at your SDL3 package:
# cmake -B build -S . -DSDL3_DIR="C:/SDL3/cmake"
cmake --build build
./build/WolfEngine_Desktop.exe
```

**ESP32 — unchanged:**
```bash
cd Game1
pio run -e esp32dev
```
