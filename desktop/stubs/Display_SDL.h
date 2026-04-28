#pragma once
// Activated by -DDISPLAY_SDL in the desktop CMakeLists.
// Provides the two macros that WE_RenderCore.hpp expects from a display header,
// and forward-declares GetDriver() so WolfEngine.hpp can call it in its constructor.

#include "WolfEngine/Drivers/DisplayDrivers/WE_Display_Driver.hpp"

// Defined in desktop/WE_Display_SDL2.cpp
DisplayDriver* GetDriver();
