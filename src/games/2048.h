#ifndef GAME2048_H
#define GAME2048_H

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
void initialize_randomness_seed(int seed);
bool is_game_over(GameState *gs);
bool is_game_finished(GameState *gs);
void take_turn(GameState *gs, int direction);

#endif
