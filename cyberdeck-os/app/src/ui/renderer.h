#ifndef CYBERDECK_RENDERER_H
#define CYBERDECK_RENDERER_H

#include "hal/display.h"

void Renderer_Frame(Display *display, const char *title, Color primary, Color secondary);
void Renderer_LabelValue(Display *display, int x, int y, const char *label, const char *value, Color label_color, Color value_color);

#endif
