#include "core/screen_manager.h"
#include "core/app.h"
#include "screens/home.h"
#include "screens/splash.h"

void ScreenManager_Init(ScreenManager *manager) {
    manager->state = SCREEN_BOOT;
    manager->state_time = 0.0f;
    manager->selected_menu = 0;
    manager->active_module = -1;
}

void ScreenManager_Set(ScreenManager *manager, ScreenState state) {
    manager->state = state;
    manager->state_time = 0.0f;
}

void ScreenManager_HandleEvent(ScreenManager *manager, App *app, NavEvent event) {
    if (manager->state == SCREEN_HOME) {
        Home_HandleEvent(manager, app, event);
    } else if (manager->state == SCREEN_MODULE) {
        if (event.type == NAV_BACK) {
            manager->active_module = -1;
            ScreenManager_Set(manager, SCREEN_HOME);
            return;
        }
        Module *module = ModuleManager_Get(&app->modules, manager->active_module);
        if (module && module->handle_event) {
            module->handle_event(module, app, event);
        }
    } else if (event.type == NAV_SELECT) {
        ScreenManager_Set(manager, SCREEN_HOME);
    }
}

void ScreenManager_Update(ScreenManager *manager, App *app, float dt) {
    manager->state_time += dt;
    ModuleManager_Update(&app->modules, app, dt);

    if (manager->state == SCREEN_BOOT && manager->state_time > 0.8f) {
        ScreenManager_Set(manager, SCREEN_SPLASH);
    } else if (manager->state == SCREEN_SPLASH && manager->state_time > 1.2f) {
        ScreenManager_Set(manager, SCREEN_HOME);
    }
}

void ScreenManager_Render(ScreenManager *manager, App *app) {
    if (manager->state == SCREEN_BOOT || manager->state == SCREEN_SPLASH) {
        Splash_Render(manager, app);
    } else if (manager->state == SCREEN_HOME) {
        Home_Render(manager, app);
    } else if (manager->state == SCREEN_MODULE) {
        Module *module = ModuleManager_Get(&app->modules, manager->active_module);
        if (module && module->render) {
            module->render(module, app);
        }
    }
}
