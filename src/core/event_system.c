#include "core/event_system.h"

void EventSystem_Init(EventSystem *events) {
    events->head = 0;
    events->tail = 0;
}

bool EventSystem_Push(EventSystem *events, NavEvent event) {
    int next = (events->tail + 1) % 32;
    if (next == events->head) {
        return false;
    }
    events->queue[events->tail] = event;
    events->tail = next;
    return true;
}

bool EventSystem_Pop(EventSystem *events, NavEvent *event) {
    if (events->head == events->tail) {
        return false;
    }
    *event = events->queue[events->head];
    events->head = (events->head + 1) % 32;
    return true;
}
