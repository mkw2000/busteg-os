#ifndef CYBERDECK_DISPLAY_H
#define CYBERDECK_DISPLAY_H

#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "core/config.h"

#define DISPLAY_VIRTUAL_WIDTH 480
#define DISPLAY_VIRTUAL_HEIGHT 320

typedef struct Display {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *target;
    TTF_Font *font;
    int window_width;
    int window_height;
    int font_size;
    bool fullscreen;
} Display;

bool Display_Init(Display *display, const Config *config);
void Display_Shutdown(Display *display);
void Display_BeginFrame(Display *display);
void Display_EndFrame(Display *display);
void Display_DrawText(Display *display, const char *text, int x, int y, Color color);
void Display_DrawRect(Display *display, int x, int y, int w, int h, Color color, bool filled);
void Display_DrawLine(Display *display, int x1, int y1, int x2, int y2, Color color);

#endif
