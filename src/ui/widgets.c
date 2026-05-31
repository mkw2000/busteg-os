#include "ui/widgets.h"

void Widgets_Progress(Display *display, int x, int y, int w, int h, float progress, Color color) {
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;
    Display_DrawRect(display, x, y, w, h, color, false);
    Display_DrawRect(display, x + 2, y + 2, (int)((w - 4) * progress), h - 4, color, true);
}
