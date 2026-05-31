#include "hal/input.h"
#include <SDL.h>
#include "hal/display.h"

static void push_key(EventSystem *events, SDL_Keycode key) {
    NavEvent event = {NAV_NONE, 0, 0, false};
    switch (key) {
        case SDLK_UP:
        case SDLK_w: event.type = NAV_UP; break;
        case SDLK_DOWN:
        case SDLK_s: event.type = NAV_DOWN; break;
        case SDLK_LEFT:
        case SDLK_a: event.type = NAV_LEFT; break;
        case SDLK_RIGHT:
        case SDLK_d: event.type = NAV_RIGHT; break;
        case SDLK_RETURN:
        case SDLK_SPACE: event.type = NAV_SELECT; break;
        case SDLK_ESCAPE:
        case SDLK_BACKSPACE: event.type = NAV_BACK; break;
        case SDLK_q: event.type = NAV_QUIT; break;
        default: break;
    }
    if (event.type != NAV_NONE) {
        EventSystem_Push(events, event);
    }
}

bool Input_Init(Input *input) {
    input->pointer_x = 0;
    input->pointer_y = 0;
    input->pointer_down = false;
    return true;
}

void Input_Shutdown(Input *input) {
    (void)input;
}

void Input_Poll(Input *input, EventSystem *events) {
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event)) {
        if (sdl_event.type == SDL_QUIT) {
            EventSystem_Push(events, (NavEvent){NAV_QUIT, 0, 0, false});
        } else if (sdl_event.type == SDL_KEYDOWN && !sdl_event.key.repeat) {
            push_key(events, sdl_event.key.keysym.sym);
        } else if (sdl_event.type == SDL_MOUSEMOTION) {
            int window_w = 1;
            int window_h = 1;
            SDL_GetWindowSize(SDL_GetWindowFromID(sdl_event.motion.windowID), &window_w, &window_h);
            int x = sdl_event.motion.x * DISPLAY_VIRTUAL_WIDTH / window_w;
            int y = sdl_event.motion.y * DISPLAY_VIRTUAL_HEIGHT / window_h;
            input->pointer_x = x;
            input->pointer_y = y;
            EventSystem_Push(events, (NavEvent){NAV_POINTER, input->pointer_x, input->pointer_y, input->pointer_down});
        } else if (sdl_event.type == SDL_MOUSEBUTTONDOWN || sdl_event.type == SDL_MOUSEBUTTONUP) {
            int window_w = 1;
            int window_h = 1;
            SDL_GetWindowSize(SDL_GetWindowFromID(sdl_event.button.windowID), &window_w, &window_h);
            input->pointer_x = sdl_event.button.x * DISPLAY_VIRTUAL_WIDTH / window_w;
            input->pointer_y = sdl_event.button.y * DISPLAY_VIRTUAL_HEIGHT / window_h;
            input->pointer_down = sdl_event.type == SDL_MOUSEBUTTONDOWN;
            EventSystem_Push(events, (NavEvent){NAV_POINTER, input->pointer_x, input->pointer_y, input->pointer_down});
        }
    }
}
