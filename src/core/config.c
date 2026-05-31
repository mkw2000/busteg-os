#include "core/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool parse_bool(const char *value) {
    return strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, "yes") == 0;
}

static Color parse_color(const char *value, Color fallback) {
    int r = 0;
    int g = 0;
    int b = 0;
    if (sscanf(value, "%d,%d,%d", &r, &g, &b) == 3) {
        Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b, 255};
        return color;
    }
    return fallback;
}

void Config_Default(Config *config) {
    config->fullscreen = false;
    config->window_width = 800;
    config->window_height = 480;
    config->font_size = 16;
    config->baud_rate = 115200;
    config->background = (Color){0, 0, 0, 255};
    config->primary = (Color){0, 255, 90, 255};
    config->secondary = (Color){92, 106, 100, 255};
    config->accent = (Color){255, 255, 255, 255};
}

bool Config_Load(Config *config, const char *path) {
    Config_Default(config);
    FILE *file = fopen(path, "r");
    if (!file) {
        return false;
    }

    char line[192];
    while (fgets(line, sizeof(line), file)) {
        char *eq = strchr(line, '=');
        if (!eq || line[0] == '[' || line[0] == '#') {
            continue;
        }
        *eq = '\0';
        char *key = line;
        char *value = eq + 1;
        value[strcspn(value, "\r\n")] = '\0';

        if (strcmp(key, "fullscreen") == 0) config->fullscreen = parse_bool(value);
        else if (strcmp(key, "window_width") == 0) config->window_width = atoi(value);
        else if (strcmp(key, "window_height") == 0) config->window_height = atoi(value);
        else if (strcmp(key, "font_size") == 0) config->font_size = atoi(value);
        else if (strcmp(key, "baud_rate") == 0) config->baud_rate = atoi(value);
        else if (strcmp(key, "background") == 0) config->background = parse_color(value, config->background);
        else if (strcmp(key, "primary") == 0) config->primary = parse_color(value, config->primary);
        else if (strcmp(key, "secondary") == 0) config->secondary = parse_color(value, config->secondary);
        else if (strcmp(key, "accent") == 0) config->accent = parse_color(value, config->accent);
    }

    fclose(file);
    return true;
}
