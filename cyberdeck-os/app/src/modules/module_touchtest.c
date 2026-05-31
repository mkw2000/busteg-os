#include "modules/module_touchtest.h"
#include "core/app.h"
#include "ui/renderer.h"
#include <stdio.h>

typedef struct TouchData {
    int x;
    int y;
    bool pressed;
} TouchData;

static TouchData data;

static bool touch_init(Module *module, App *app) {
    (void)app;
    module->data = &data;
    data.x = DISPLAY_VIRTUAL_WIDTH / 2;
    data.y = DISPLAY_VIRTUAL_HEIGHT / 2;
    data.pressed = false;
    return true;
}

static void touch_event(Module *module, App *app, NavEvent event) {
    (void)app;
    TouchData *state = module->data;
    if (event.type == NAV_POINTER) {
        state->x = event.x;
        state->y = event.y;
        state->pressed = event.pressed;
    }
}

static void touch_render(Module *module, App *app) {
    TouchData *state = module->data;
    char value[64];
    Renderer_Frame(&app->display, "TOUCH TEST", app->config.primary, app->config.secondary);
    snprintf(value, sizeof(value), "%d,%d", state->x, state->y);
    Renderer_LabelValue(&app->display, 20, 54, "position", value, app->config.secondary, app->config.accent);
    Renderer_LabelValue(&app->display, 20, 82, "pressed", state->pressed ? "yes" : "no", app->config.secondary, app->config.accent);
    Display_DrawLine(&app->display, state->x - 12, state->y, state->x + 12, state->y, app->config.accent);
    Display_DrawLine(&app->display, state->x, state->y - 12, state->x, state->y + 12, app->config.accent);
    Display_DrawRect(&app->display, state->x - 4, state->y - 4, 8, 8, state->pressed ? app->config.primary : app->config.secondary, state->pressed);
    Display_DrawText(&app->display, "move mouse or touch screen to test mapped input", 20, 286, app->config.secondary);
}

Module Module_TouchTest_Create(void) {
    return (Module){"Touch Test", touch_init, NULL, touch_render, touch_event, NULL, NULL};
}
