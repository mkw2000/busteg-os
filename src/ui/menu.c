#include "ui/menu.h"

void Menu_Render(Display *display, const char *const *items, int count, int selected, Color primary, Color secondary, Color accent) {
    for (int i = 0; i < count; ++i) {
        int y = 58 + i * 30;
        Color color = i == selected ? accent : primary;
        if (i == selected) {
            Display_DrawRect(display, 34, y - 4, 250, 24, secondary, false);
            Display_DrawText(display, ">", 44, y, accent);
        }
        Display_DrawText(display, items[i], 68, y, color);
    }
}
