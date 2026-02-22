#pragma once
/*
============================================================================================
WOLF ENGINE PRECOMPİLE SETTINGS HEADER FILE

------------> Module settings are inside modules itself. <-------------
This file contains all the necessary settings and configurations for the Wolf Engine conditional compilation. 

It only includes settings that matters before the engine is compiled, such as display target for the renderer,
input device configurations, audio settings, and other engine-wide configurations. Not like settings for game.

It uses definitions and conditional compilation to allow for easy customization and configuration of the engine's features and behavior.
This file should be included in the main engine header file (WolfEngine.h) and should not be included directly in game code or other engine modules.
============================================================================================
*/


// ==================== ENGINE GENERAL SETTINGS ==================

// Target frame time in microseconds (1,000,000 / 30 = 33,333us)
#define TARGET_FRAME_TIME_US 33333





// =================== GAME OBJECT SETTINGS =======================
#define MAX_GAME_OBJECTS 64


// =================== RENDERER SETTINGS =======================

// ==== Display Target ====
// Define the target display for the renderer. Only one should be defined at a time.
#define DISPLAY_ST7735
// #define DISPLAY_CUSTOM

// -------------------------------------------------------------
//  DEFAULT_BACKGROUND_PIXEL
//  The color the renderer clears the framebuffer to at the start
//  of every frame. Visible anywhere no sprite covers the screen.
//  Format is RGB565 (16-bit): RRRRRGGGGGGBBBBB
//
//  Common values:
//      0x0000 — Black
//      0xFFFF — White
//      0x001F — Blue
//      0xF800 — Red
//      0x07E0 — Green
//  
//  Advanced:
//  Conversion from RGB888: uint16_t rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
// -------------------------------------------------------------
#define DEFAULT_BACKGROUND_PIXEL 0x0000 // Black

// -------------------------------------------------------------
//  RENDER_UI_START_ROW
//  The row where the UI region begins.
//  Rows 0 to RENDER_UI_START_ROW-1 are the game region, flushed
//  every frame. Rows RENDER_UI_START_ROW to screen height are
//  the UI region, only flushed when UI content changes.
//
//  Advanced:
//  Game region height equals RENDER_UI_START_ROW, UI region
//  height equals screenHeight - RENDER_UI_START_ROW.
// -------------------------------------------------------------
#define RENDER_UI_START_ROW 128