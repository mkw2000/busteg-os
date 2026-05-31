#include "screens/splash.h"
#include "core/app.h"
#include "ui/renderer.h"
#include "ui/widgets.h"

void Splash_Render(ScreenManager *manager, App *app) {
    Color primary = app->config.primary;
    Color secondary = app->config.secondary;
    Color accent = app->config.accent;
    Renderer_Frame(&app->display, manager->state == SCREEN_BOOT ? "BOOT" : "SPLASH", primary, secondary);
    Display_DrawText(&app->display, "PI CYBERDECK", 152, 112, primary);
    Display_DrawText(&app->display, "appliance ui core", 144, 142, secondary);
    Display_DrawText(&app->display, "hardware diagnostics only", 116, 170, accent);
    Widgets_Progress(&app->display, 110, 218, 260, 14, manager->state_time / (manager->state == SCREEN_BOOT ? 0.8f : 1.2f), primary);
}
