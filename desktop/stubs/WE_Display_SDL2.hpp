#pragma once
// No-op DisplayDriver implementation for Phase 1 desktop build.
// flush() does nothing — rendering to screen comes in a later phase.

#include "WolfEngine/Drivers/DisplayDrivers/WE_Display_Driver.hpp"

class SDL2DisplayDriver : public DisplayDriver {
public:
    SDL2DisplayDriver() {
        screenWidth  = 128;
        screenHeight = 160;
        requiresByteSwap = false;
    }
    void initialize()                                                    override {}
    void flush(const uint16_t*, int, int, int, int)                     override {}
    void setBacklight(uint8_t)                                           override {}
    void sleep(bool)                                                     override {}
};
