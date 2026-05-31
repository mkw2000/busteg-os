#include "core/app.h"
#include <SDL.h>
#include <string.h>
#include "modules/module_rfcat.h"
#include "modules/module_rfnano.h"
#include "modules/module_settings.h"
#include "modules/module_system.h"
#include "modules/module_touchtest.h"

bool App_Init(App *app, const char *config_path) {
    memset(app, 0, sizeof(*app));
    app->running = true;

    Config_Load(&app->config, config_path);
    EventSystem_Init(&app->events);

    if (!Display_Init(&app->display, &app->config)) {
        return false;
    }
    if (!Input_Init(&app->input)) {
        Display_Shutdown(&app->display);
        return false;
    }

    ModuleManager_Init(&app->modules);
    ModuleManager_Add(&app->modules, Module_System_Create());
    ModuleManager_Add(&app->modules, Module_Rfcat_Create());
    ModuleManager_Add(&app->modules, Module_RfNano_Create());
    ModuleManager_Add(&app->modules, Module_TouchTest_Create());
    ModuleManager_Add(&app->modules, Module_Settings_Create());
    ModuleManager_InitModules(&app->modules, app);

    ScreenManager_Init(&app->screens);
    return true;
}

void App_Run(App *app) {
    Uint32 last_ticks = SDL_GetTicks();

    while (app->running) {
        Uint32 now = SDL_GetTicks();
        float dt = (float)(now - last_ticks) / 1000.0f;
        last_ticks = now;

        Input_Poll(&app->input, &app->events);
        NavEvent event;
        while (EventSystem_Pop(&app->events, &event)) {
            if (event.type == NAV_QUIT) {
                App_RequestQuit(app);
            } else {
                ScreenManager_HandleEvent(&app->screens, app, event);
            }
        }

        ScreenManager_Update(&app->screens, app, dt);

        Display_BeginFrame(&app->display);
        ScreenManager_Render(&app->screens, app);
        Display_EndFrame(&app->display);
        SDL_Delay(16);
    }
}

void App_RequestQuit(App *app) {
    app->running = false;
}

void App_Shutdown(App *app) {
    ModuleManager_ShutdownModules(&app->modules, app);
    Input_Shutdown(&app->input);
    Display_Shutdown(&app->display);
}
