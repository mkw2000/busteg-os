#ifndef CYBERDECK_INPUT_H
#define CYBERDECK_INPUT_H

#include <stdbool.h>
#include "core/event_system.h"

typedef struct Input {
    int pointer_x;
    int pointer_y;
    bool pointer_down;
} Input;

bool Input_Init(Input *input);
void Input_Shutdown(Input *input);
void Input_Poll(Input *input, EventSystem *events);

#endif
