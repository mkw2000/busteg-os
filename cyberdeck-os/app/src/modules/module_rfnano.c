#include "modules/module_rfnano.h"
#include "core/app.h"
#include "hal/serial.h"
#include "hal/system.h"
#include "ui/renderer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define RFNANO_CHANNELS 126

typedef struct RfNanoData {
    SerialDevice devices[8];
    char usb_lines[8][96];
    char adapter_label[96];
    char active_path[128];
    char line_buffer[160];
    char last_status[96];
    char last_rx[96];
    char last_scan[96];
    char last_payload_hex[80];
    char last_tx[96];
    int scan_levels[RFNANO_CHANNELS];
    int count;
    int usb_count;
    int fd;
    int channel;
    size_t line_length;
    bool usb_adapter_present;
    bool bridge_ready;
    bool scan_enabled;
    float refresh_timer;
    float poll_timer;
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

static void reset_bridge_state(RfNanoData *state, const char *status) {
    state->bridge_ready = false;
    state->line_length = 0;
    snprintf(state->last_status, sizeof(state->last_status), "%s", status);
    state->last_rx[0] = '\0';
    state->last_scan[0] = '\0';
    state->last_payload_hex[0] = '\0';
}

static void close_bridge(RfNanoData *state, const char *status) {
    if (state->fd >= 0) {
        Serial_Close(state->fd);
        state->fd = -1;
    }
    state->active_path[0] = '\0';
    reset_bridge_state(state, status);
}

static void send_command(RfNanoData *state, const char *command) {
    if (state->fd < 0) {
        return;
    }
    Serial_WriteText(state->fd, command);
    Serial_WriteText(state->fd, "\n");
    snprintf(state->last_tx, sizeof(state->last_tx), "%s", command);
}

static void open_first_serial(RfNanoData *state, int baud_rate) {
    if (state->fd >= 0 || state->count == 0) {
        return;
    }

    state->fd = Serial_Open(state->devices[0].path, baud_rate);
    if (state->fd < 0) {
        snprintf(state->last_status, sizeof(state->last_status), "open failed: %.82s", state->devices[0].path);
        return;
    }

    snprintf(state->active_path, sizeof(state->active_path), "%s", state->devices[0].path);
    reset_bridge_state(state, "serial open, probing bridge");
    usleep(150000);
    send_command(state, "PING");
    send_command(state, "STATUS");
}

static void handle_bridge_line(RfNanoData *state, const char *line) {
    int scan_channel = -1;
    int rx_length = 0;
    char rx_payload[80];

    if (strncmp(line, "OK RFNANO_BRIDGE", 16) == 0) {
        state->bridge_ready = true;
        snprintf(state->last_status, sizeof(state->last_status), "bridge ready");
    } else if (strncmp(line, "OK CH ", 6) == 0) {
        snprintf(state->last_status, sizeof(state->last_status), "%.95s", line);
    } else if (strncmp(line, "OK TX", 5) == 0 || strncmp(line, "ERR TX", 6) == 0) {
        snprintf(state->last_status, sizeof(state->last_status), "%.95s", line);
    } else if (strncmp(line, "OK SCAN START", 13) == 0) {
        state->scan_enabled = true;
        snprintf(state->last_status, sizeof(state->last_status), "passive scan on");
    } else if (strncmp(line, "OK SCAN STOP", 12) == 0) {
        state->scan_enabled = false;
        snprintf(state->last_status, sizeof(state->last_status), "passive scan off");
    } else if (strncmp(line, "SCAN ", 5) == 0) {
        snprintf(state->last_scan, sizeof(state->last_scan), "%.95s", line);
        if (sscanf(line, "SCAN %d RPD", &scan_channel) == 1 &&
            scan_channel >= 0 && scan_channel < RFNANO_CHANNELS) {
            state->scan_levels[scan_channel] = 12;
        }
    } else if (strncmp(line, "RX ", 3) == 0) {
        snprintf(state->last_rx, sizeof(state->last_rx), "%.95s", line);
        rx_payload[0] = '\0';
        if (sscanf(line, "RX %d %79s", &rx_length, rx_payload) == 2 && rx_length > 0) {
            snprintf(state->last_payload_hex, sizeof(state->last_payload_hex), "%.79s", rx_payload);
        }
    } else if (line[0]) {
        snprintf(state->last_status, sizeof(state->last_status), "%.95s", line);
    }
}

static void poll_bridge(RfNanoData *state) {
    if (state->fd < 0) {
        return;
    }

    char buffer[96];
    int bytes = Serial_ReadAvailable(state->fd, buffer, sizeof(buffer));
    if (bytes < 0) {
        close_bridge(state, "serial read failed");
        return;
    }

    for (int i = 0; i < bytes; ++i) {
        char ch = buffer[i];
        if (ch == '\r') {
            continue;
        }
        if (ch == '\n') {
            state->line_buffer[state->line_length] = '\0';
            handle_bridge_line(state, state->line_buffer);
            state->line_length = 0;
        } else if (state->line_length + 1 < sizeof(state->line_buffer)) {
            state->line_buffer[state->line_length++] = ch;
        } else {
            state->line_length = 0;
        }
    }
}

static const char *status_text(const RfNanoData *state) {
    if (state->count > 0) {
        return state->bridge_ready ? "RF-Nano bridge ready" : "serial candidate detected";
    }
    if (state->usb_adapter_present) {
        return "USB adapter seen, no tty";
    }
    return "no USB serial detected";
}

static bool rfnano_init(Module *module, App *app) {
    module->data = &data;
    memset(&data, 0, sizeof(data));
    data.fd = -1;
    data.channel = 76;
    snprintf(data.last_status, sizeof(data.last_status), "waiting for RF-Nano");
    refresh(&data);
    open_first_serial(&data, app->config.baud_rate);
    data.refresh_timer = 0.0f;
    data.poll_timer = 0.0f;
    (void)app;
    return true;
}

static void rfnano_update(Module *module, App *app, float dt) {
    (void)app;
    RfNanoData *state = module->data;
    state->refresh_timer += dt;
    state->poll_timer += dt;
    if (state->poll_timer > 0.05f) {
        poll_bridge(state);
        for (int i = 0; i < RFNANO_CHANNELS; ++i) {
            if (state->scan_levels[i] > 0) {
                state->scan_levels[i]--;
            }
        }
        state->poll_timer = 0.0f;
    }
    if (state->refresh_timer > 3.0f) {
        refresh(state);
        if (state->fd >= 0 && state->count == 0) {
            close_bridge(state, "serial disappeared");
        }
        open_first_serial(state, app->config.baud_rate);
        state->refresh_timer = 0.0f;
    }
}

static void rfnano_handle_event(Module *module, App *app, NavEvent event) {
    (void)app;
    RfNanoData *state = module->data;
    char command[96];

    if (event.type == NAV_LEFT && state->channel > 0) {
        state->channel--;
        snprintf(command, sizeof(command), "CH %d", state->channel);
        send_command(state, command);
    } else if (event.type == NAV_RIGHT && state->channel < 125) {
        state->channel++;
        snprintf(command, sizeof(command), "CH %d", state->channel);
        send_command(state, command);
    } else if (event.type == NAV_SELECT) {
        send_command(state, state->scan_enabled ? "SCAN STOP" : "SCAN START");
    } else if (event.type == NAV_DOWN && state->last_payload_hex[0]) {
        snprintf(command, sizeof(command), "TX %.79s", state->last_payload_hex);
        send_command(state, command);
    } else if (event.type == NAV_UP) {
        send_command(state, "STATUS");
    }
}

static void draw_scan_graph(Display *display, const RfNanoData *state, Color primary, Color secondary, Color accent) {
    int x = 20;
    int y = 210;
    int w = 440;
    int h = 46;
    int bar_w = 3;

    Display_DrawRect(display, x, y, w, h, secondary, false);
    for (int i = 0; i < RFNANO_CHANNELS; ++i) {
        int level = state->scan_levels[i];
        int bar_h = level <= 0 ? 1 : 3 + level * 3;
        if (bar_h > h - 4) {
            bar_h = h - 4;
        }
        int bx = x + 2 + (i * (w - 4)) / RFNANO_CHANNELS;
        int by = y + h - 2 - bar_h;
        Color color = level > 8 ? accent : (level > 0 ? primary : secondary);
        Display_DrawRect(display, bx, by, bar_w, bar_h, color, true);
    }
    Display_DrawText(display, "0", x, y + h + 4, secondary);
    Display_DrawText(display, "63", x + 212, y + h + 4, secondary);
    Display_DrawText(display, "125", x + 408, y + h + 4, secondary);
}

static void rfnano_shutdown(Module *module, App *app) {
    (void)app;
    RfNanoData *state = module->data;
    if (state) {
        close_bridge(state, "shutdown");
    }
}

static void rfnano_render(Module *module, App *app) {
    RfNanoData *state = module->data;
    char value[96];
    Renderer_Frame(&app->display, "RF-NANO NRF24", app->config.primary, app->config.secondary);
    Renderer_LabelValue(&app->display, 20, 58, "status", status_text(state), app->config.secondary, app->config.accent);
    snprintf(value, sizeof(value), "%d", app->config.baud_rate);
    Renderer_LabelValue(&app->display, 20, 86, "baud rate", value, app->config.secondary, app->config.accent);
    snprintf(value, sizeof(value), "%d", state->channel);
    Renderer_LabelValue(&app->display, 20, 114, "channel", value, app->config.secondary, app->config.accent);
    if (state->usb_adapter_present) {
        Renderer_LabelValue(&app->display, 20, 142, "usb", state->adapter_label, app->config.secondary, app->config.accent);
    }
    if (state->fd >= 0) {
        Renderer_LabelValue(&app->display, 20, 170, "serial", state->active_path, app->config.secondary, app->config.accent);
        Renderer_LabelValue(&app->display, 20, 198, "bridge", state->last_status, app->config.secondary, app->config.accent);
        draw_scan_graph(&app->display, state, app->config.primary, app->config.secondary, app->config.accent);
        Renderer_LabelValue(&app->display, 20, 270, "last rx", state->last_rx[0] ? state->last_rx : "none", app->config.secondary, app->config.accent);
        Display_DrawText(&app->display, "ENTER scan  DOWN replay bridge RX  UP status", 20, 292, app->config.secondary);
        return;
    }
    for (int i = 0; i < state->count && i < 4; ++i) {
        snprintf(value, sizeof(value), "device %d", i);
        Renderer_LabelValue(&app->display, 20, 170 + i * 24, value, state->devices[i].path, app->config.secondary, app->config.accent);
    }
    if (state->count == 0 && state->usb_count > 0) {
        Display_DrawText(&app->display, "USB inventory", 20, 170, app->config.primary);
        for (int i = 0; i < state->usb_count && i < 4; ++i) {
            Display_DrawTextClipped(&app->display, state->usb_lines[i], 20, 194 + i * 20, 436, app->config.secondary);
        }
    }
    Display_DrawText(&app->display, "flash RF-Nano bridge sketch for tx/rx", 20, 286, app->config.secondary);
}

Module Module_RfNano_Create(void) {
    return (Module){"RF-NANO nRF24", rfnano_init, rfnano_update, rfnano_render, rfnano_handle_event, rfnano_shutdown, NULL};
}
