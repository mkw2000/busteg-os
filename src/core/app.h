#ifndef CYBERDECK_APP_H
#define CYBERDECK_APP_H

#include <stdbool.h>
#include "core/config.h"
#include "core/event_system.h"
#include "core/module_manager.h"
#include "core/screen_manager.h"
#include "hal/display.h"
#include "hal/input.h"

typedef struct App {
    bool running;
    Config config;
    Display display;
    Input input;
    EventSystem events;
    ModuleManager modules;
    ScreenManager screens;
} App;

bool App_Init(App *app, const char *config_path);
void App_Run(App *app);
void App_Shutdown(App *app);
void App_RequestQuit(App *app);

#endif
