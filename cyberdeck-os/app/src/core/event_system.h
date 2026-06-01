#ifndef CYBERDECK_EVENT_SYSTEM_H
#define CYBERDECK_EVENT_SYSTEM_H

#include <stdbool.h>

typedef enum NavEventType {
    NAV_NONE,
    NAV_UP,
    NAV_DOWN,
    NAV_LEFT,
    NAV_RIGHT,
    NAV_SELECT,
    NAV_BACK,
    NAV_QUIT,
    NAV_POINTER
} NavEventType;

typedef struct NavEvent {
    NavEventType type;
    int x;
    int y;
    bool pressed;
} NavEvent;

#define EVENT_QUEUE_CAPACITY 32

typedef struct EventSystem {
    NavEvent queue[EVENT_QUEUE_CAPACITY];
    int head;
    int tail;
} EventSystem;

void EventSystem_Init(EventSystem *events);
bool EventSystem_Push(EventSystem *events, NavEvent event);
bool EventSystem_Pop(EventSystem *events, NavEvent *event);

#endif
