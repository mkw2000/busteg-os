#include "ui/theme.h"

Theme Theme_FromConfig(const Config *config) {
    Theme theme;
    theme.background = config->background;
    theme.primary = config->primary;
    theme.secondary = config->secondary;
    theme.accent = config->accent;
    return theme;
}
