#include "modules/module_settings.h"
#include "core/app.h"
#include "hal/display.h"
#include "ui/renderer.h"
#include <stdio.h>

static bool settings_init(Module *module, App *app) {
    (void)module;
    (void)app;
    return true;
}

static void settings_render(Module *module, App *app) {
    (void)module;
    char value[96];
    Renderer_Frame(&app->display, "SETTINGS", app->config.primary, app->config.secondary);
    snprintf(value, sizeof(value), "%dx%d", DISPLAY_VIRTUAL_WIDTH, DISPLAY_VIRTUAL_HEIGHT);
    Renderer_LabelValue(&app->display, 24, 58, "virtual res", value, app->config.secondary, app->config.accent);
    snprintf(value, sizeof(value), "%dx%d", app->config.window_width, app->config.window_height);
    Renderer_LabelValue(&app->display, 24, 86, "window", value, app->config.secondary, app->config.accent);
    Renderer_LabelValue(&app->display, 24, 114, "fullscreen", app->config.fullscreen ? "true" : "false", app->config.secondary, app->config.accent);
    snprintf(value, sizeof(value), "%d", app->config.font_size);
    Renderer_LabelValue(&app->display, 24, 142, "font size", value, app->config.secondary, app->config.accent);
    snprintf(value, sizeof(value), "%d", app->config.baud_rate);
    Renderer_LabelValue(&app->display, 24, 170, "baud", value, app->config.secondary, app->config.accent);
    Renderer_LabelValue(&app->display, 24, 198, "input", "keyboard mouse touch gpio rotary", app->config.secondary, app->config.accent);
    Renderer_LabelValue(&app->display, 24, 226, "version", "0.1.0-core", app->config.secondary, app->config.accent);
}

Module Module_Settings_Create(void) {
    return (Module){"Settings", settings_init, NULL, settings_render, NULL, NULL, NULL};
}
