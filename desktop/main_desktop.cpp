// Phase 1 desktop entry point.
//
// Mirrors app_main() from src/main.cpp:
//   Engine().StartEngine()  — initialises all subsystems against no-op stubs
//   Engine().StartGame()    — enters the 30 fps game loop (blocks forever)
//
// StartGame() is run on a background thread so the SDL2 event loop can
// stay on the main thread (required on macOS and most Linux WMs).
// ESC or window-close calls std::exit(0) which tears down cleanly for Phase 1.

#include <SDL3/SDL.h>
#include <thread>
#include <cstdlib>
#include <cstdio>

#include "WolfEngine/WolfEngine.hpp"

int main(int /*argc*/, char* /*argv*/[]) {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        getchar();
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow(
        "WolfEngine Desktop",
        RENDER_SCREEN_WIDTH, RENDER_SCREEN_HEIGHT,
        0
    );
    if (!win) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Initialise all engine subsystems.  All hardware calls hit no-op stubs.
    Engine().StartEngine();

    // StartGame() never returns — run it on a background thread.
    std::thread engineThread([]{ Engine().StartGame(); });
    engineThread.detach();

    // Main thread: process SDL2 events so the window stays responsive.
    SDL_Event e;
    while (true) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                goto shutdown;
            }
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) {
                goto shutdown;
            }
        }
        SDL_Delay(16);  // ~60 Hz event polling, no busy-spin
    }

shutdown:
    SDL_DestroyWindow(win);
    SDL_Quit();
    std::exit(0);
}
