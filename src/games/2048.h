#ifndef GAME2048_H
#define GAME2048_H

#include "../common/platform/interface/display.hpp"
#include "../common/platform/interface/delay.hpp"
#include "../common/platform/interface/controller.hpp"

typedef enum ConfigOption {
        GRID_SIZE = 0,
        TARGET_MAX_TILE = 1,
        READY_TO_GO = 2,
} ConfigOption;

typedef struct GameConfiguration {
        int grid_size;
        int target_max_tile;
        ConfigOption config_option;
} GameConfiguration;

typedef struct GameState {
        int **grid;
        int **old_grid;
        int score;
        int occupied_tiles;
        int grid_size;
        int target_max_tile;
} GameState;

GameState *initialize_game_state(int gridSize, int target_max_tile);

void draw_game_canvas(Display *display, GameState *state);
void draw(Display *display, GameState *state);
void draw_game_over(Display *display, GameState *state);
void draw_game_won(Display *display, GameState *state);
void update_game_grid(Display *display, GameState *gs);

void initialize_randomness_seed(int seed);
bool is_game_over(GameState *gs);
bool is_game_finished(GameState *gs);
void take_turn(GameState *gs, int direction);

void enter_game_loop(Display *display, Controller *joystick_controller,
                     Controller *keypad_controller, DelayProvider *delay_provider);

#endif
