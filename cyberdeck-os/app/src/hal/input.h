#ifndef CYBERDECK_INPUT_H
#define CYBERDECK_INPUT_H

#include <stdbool.h>
#include "core/event_system.h"

#define INPUT_MAX_EVENT_DEVICES 16

typedef struct Input {
    int pointer_x;
    int pointer_y;
    int event_fds[INPUT_MAX_EVENT_DEVICES];
    int event_fd_count;
    bool pointer_down;
} Input;

bool Input_Init(Input *input);
void Input_Shutdown(Input *input);
void Input_Poll(Input *input, EventSystem *events);

#endif
