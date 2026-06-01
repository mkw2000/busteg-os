#include "screens/home.h"
#include "core/app.h"
#include "ui/menu.h"
#include "ui/renderer.h"

#define HOME_EXTRA_ITEMS 1

void Home_HandleEvent(ScreenManager *manager, App *app, NavEvent event) {
    int item_count = app->modules.count + HOME_EXTRA_ITEMS;
    if (event.type == NAV_UP) {
        manager->selected_menu = (manager->selected_menu + item_count - 1) % item_count;
    } else if (event.type == NAV_DOWN) {
        manager->selected_menu = (manager->selected_menu + 1) % item_count;
    } else if (event.type == NAV_SELECT) {
        if (manager->selected_menu == app->modules.count) {
            App_RequestQuit(app);
        } else {
            manager->active_module = manager->selected_menu;
            ScreenManager_Set(manager, SCREEN_MODULE);
        }
    } else if (event.type == NAV_BACK) {
        App_RequestQuit(app);
    }
}

void Home_Render(ScreenManager *manager, App *app) {
    const char *items[MODULE_MANAGER_MAX_MODULES + HOME_EXTRA_ITEMS];
    for (int i = 0; i < app->modules.count; ++i) {
        items[i] = app->modules.modules[i].name;
    }
    items[app->modules.count] = "Exit";

    Renderer_Frame(&app->display, "BUSTEG OS", app->config.primary, app->config.secondary);
    Menu_Render(&app->display, items, app->modules.count + 1, manager->selected_menu, app->config.primary, app->config.secondary, app->config.accent);
    Display_DrawText(&app->display, "UP/DOWN select  ENTER open  ESC back  Q quit", 36, 288, app->config.secondary);
}
