#include "core/app.h"
#include <SDL.h>
#include <string.h>
#include "modules/module_rfcat.h"
#include "modules/module_rfnano.h"
#include "modules/module_settings.h"
#include "modules/module_system.h"
#include "modules/module_touchtest.h"

static void register_modules(App *app) {
    /*
     * This is the table of apps/tools shown on the home screen.
     * To add a new tool:
     *   1. Create modules/module_mytool.c and .h.
     *   2. Add its header above.
     *   3. Add ModuleManager_Add(&app->modules, Module_MyTool_Create()) here.
     */
    ModuleManager_Add(&app->modules, Module_System_Create());
    ModuleManager_Add(&app->modules, Module_Rfcat_Create());
    ModuleManager_Add(&app->modules, Module_RfNano_Create());
    ModuleManager_Add(&app->modules, Module_TouchTest_Create());
    ModuleManager_Add(&app->modules, Module_Settings_Create());
}

static void handle_pending_events(App *app) {
    Input_Poll(&app->input, &app->events);

    NavEvent event;
    while (EventSystem_Pop(&app->events, &event)) {
        if (event.type == NAV_QUIT) {
            App_RequestQuit(app);
        } else {
            ScreenManager_HandleEvent(&app->screens, app, event);
        }
    }
}

static void update_frame_timer(Uint32 *last_ticks, float *dt) {
    Uint32 now = SDL_GetTicks();
    *dt = (float)(now - *last_ticks) / 1000.0f;
    *last_ticks = now;
}

static void render_frame(App *app) {
    Display_BeginFrame(&app->display);
    ScreenManager_Render(&app->screens, app);
    Display_EndFrame(&app->display);
}

bool App_Init(App *app, const char *config_path) {
    memset(app, 0, sizeof(*app));
    app->running = true;

    /*
     * Config_Load starts with safe defaults, then applies config.ini if it
     * exists. That keeps desktop development and the Pi image both simple.
     */
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
    register_modules(app);
    ModuleManager_InitModules(&app->modules, app);

    ScreenManager_Init(&app->screens);
    return true;
}

void App_Run(App *app) {
    Uint32 last_ticks = SDL_GetTicks();

    while (app->running) {
        float dt = 0.0f;
        update_frame_timer(&last_ticks, &dt);

        handle_pending_events(app);
        ScreenManager_Update(&app->screens, app, dt);
        render_frame(app);

        /* About 60 FPS. This is a simple appliance UI, not a game engine. */
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
