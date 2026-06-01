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
    char adapter_label[96];
    int count;
    int usb_count;
    bool usb_adapter_present;
    float refresh_timer;
} RfNanoData;

static RfNanoData data;

typedef struct UsbId {
    const char *vendor_id;
    const char *product_id;
} UsbId;

static const UsbId usb_serial_adapter_ids[] = {
    {"1a86", "7523"}, /* QinHeng CH340/CH341 */
    {"1a86", "5523"}, /* QinHeng CH341 */
    {"0403", "6001"}, /* FTDI FT232 */
    {"0403", "6015"}, /* FTDI FT231X */
    {"10c4", "ea60"}, /* Silicon Labs CP210x */
    {"067b", "2303"}, /* Prolific PL2303 */
    {"2341", "0001"}, /* Arduino Uno */
    {"2341", "0043"}, /* Arduino Uno R3 */
    {"2341", "0058"}, /* Arduino Nano Every */
    {"2a03", "0043"}  /* Arduino.org Uno R3 */
};

static bool find_usb_serial_adapter(char *label, size_t label_size) {
    for (size_t i = 0; i < sizeof(usb_serial_adapter_ids) / sizeof(usb_serial_adapter_ids[0]); ++i) {
        if (System_FindUsbId(usb_serial_adapter_ids[i].vendor_id,
                             usb_serial_adapter_ids[i].product_id,
                             label,
                             label_size)) {
            return true;
        }
    }
    return false;
}

static void refresh(RfNanoData *state) {
    state->count = Serial_ListDevices(state->devices, 8);
    state->usb_adapter_present = find_usb_serial_adapter(state->adapter_label, sizeof(state->adapter_label));
    state->usb_count = System_ListUsb(state->usb_lines, 8);
}

static const char *status_text(const RfNanoData *state) {
    if (state->count > 0) {
        return "serial candidate detected";
    }
    if (state->usb_adapter_present) {
        return "USB adapter seen, no tty";
    }
    return "no USB serial detected";
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
    if (state->usb_adapter_present) {
        Renderer_LabelValue(&app->display, 20, 114, "usb", state->adapter_label, app->config.secondary, app->config.accent);
    }
    for (int i = 0; i < state->count && i < 4; ++i) {
        snprintf(value, sizeof(value), "device %d", i);
        Renderer_LabelValue(&app->display, 20, 138 + i * 24, value, state->devices[i].path, app->config.secondary, app->config.accent);
    }
    if (state->count == 0 && state->usb_count > 0) {
        Display_DrawText(&app->display, "USB inventory", 20, 170, app->config.primary);
        for (int i = 0; i < state->usb_count && i < 4; ++i) {
            Display_DrawTextClipped(&app->display, state->usb_lines[i], 20, 194 + i * 20, 436, app->config.secondary);
        }
    }
    Display_DrawText(&app->display, "read-only detection", 20, 286, app->config.secondary);
}

Module Module_RfNano_Create(void) {
    return (Module){"RF-NANO nRF24", rfnano_init, rfnano_update, rfnano_render, NULL, NULL, NULL};
}
