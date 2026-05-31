#include "hal/display.h"
#include <stdio.h>
#include <string.h>

static const char *font_candidates[] = {
    "assets/font.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
    "/usr/share/fonts/dejavu/DejaVuSansMono.ttf",
    "C:/Windows/Fonts/consola.ttf",
    "C:/Windows/Fonts/lucon.ttf"
};

static SDL_Color to_sdl_color(Color color) {
    return (SDL_Color){color.r, color.g, color.b, color.a};
}

static TTF_Font *open_font(int size) {
    for (size_t i = 0; i < sizeof(font_candidates) / sizeof(font_candidates[0]); ++i) {
        TTF_Font *font = TTF_OpenFont(font_candidates[i], size);
        if (font) {
            return font;
        }
    }
    return NULL;
}

bool Display_Init(Display *display, const Config *config) {
    memset(display, 0, sizeof(*display));
    display->window_width = config->window_width;
    display->window_height = config->window_height;
    display->font_size = config->font_size;
    display->fullscreen = config->fullscreen;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }
    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return false;
    }

    Uint32 flags = SDL_WINDOW_SHOWN;
    if (display->fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    display->window = SDL_CreateWindow("BUSTEG OS",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       display->window_width,
                                       display->window_height,
                                       flags);
    if (!display->window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        Display_Shutdown(display);
        return false;
    }

    display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!display->renderer) {
        display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!display->renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        Display_Shutdown(display);
        return false;
    }

    display->target = SDL_CreateTexture(display->renderer,
                                        SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET,
                                        DISPLAY_VIRTUAL_WIDTH,
                                        DISPLAY_VIRTUAL_HEIGHT);
    if (!display->target) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        Display_Shutdown(display);
        return false;
    }

    display->font = open_font(display->font_size);
    if (!display->font) {
        fprintf(stderr, "No monospace TTF font found. Install DejaVuSansMono or set assets/font.ttf.\n");
        Display_Shutdown(display);
        return false;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    return true;
}

void Display_Shutdown(Display *display) {
    if (display->font) TTF_CloseFont(display->font);
    if (display->target) SDL_DestroyTexture(display->target);
    if (display->renderer) SDL_DestroyRenderer(display->renderer);
    if (display->window) SDL_DestroyWindow(display->window);
    TTF_Quit();
    SDL_Quit();
}

void Display_BeginFrame(Display *display) {
    SDL_SetRenderTarget(display->renderer, display->target);
    SDL_SetRenderDrawColor(display->renderer, 0, 0, 0, 255);
    SDL_RenderClear(display->renderer);
}

void Display_EndFrame(Display *display) {
    SDL_SetRenderTarget(display->renderer, NULL);
    int win_w = 0;
    int win_h = 0;
    SDL_GetWindowSize(display->window, &win_w, &win_h);

    float scale_x = (float)win_w / (float)DISPLAY_VIRTUAL_WIDTH;
    float scale_y = (float)win_h / (float)DISPLAY_VIRTUAL_HEIGHT;
    float scale = scale_x < scale_y ? scale_x : scale_y;
    int out_w = (int)((float)DISPLAY_VIRTUAL_WIDTH * scale);
    int out_h = (int)((float)DISPLAY_VIRTUAL_HEIGHT * scale);
    SDL_Rect dst = {(win_w - out_w) / 2, (win_h - out_h) / 2, out_w, out_h};

    SDL_SetRenderDrawColor(display->renderer, 0, 0, 0, 255);
    SDL_RenderClear(display->renderer);
    SDL_RenderCopy(display->renderer, display->target, NULL, &dst);
    SDL_RenderPresent(display->renderer);
}

void Display_DrawText(Display *display, const char *text, int x, int y, Color color) {
    if (!text || !display->font) {
        return;
    }
    SDL_Surface *surface = TTF_RenderUTF8_Blended(display->font, text, to_sdl_color(color));
    if (!surface) {
        return;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(display->renderer, surface);
    if (texture) {
        SDL_Rect dst = {x, y, surface->w, surface->h};
        SDL_RenderCopy(display->renderer, texture, NULL, &dst);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void Display_DrawRect(Display *display, int x, int y, int w, int h, Color color, bool filled) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(display->renderer, color.r, color.g, color.b, color.a);
    if (filled) {
        SDL_RenderFillRect(display->renderer, &rect);
    } else {
        SDL_RenderDrawRect(display->renderer, &rect);
    }
}

void Display_DrawLine(Display *display, int x1, int y1, int x2, int y2, Color color) {
    SDL_SetRenderDrawColor(display->renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(display->renderer, x1, y1, x2, y2);
}
