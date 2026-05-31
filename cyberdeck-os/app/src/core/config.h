#ifndef CYBERDECK_CONFIG_H
#define CYBERDECK_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

typedef struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color;

typedef struct Config {
    bool fullscreen;
    int window_width;
    int window_height;
    int font_size;
    int baud_rate;
    Color background;
    Color primary;
    Color secondary;
    Color accent;
} Config;

void Config_Default(Config *config);
bool Config_Load(Config *config, const char *path);

#endif
