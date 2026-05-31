#ifndef CYBERDECK_HOME_H
#define CYBERDECK_HOME_H

#include "core/screen_manager.h"

struct App;

void Home_HandleEvent(ScreenManager *manager, struct App *app, NavEvent event);
void Home_Render(ScreenManager *manager, struct App *app);

#endif
