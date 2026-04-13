# Known Issues — Desktop Build

---

## Shutdown signal is a hard kill (std::exit)

**Location:** desktop/main_desktop.cpp — ESC/window-close handler

**What it does now:** calls std::exit(0) which hard-kills the process. The detached engine
thread gets no chance to run destructors or release resources. This is acceptable in Phase 1
because the engine holds no real resources (all hardware calls are no-ops).

**What it must become:** when Phase 2 adds a real SDL2 renderer and textures, std::exit must
be replaced with a proper shutdown signal. The standard pattern is:

    std::atomic<bool> g_running{true};

Both the SDL2 event loop and the engine frame loop watch g_running. On exit:
1. Event loop sets g_running = false
2. Engine loop sees it, exits StartGame(), releases its resources
3. engineThread.join() (not detach) so main waits for clean exit
4. SDL_DestroyWindow / SDL_Quit run after the engine is fully stopped


## WE_Settings.hpp display define is patched at CMake configure time

**Location:** desktop/CMakeLists.txt — configure-time file patch

**Why:** WE_Settings.hpp intentionally contains a user-facing #define DISPLAY_ST7735
that selects the display driver at compile time. This cannot be overridden by a
command-line -D flag. The desktop CMake patches a copy of the file to suppress
DISPLAY_ST7735 and activate DISPLAY_CUSTOM instead.

**Maintenance note:** If a new display type is added to WE_Settings.hpp (e.g.
#define DISPLAY_ILI9341), the CMake patch regex must also suppress that new define.
Otherwise the desktop build will break when the user switches display targets.

**Location of patch logic:** desktop/CMakeLists.txt, configure-time file(READ/WRITE) block.

## FreeRTOS stub behaviour differences

**Location:** `desktop/stubs/freertos/`

**Phase to review:** Phase 3 (Time / Delta)

**Items to watch:**

- `vTaskDelay` is a no-op — any engine code that uses it for timing or hardware
  settle delays will skip those waits entirely on desktop. If initialization order
  depends on these delays, behaviour may differ from hardware.

- `xSemaphoreTake` always returns `pdTRUE` — no real locking occurs. Safe while
  the engine runs on a single thread, but will cause race conditions if real
  desktop threading is introduced in later phases.

- `pdMS_TO_TICKS` is a straight passthrough — any code that does tick-based math
  rather than millisecond-based math may produce different timing results between
  desktop and hardware.

**Current status:** harmless in Phase 1, all hardware calls are no-ops anyway.
Revisit when real timing and threading are introduced.

---

## ESP_ERROR_CHECK silently swallows errors

**Location:** `desktop/stubs/esp_err.h`

**What it does now:** `ESP_ERROR_CHECK(x)` expands to `(x)` — the return value is evaluated
and discarded. On real hardware `ESP_ERROR_CHECK` calls `abort()` on any non-`ESP_OK` result.

**Impact:** Initialization failures in driver setup (I2C, GPIO, ADC, LEDC) are completely
invisible on desktop. Code that depends on a hard abort to catch misconfiguration will silently
continue in a broken state, masking bugs that would be immediately obvious on hardware.

---

## ESP_LOGD / ESP_LOGV are permanently suppressed

**Location:** `desktop/stubs/esp_log.h`

**What it does now:** `ESP_LOGD` and `ESP_LOGV` are no-ops (`do {} while(0)`). There is no
way to enable verbose or debug log levels on desktop.

**Impact:** Debug-level log output that exists in the engine cannot be activated during desktop
development. Any diagnostic instrumentation that uses `ESP_LOGD`/`ESP_LOGV` is invisible,
making it harder to trace issues in subsystems that were designed to log at those levels.

**Secondary issue:** All log output goes to `printf` (stdout). `printf` is not atomic under
concurrent writes from multiple threads, so log lines from the engine thread and any future
worker threads may interleave or corrupt each other.

---

## esp_timer desktop resolution is OS-dependent

**Location:** `desktop/stubs/esp_timer.h`

**What it does now:** `esp_timer_get_time()` uses `std::chrono::steady_clock`. The precision
and jitter of `steady_clock` are platform- and OS-scheduler-dependent (Windows in particular
has historically had coarser timer resolution than hardware timers).

**Impact:** Timing-sensitive subsystems (delta-time calculation, animation step, fixed-update
loops) may behave correctly on ESP32 but exhibit visible frame-rate variation or drift on the
desktop build, making it harder to distinguish engine bugs from platform differences.

---

## vTaskDelay declared in FreeRTOS.h instead of task.h

**Location:** `desktop/stubs/freertos/FreeRTOS.h`

**What it does now:** `vTaskDelay` is defined directly in `FreeRTOS.h`. In real FreeRTOS,
`vTaskDelay` lives in `task.h`, and `FreeRTOS.h` only provides the core primitives.

**Impact:** Any engine source file that calls `vTaskDelay` but only includes `<freertos/FreeRTOS.h>`
(not `<freertos/task.h>`) will compile cleanly on desktop but fail to compile on hardware where
the symbol is absent from `FreeRTOS.h`. This masks a missing-include class of bug.

---

## Semaphore create functions return nullptr

**Location:** `desktop/stubs/freertos/semphr.h`

**What it does now:** `xSemaphoreCreateBinary()` and `xSemaphoreCreateMutex()` both return
`nullptr`. On real FreeRTOS, a `NULL` return means allocation failed (out of heap).

**Impact:** Engine code that checks `if (handle == NULL) { /* error */ }` after creating a
semaphore will always take the error branch on desktop, masking the actual lock-usage path.
Code that does not check and proceeds to `xSemaphoreTake`/`xSemaphoreGive` with a null handle
is formally undefined behaviour, even though the stub functions currently accept `void*` and
ignore it.

**Secondary issue:** `xSemaphoreGiveFromISR` always writes `pdFALSE` to `pxHigherPriorityTaskWoken`.
On hardware this flag is used to trigger a context switch from an ISR. Any ISR-driven
wake-up logic will never fire on desktop.

---

## GPIO reads always return 0 (all inputs permanently low)

**Location:** `desktop/stubs/driver/gpio.h`

**What it does now:** `gpio_get_level()` always returns `0` regardless of pin.

**Impact:** Any input logic that polls GPIO pins (buttons, direction pads, hardware signals)
will always read "not pressed / low". Logic that waits for a rising edge or a high level
will never proceed, and edge-detection code cannot be tested on desktop.

**Also missing:** `gpio_install_isr_service`, `gpio_isr_handler_add`, and
`gpio_isr_handler_remove` are not stubbed. If any subsystem registers GPIO interrupts, the
desktop build will fail to compile with an undefined-symbol error.

---

## I2C read functions never populate the output buffer

**Location:** `desktop/stubs/driver/i2c.h`

**What it does now:** `i2c_master_read()` and `i2c_master_read_byte()` accept a `uint8_t*`
output buffer but never write to it. The buffer contents remain uninitialised after the call.

**Impact:** Any subsystem that performs an I2C read (sensor polling, external memory, etc.)
will operate on garbage data without any error signal. Bugs in I2C-based sensor code will be
invisible on desktop.

**Secondary issue:** `i2c_cmd_link_create()` returns `nullptr`. All subsequent I2C command
functions silently accept and ignore this null handle. The full I2C transaction sequence
compiles and runs without error but does absolutely nothing, making it impossible to test
command-building logic on desktop.

---

## LEDC duty reads always return 0

**Location:** `desktop/stubs/driver/ledc.h`

**What it does now:** `ledc_get_duty()` always returns `0` regardless of channel or what was
previously set via `ledc_set_duty`.

**Impact:** Any code that reads back the current duty cycle to compute a new value (e.g.,
fade logic, brightness stepping) will always start from `0` on desktop, producing different
behaviour from hardware where the hardware register holds the last-written value.

**Also missing:** `ledc_fade_func_install`, `ledc_set_fade_with_time`, `ledc_fade_start`, and
`ledc_fade_stop` are not stubbed. If the engine uses hardware-assisted fading, the desktop
build will fail to compile.

---

## ADC always returns mid-range (2048); analog inputs are frozen

**Location:** `desktop/stubs/esp_adc/adc_oneshot.h`

**What it does now:** `adc_oneshot_read()` always writes `2048` to the output — the exact
midpoint of the 12-bit range.

**Impact:** Any analog input (joystick axes, potentiometers, analog buttons) is permanently
frozen at the center/neutral value. Threshold-based input code (e.g., joystick tilt direction,
trigger press depth) can never be triggered on desktop, making analog input paths completely
untestable.

**Also missing:** `adc_cali_*` calibration functions (`adc_cali_create_scheme_curve_fitting`,
`adc_cali_raw_to_voltage`, `adc_cali_delete_scheme_curve_fitting`) are not stubbed. If the
engine uses calibrated ADC readings, the desktop build will fail to compile.

---

## SDL display driver has no renderer — window is blank

**Location:** `desktop/stubs/WE_Display_SDL2.hpp`, `desktop/WE_Display_SDL2.cpp`

**What it does now:** `SDL2DisplayDriver::flush()` is a no-op. The SDL window is created but
no `SDL_Renderer` or `SDL_Texture` is attached. The window stays black at all times.

**Impact:** No visual output from the engine framebuffer is visible on desktop in Phase 1.
This is intentional, but means the rendering pipeline (pixel writes, camera transforms, sprite
blitting) cannot be visually verified until Phase 2 wires up the real SDL renderer.

**`requiresByteSwap = false` mismatch:** The real ST7735 driver requires RGB565 big-endian
byte-swap before sending pixels over SPI. The SDL stub sets `requiresByteSwap = false`. When
Phase 2 renders real pixels, the render core must handle this difference explicitly or colours
will appear wrong on hardware after desktop testing.

---

## Screen resolution is hardcoded and not derived from settings

**Location:** `desktop/stubs/Display_SDL.h`, `desktop/stubs/WE_Display_SDL2.hpp`

**What it does now:** `RENDER_SCREEN_WIDTH` and `RENDER_SCREEN_HEIGHT` are both defined as
compile-time literals (`128` / `160`) in `Display_SDL.h`. The SDL window is also created with
these literal values in `main_desktop.cpp`.

**Impact:** If the target hardware display changes resolution (e.g., switching from ST7735
128×160 to a 240×240 panel), the desktop build must be updated manually in at least two
places. There is no single source of truth that both the hardware and desktop builds share for
screen dimensions.

---

## main_desktop.cpp comment says SDL2, code uses SDL3 API

**Location:** `desktop/main_desktop.cpp`

**What it does now:** The file header comment and several other locations in the codebase
reference "SDL2", but the actual include is `<SDL3/SDL.h>` and the code uses SDL3 event
types (`SDL_EVENT_QUIT`, `SDL_EVENT_KEY_DOWN`, `e.key.key`).

**Impact:** Not a runtime bug, but misleading. Documentation, CMake find-package calls, and
contributor instructions that say "SDL2" will lead to the wrong library being linked. Any
future SDL2-specific workaround applied based on the comments will break the SDL3 build.

---

## SDL audio is not initialised; future sound support requires CMake and init changes

**Location:** `desktop/main_desktop.cpp` — `SDL_Init(SDL_INIT_VIDEO)`

**What it does now:** SDL is initialised with `SDL_INIT_VIDEO` only.

**Impact:** When the sound subsystem (`WE_SoundManager`) is wired to a real SDL audio backend,
`SDL_Init` must also receive `SDL_INIT_AUDIO` (or be called again with that flag). This is a
known deferred step but is not documented elsewhere, so it may be overlooked when Phase 2
work begins on audio.