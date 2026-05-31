#ifndef CYBERDECK_SCREEN_MANAGER_H
#define CYBERDECK_SCREEN_MANAGER_H

#include "core/event_system.h"

struct App;

typedef enum ScreenState {
    SCREEN_BOOT,
    SCREEN_SPLASH,
    SCREEN_HOME,
    SCREEN_MODULE
} ScreenState;

typedef struct ScreenManager {
    ScreenState state;
    float state_time;
    int selected_menu;
    int active_module;
} ScreenManager;

void ScreenManager_Init(ScreenManager *manager);
void ScreenManager_Set(ScreenManager *manager, ScreenState state);
void ScreenManager_HandleEvent(ScreenManager *manager, struct App *app, NavEvent event);
void ScreenManager_Update(ScreenManager *manager, struct App *app, float dt);
void ScreenManager_Render(ScreenManager *manager, struct App *app);

#endif
