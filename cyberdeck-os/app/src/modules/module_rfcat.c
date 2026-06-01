#include "modules/module_rfcat.h"
#include "core/app.h"
#include "hal/serial.h"
#include "hal/system.h"
#include "ui/renderer.h"
#include <stdio.h>
#include <string.h>

typedef struct RfcatData {
    SerialDevice serial[8];
    int serial_count;
    char rfcat_label[96];
    bool rfcat_usb_present;
    char usb_lines[8][96];
    int usb_count;
    float refresh_timer;
} RfcatData;

static RfcatData data;

static void refresh(RfcatData *state) {
    state->serial_count = Serial_ListDevices(state->serial, 8);
    state->rfcat_usb_present =
        System_FindUsbId("1d50", "605b", state->rfcat_label, sizeof(state->rfcat_label));
    state->usb_count = System_ListUsb(state->usb_lines, 8);
}

static const char *status_text(const RfcatData *state) {
    if (state->rfcat_usb_present) {
        return "RFcat USB detected";
    }
    if (state->serial_count > 0) {
        return "serial device present";
    }
    if (state->usb_count > 0 && strncmp(state->usb_lines[0], "No USB", 6) != 0 &&
        strncmp(state->usb_lines[0], "USB inventory unavailable", 25) != 0) {
        return "USB seen; no ttyUSB/ttyACM";
    }
    return "not detected";
}

static bool rfcat_init(Module *module, App *app) {
    (void)app;
    module->data = &data;
    refresh(&data);
    data.refresh_timer = 0.0f;
    return true;
}

static void rfcat_update(Module *module, App *app, float dt) {
    (void)app;
    RfcatData *state = module->data;
    state->refresh_timer += dt;
    if (state->refresh_timer > 3.0f) {
        refresh(state);
        state->refresh_timer = 0.0f;
    }
}

static void rfcat_render(Module *module, App *app) {
    RfcatData *state = module->data;
    char line[96];
    Renderer_Frame(&app->display, "RFCAT CC1111", app->config.primary, app->config.secondary);
    snprintf(line, sizeof(line), "%s", status_text(state));
    Renderer_LabelValue(&app->display, 20, 54, "status", line, app->config.secondary, app->config.accent);
    if (state->rfcat_usb_present) {
        Renderer_LabelValue(&app->display, 20, 84, "device", state->rfcat_label, app->config.secondary, app->config.accent);
        Display_DrawText(&app->display, "RFcat talks over USB/libusb, not tty serial", 20, 118, app->config.secondary);
    }
    for (int i = 0; i < state->serial_count && i < 4; ++i) {
        snprintf(line, sizeof(line), "serial %d", i);
        Renderer_LabelValue(&app->display, 20, 84 + i * 24, line, state->serial[i].path, app->config.secondary, app->config.accent);
    }
    Display_DrawText(&app->display, "USB inventory", 20, 190, app->config.primary);
    for (int i = 0; i < state->usb_count && i < 3; ++i) {
        Display_DrawText(&app->display, state->usb_lines[i], 20, 216 + i * 22, app->config.secondary);
    }
    Display_DrawText(&app->display, "read-only detection; no transmit functions", 20, 286, app->config.secondary);
}

Module Module_Rfcat_Create(void) {
    return (Module){"RFcat CC1111", rfcat_init, rfcat_update, rfcat_render, NULL, NULL, NULL};
}
