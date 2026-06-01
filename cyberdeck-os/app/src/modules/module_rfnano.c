#include "modules/module_rfnano.h"
#include "core/app.h"
#include "hal/serial.h"
#include "hal/system.h"
#include "ui/renderer.h"
#include <stdio.h>
#include <string.h>

typedef struct RfNanoData {
    SerialDevice devices[8];
    char usb_lines[8][96];
    int count;
    int usb_count;
    float refresh_timer;
} RfNanoData;

static RfNanoData data;

static void refresh(RfNanoData *state) {
    state->count = Serial_ListDevices(state->devices, 8);
    state->usb_count = System_ListUsb(state->usb_lines, 8);
}

static const char *status_text(const RfNanoData *state) {
    if (state->count > 0) {
        return "serial candidate detected";
    }
    if (state->usb_count > 0 && strncmp(state->usb_lines[0], "No USB", 6) != 0 &&
        strncmp(state->usb_lines[0], "USB inventory unavailable", 25) != 0) {
        return "USB seen; no ttyUSB/ttyACM";
    }
    return "not detected";
}

static bool rfnano_init(Module *module, App *app) {
    module->data = &data;
    refresh(&data);
    data.refresh_timer = 0.0f;
    (void)app;
    return true;
}

static void rfnano_update(Module *module, App *app, float dt) {
    (void)app;
    RfNanoData *state = module->data;
    state->refresh_timer += dt;
    if (state->refresh_timer > 3.0f) {
        refresh(state);
        state->refresh_timer = 0.0f;
    }
}

static void rfnano_render(Module *module, App *app) {
    RfNanoData *state = module->data;
    char value[96];
    Renderer_Frame(&app->display, "RF-NANO NRF24", app->config.primary, app->config.secondary);
    Renderer_LabelValue(&app->display, 20, 58, "status", status_text(state), app->config.secondary, app->config.accent);
    snprintf(value, sizeof(value), "%d", app->config.baud_rate);
    Renderer_LabelValue(&app->display, 20, 86, "baud rate", value, app->config.secondary, app->config.accent);
    for (int i = 0; i < state->count && i < 4; ++i) {
        snprintf(value, sizeof(value), "device %d", i);
        Renderer_LabelValue(&app->display, 20, 122 + i * 24, value, state->devices[i].path, app->config.secondary, app->config.accent);
    }
    if (state->count == 0 && state->usb_count > 0) {
        Display_DrawText(&app->display, "USB inventory", 20, 190, app->config.primary);
        for (int i = 0; i < state->usb_count && i < 3; ++i) {
            Display_DrawText(&app->display, state->usb_lines[i], 20, 216 + i * 22, app->config.secondary);
        }
    }
    Display_DrawText(&app->display, "optional safe serial ping can be added here", 20, 262, app->config.secondary);
    Display_DrawText(&app->display, "no packet injection or replay", 20, 286, app->config.secondary);
}

Module Module_RfNano_Create(void) {
    return (Module){"RF-NANO nRF24", rfnano_init, rfnano_update, rfnano_render, NULL, NULL, NULL};
}
