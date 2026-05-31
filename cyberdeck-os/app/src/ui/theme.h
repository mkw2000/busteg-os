#ifndef CYBERDECK_THEME_H
#define CYBERDECK_THEME_H

#include "core/config.h"

typedef struct Theme {
    Color background;
    Color primary;
    Color secondary;
    Color accent;
} Theme;

Theme Theme_FromConfig(const Config *config);

#endif
