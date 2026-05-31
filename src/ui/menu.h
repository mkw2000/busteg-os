#ifndef CYBERDECK_MENU_H
#define CYBERDECK_MENU_H

#include "hal/display.h"

void Menu_Render(Display *display, const char *const *items, int count, int selected, Color primary, Color secondary, Color accent);

#endif
