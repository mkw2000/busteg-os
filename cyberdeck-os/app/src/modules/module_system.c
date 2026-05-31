#include "modules/module_system.h"
#include "core/app.h"
#include "hal/system.h"
#include "ui/renderer.h"

typedef struct SystemModuleData {
    SystemInfo info;
    float refresh_timer;
} SystemModuleData;

static SystemModuleData data;

static bool system_init(Module *module, App *app) {
    (void)app;
    module->data = &data;
    System_ReadInfo(&data.info);
    data.refresh_timer = 0.0f;
    return true;
}

static void system_update(Module *module, App *app, float dt) {
    (void)app;
    SystemModuleData *state = module->data;
    state->refresh_timer += dt;
    if (state->refresh_timer > 2.0f) {
        System_ReadInfo(&state->info);
        state->refresh_timer = 0.0f;
    }
}

static void system_render(Module *module, App *app) {
    SystemModuleData *state = module->data;
    Renderer_Frame(&app->display, "SYSTEM", app->config.primary, app->config.secondary);
    Renderer_LabelValue(&app->display, 24, 58, "hostname", state->info.hostname, app->config.secondary, app->config.accent);
    Renderer_LabelValue(&app->display, 24, 86, "uptime", state->info.uptime, app->config.secondary, app->config.accent);
    Renderer_LabelValue(&app->display, 24, 114, "cpu temp", state->info.cpu_temp, app->config.secondary, app->config.accent);
    Renderer_LabelValue(&app->display, 24, 142, "memory", state->info.memory, app->config.secondary, app->config.accent);
    Renderer_LabelValue(&app->display, 24, 170, "disk", state->info.disk, app->config.secondary, app->config.accent);
    Renderer_LabelValue(&app->display, 24, 198, "ip addr", state->info.ip_address, app->config.secondary, app->config.accent);
    Display_DrawText(&app->display, "ESC returns home", 24, 286, app->config.secondary);
}

Module Module_System_Create(void) {
    return (Module){"System", system_init, system_update, system_render, NULL, NULL, NULL};
}
