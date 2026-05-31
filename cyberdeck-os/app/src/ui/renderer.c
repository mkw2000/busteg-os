#include "ui/renderer.h"
#include <stdio.h>

void Renderer_Frame(Display *display, const char *title, Color primary, Color secondary) {
    Display_DrawRect(display, 4, 4, DISPLAY_VIRTUAL_WIDTH - 8, DISPLAY_VIRTUAL_HEIGHT - 8, primary, false);
    Display_DrawLine(display, 4, 32, DISPLAY_VIRTUAL_WIDTH - 4, 32, secondary);
    Display_DrawText(display, title, 12, 10, primary);
}

void Renderer_LabelValue(Display *display, int x, int y, const char *label, const char *value, Color label_color, Color value_color) {
    char buffer[160];
    snprintf(buffer, sizeof(buffer), "%-14s", label);
    Display_DrawText(display, buffer, x, y, label_color);
    Display_DrawText(display, value, x + 132, y, value_color);
}
