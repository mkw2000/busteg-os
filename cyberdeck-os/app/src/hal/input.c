#include "hal/input.h"
#ifdef __linux__
#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#endif
#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "hal/display.h"

static void push_sdl_key(EventSystem *events, SDL_Keycode key) {
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

#ifdef __linux__
static void push_evdev_key(EventSystem *events, unsigned short key) {
    NavEvent event = {NAV_NONE, 0, 0, false};
    switch (key) {
        case KEY_UP:
        case KEY_W: event.type = NAV_UP; break;
        case KEY_DOWN:
        case KEY_S: event.type = NAV_DOWN; break;
        case KEY_LEFT:
        case KEY_A: event.type = NAV_LEFT; break;
        case KEY_RIGHT:
        case KEY_D: event.type = NAV_RIGHT; break;
        case KEY_ENTER:
        case KEY_SPACE: event.type = NAV_SELECT; break;
        case KEY_ESC:
        case KEY_BACKSPACE: event.type = NAV_BACK; break;
        case KEY_Q: event.type = NAV_QUIT; break;
        default: break;
    }
    if (event.type != NAV_NONE) {
        EventSystem_Push(events, event);
    }
}

static void open_evdev_inputs(Input *input) {
    DIR *dir = opendir("/dev/input");
    if (!dir) {
        fprintf(stderr, "evdev input unavailable: /dev/input not found\n");
        return;
    }

    struct dirent *entry = NULL;
    while ((entry = readdir(dir)) != NULL && input->event_fd_count < INPUT_MAX_EVENT_DEVICES) {
        if (strncmp(entry->d_name, "event", 5) != 0) {
            continue;
        }

        char path[128];
        snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
            continue;
        }

        input->event_fds[input->event_fd_count++] = fd;
        fprintf(stderr, "opened evdev input: %s\n", path);
    }

    closedir(dir);
    if (input->event_fd_count == 0) {
        fprintf(stderr, "no evdev input devices opened\n");
    }
}

static void poll_evdev_inputs(Input *input, EventSystem *events) {
    for (int i = 0; i < input->event_fd_count; i++) {
        struct input_event event;
        ssize_t bytes = 0;
        while ((bytes = read(input->event_fds[i], &event, sizeof(event))) == (ssize_t)sizeof(event)) {
            if (event.type == EV_KEY && event.value == 1) {
                push_evdev_key(events, event.code);
            }
        }
    }
}
#else
static void open_evdev_inputs(Input *input) {
    (void)input;
}

static void poll_evdev_inputs(Input *input, EventSystem *events) {
    (void)input;
    (void)events;
}
#endif

bool Input_Init(Input *input) {
    input->pointer_x = 0;
    input->pointer_y = 0;
    input->pointer_down = false;
    input->event_fd_count = 0;
    for (int i = 0; i < INPUT_MAX_EVENT_DEVICES; i++) {
        input->event_fds[i] = -1;
    }
    open_evdev_inputs(input);
    return true;
}

void Input_Shutdown(Input *input) {
#ifdef __linux__
    for (int i = 0; i < input->event_fd_count; i++) {
        if (input->event_fds[i] >= 0) {
            close(input->event_fds[i]);
            input->event_fds[i] = -1;
        }
    }
#endif
    input->event_fd_count = 0;
}

void Input_Poll(Input *input, EventSystem *events) {
    poll_evdev_inputs(input, events);

    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event)) {
        if (sdl_event.type == SDL_QUIT) {
            EventSystem_Push(events, (NavEvent){NAV_QUIT, 0, 0, false});
        } else if (sdl_event.type == SDL_KEYDOWN && !sdl_event.key.repeat) {
            push_sdl_key(events, sdl_event.key.keysym.sym);
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
