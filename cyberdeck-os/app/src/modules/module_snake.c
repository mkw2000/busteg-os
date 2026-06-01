#include "modules/module_snake.h"
#include "core/app.h"
#include "ui/renderer.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SNAKE_GRID_WIDTH 30
#define SNAKE_GRID_HEIGHT 18
#define SNAKE_CELL_SIZE 12
#define SNAKE_BOARD_X 60
#define SNAKE_BOARD_Y 54
#define SNAKE_START_LENGTH 5
#define SNAKE_MAX_LENGTH (SNAKE_GRID_WIDTH * SNAKE_GRID_HEIGHT)
#define SNAKE_STEP_SECONDS 0.16f
#define SNAKE_SWIPE_PIXELS 24

typedef enum SnakeDirection {
    SNAKE_UP,
    SNAKE_DOWN,
    SNAKE_LEFT,
    SNAKE_RIGHT
} SnakeDirection;

typedef struct SnakePoint {
    int x;
    int y;
} SnakePoint;

typedef struct SnakeData {
    SnakePoint body[SNAKE_MAX_LENGTH];
    SnakePoint food;
    int length;
    int score;
    SnakeDirection direction;
    SnakeDirection next_direction;
    float step_timer;
    unsigned int random_seed;
    bool game_over;
    bool pointer_down;
    int swipe_start_x;
    int swipe_start_y;
} SnakeData;

static SnakeData data;

static bool same_point(SnakePoint a, SnakePoint b) {
    return a.x == b.x && a.y == b.y;
}

static bool direction_is_opposite(SnakeDirection a, SnakeDirection b) {
    return (a == SNAKE_UP && b == SNAKE_DOWN) ||
           (a == SNAKE_DOWN && b == SNAKE_UP) ||
           (a == SNAKE_LEFT && b == SNAKE_RIGHT) ||
           (a == SNAKE_RIGHT && b == SNAKE_LEFT);
}

static unsigned int next_random(SnakeData *state) {
    state->random_seed = state->random_seed * 1103515245u + 12345u;
    return state->random_seed;
}

static bool snake_contains(const SnakeData *state, SnakePoint point) {
    for (int i = 0; i < state->length; i++) {
        if (same_point(state->body[i], point)) {
            return true;
        }
    }
    return false;
}

static void place_food(SnakeData *state) {
    for (int tries = 0; tries < 200; tries++) {
        SnakePoint candidate = {
            (int)(next_random(state) % SNAKE_GRID_WIDTH),
            (int)(next_random(state) % SNAKE_GRID_HEIGHT)
        };
        if (!snake_contains(state, candidate)) {
            state->food = candidate;
            return;
        }
    }

    /* Fallback for an almost-full board: scan until an empty cell is found. */
    for (int y = 0; y < SNAKE_GRID_HEIGHT; y++) {
        for (int x = 0; x < SNAKE_GRID_WIDTH; x++) {
            SnakePoint candidate = {x, y};
            if (!snake_contains(state, candidate)) {
                state->food = candidate;
                return;
            }
        }
    }
}

static void snake_reset(SnakeData *state) {
    state->length = SNAKE_START_LENGTH;
    state->score = 0;
    state->direction = SNAKE_RIGHT;
    state->next_direction = SNAKE_RIGHT;
    state->step_timer = 0.0f;
    state->random_seed = 0xB057E6u;
    state->game_over = false;
    state->pointer_down = false;

    int start_x = SNAKE_GRID_WIDTH / 2;
    int start_y = SNAKE_GRID_HEIGHT / 2;
    for (int i = 0; i < state->length; i++) {
        state->body[i] = (SnakePoint){start_x - i, start_y};
    }

    place_food(state);
}

static bool snake_hits_itself(const SnakeData *state, SnakePoint head, bool ate_food) {
    /*
     * If no food was eaten, the last tail cell moves away this step. Moving
     * into that old tail square is allowed in classic Snake.
     */
    int cells_to_check = ate_food ? state->length : state->length - 1;
    for (int i = 0; i < cells_to_check; i++) {
        if (same_point(state->body[i], head)) {
            return true;
        }
    }
    return false;
}

static SnakePoint moved_head(SnakePoint head, SnakeDirection direction) {
    if (direction == SNAKE_UP) head.y--;
    else if (direction == SNAKE_DOWN) head.y++;
    else if (direction == SNAKE_LEFT) head.x--;
    else if (direction == SNAKE_RIGHT) head.x++;
    return head;
}

static void snake_step(SnakeData *state) {
    state->direction = state->next_direction;
    SnakePoint new_head = moved_head(state->body[0], state->direction);
    bool ate_food = same_point(new_head, state->food);

    if (new_head.x < 0 || new_head.x >= SNAKE_GRID_WIDTH ||
        new_head.y < 0 || new_head.y >= SNAKE_GRID_HEIGHT ||
        snake_hits_itself(state, new_head, ate_food)) {
        state->game_over = true;
        return;
    }

    if (ate_food && state->length < SNAKE_MAX_LENGTH) {
        state->length++;
        state->score++;
    }

    for (int i = state->length - 1; i > 0; i--) {
        state->body[i] = state->body[i - 1];
    }
    state->body[0] = new_head;

    if (ate_food) {
        place_food(state);
    }
}

static void request_direction(SnakeData *state, SnakeDirection direction) {
    if (!direction_is_opposite(direction, state->direction)) {
        state->next_direction = direction;
    }
}

static void handle_swipe(SnakeData *state, int x, int y) {
    int dx = x - state->swipe_start_x;
    int dy = y - state->swipe_start_y;
    int abs_dx = abs(dx);
    int abs_dy = abs(dy);

    if (abs_dx < SNAKE_SWIPE_PIXELS && abs_dy < SNAKE_SWIPE_PIXELS) {
        return;
    }

    if (abs_dx > abs_dy) {
        request_direction(state, dx > 0 ? SNAKE_RIGHT : SNAKE_LEFT);
    } else {
        request_direction(state, dy > 0 ? SNAKE_DOWN : SNAKE_UP);
    }

    state->swipe_start_x = x;
    state->swipe_start_y = y;
}

static bool snake_init(Module *module, App *app) {
    (void)app;
    module->data = &data;
    snake_reset(&data);
    return true;
}

static void snake_update(Module *module, App *app, float dt) {
    (void)app;
    SnakeData *state = module->data;
    if (state->game_over) {
        return;
    }

    state->step_timer += dt;
    while (state->step_timer >= SNAKE_STEP_SECONDS) {
        snake_step(state);
        state->step_timer -= SNAKE_STEP_SECONDS;
    }
}

static void snake_event(Module *module, App *app, NavEvent event) {
    (void)app;
    SnakeData *state = module->data;

    if (event.type == NAV_SELECT && state->game_over) {
        snake_reset(state);
        return;
    }
    if (event.type == NAV_POINTER && event.pressed && state->game_over) {
        snake_reset(state);
        return;
    }

    if (event.type == NAV_UP) request_direction(state, SNAKE_UP);
    else if (event.type == NAV_DOWN) request_direction(state, SNAKE_DOWN);
    else if (event.type == NAV_LEFT) request_direction(state, SNAKE_LEFT);
    else if (event.type == NAV_RIGHT) request_direction(state, SNAKE_RIGHT);
    else if (event.type == NAV_POINTER) {
        if (event.pressed && !state->pointer_down) {
            state->swipe_start_x = event.x;
            state->swipe_start_y = event.y;
        } else if (event.pressed) {
            handle_swipe(state, event.x, event.y);
        }
        state->pointer_down = event.pressed;
    }
}

static void draw_cell(App *app, int grid_x, int grid_y, Color color, bool filled) {
    Display_DrawRect(&app->display,
                     SNAKE_BOARD_X + grid_x * SNAKE_CELL_SIZE,
                     SNAKE_BOARD_Y + grid_y * SNAKE_CELL_SIZE,
                     SNAKE_CELL_SIZE - 1,
                     SNAKE_CELL_SIZE - 1,
                     color,
                     filled);
}

static void snake_render(Module *module, App *app) {
    SnakeData *state = module->data;
    char text[64];
    Renderer_Frame(&app->display, "SNAKE", app->config.primary, app->config.secondary);

    snprintf(text, sizeof(text), "score %d", state->score);
    Renderer_LabelValue(&app->display, 24, 34, "snake", text, app->config.secondary, app->config.accent);

    Display_DrawRect(&app->display,
                     SNAKE_BOARD_X - 2,
                     SNAKE_BOARD_Y - 2,
                     SNAKE_GRID_WIDTH * SNAKE_CELL_SIZE + 3,
                     SNAKE_GRID_HEIGHT * SNAKE_CELL_SIZE + 3,
                     app->config.secondary,
                     false);

    draw_cell(app, state->food.x, state->food.y, app->config.accent, true);
    for (int i = state->length - 1; i >= 0; i--) {
        Color color = i == 0 ? app->config.accent : app->config.primary;
        draw_cell(app, state->body[i].x, state->body[i].y, color, true);
    }

    if (state->game_over) {
        Display_DrawText(&app->display, "GAME OVER", 190, 136, app->config.accent);
        Display_DrawText(&app->display, "ENTER/tap to restart", 158, 164, app->config.primary);
    } else {
        Display_DrawText(&app->display, "swipe or arrows/WASD", 150, 286, app->config.secondary);
    }
}

Module Module_Snake_Create(void) {
    return (Module){"Snake", snake_init, snake_update, snake_render, snake_event, NULL, NULL};
}
