#include "core/event_system.h"

void EventSystem_Init(EventSystem *events) {
    events->head = 0;
    events->tail = 0;
}

bool EventSystem_Push(EventSystem *events, NavEvent event) {
    /*
     * Ring buffer basics:
     * tail is where the next event is written.
     * head is where the next event is read.
     */
    int next = (events->tail + 1) % EVENT_QUEUE_CAPACITY;
    if (next == events->head) {
        return false;
    }
    events->queue[events->tail] = event;
    events->tail = next;
    return true;
}

bool EventSystem_Pop(EventSystem *events, NavEvent *event) {
    /* Empty queue: head and tail point at the same slot. */
    if (events->head == events->tail) {
        return false;
    }
    *event = events->queue[events->head];
    events->head = (events->head + 1) % EVENT_QUEUE_CAPACITY;
    return true;
}
