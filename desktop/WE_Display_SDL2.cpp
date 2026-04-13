// Phase 1 desktop display driver.
// GetDriver() satisfies the call in WolfEngine's constructor.
// The driver is a no-op — flushing the framebuffer to the SDL2 window
// is deferred to a later phase.

#include "stubs/WE_Display_SDL2.hpp"

DisplayDriver* GetDriver() {
    static SDL2DisplayDriver driver;
    return &driver;
}
