#pragma once
#include "WE_ConfigTypes.hpp"

// ── Display target selector ─────────────────────────────────────────────────
// Select your display target here. This sets the displayTarget field in Settings.render,
// which the Renderer uses to determine which display driver to compile and use.
// Only one display target should be defined at a time.
// You can define display target by uncommenting one of the options.

#define DISPLAY_ST7735
// #define DISPLAY_CUSTOM


// ── SDL injection ─────────────────────────────────────────────────
// DISPLAY_SDL is injected by the desktop CMake build (-DDISPLAY_SDL).
// So if thats the case target selection definition is overriden to SDL.
#ifdef DISPLAY_SDL
    DisplayTargetConfig constexpr DISPLAY_TARGET = { 640, 480, DisplayTarget::SDL };
#endif


// ── Display Choice ─────────────────────────────────────────────────
#ifndef DISPLAY_SDL

    #if defined(DISPLAY_ST7735)
        DisplayTargetConfig constexpr DISPLAY_TARGET = { 128, 160, DisplayTarget::ST7735 };
    #endif

    #if defined(DISPLAY_CUSTOM)
    #endif

#endif
