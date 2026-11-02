/* The app icon on a window: res/icon.png, embedded at build time (CMakeLists.txt writes it into sdw_icon_png.h and
 * defines SDW_HAVE_APP_ICON when the file exists). SDL decodes it and hands it to the system: the taskbar and title bar
 * on Windows, the window manager on Linux (X11, and Wayland where the compositor supports window icons), the Dock on
 * macOS for a run outside the app bundle. For the game (platform_sdl.cpp) and the launcher, which both include SDL. */
#ifndef SDW_APP_ICON_H
#define SDW_APP_ICON_H

#include <SDL3/SDL.h>

#ifdef SDW_HAVE_APP_ICON
#include "sdw_icon_png.h" /* g_sdwIconPng[] (generated in the build folder) */
#endif

static inline void App_SetWindowIcon(SDL_Window *window)
{
#ifdef SDW_HAVE_APP_ICON
    SDL_Surface *icon = SDL_LoadPNG_IO(SDL_IOFromConstMem(g_sdwIconPng, sizeof(g_sdwIconPng)), true);
    if (!icon) {
        SDL_Log("app icon: %s", SDL_GetError());
        return;
    }
    if (!SDL_SetWindowIcon(window, icon))
        SDL_Log("app icon: %s", SDL_GetError());
    SDL_DestroySurface(icon);
#else
    (void)window;
#endif
}

#endif
