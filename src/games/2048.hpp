#pragma once

#include "../common/platform/interface/display.hpp"
#include "../common/platform/interface/platform.hpp"
#include "game_executor.hpp"

typedef struct GameConfiguration {
        int grid_size;
        int target_max_tile;
} GameConfiguration;

class GameState
{
      public:
        int **grid;
        int **old_grid;
        int score;
        int occupied_tiles;
        int grid_size;
        int target_max_tile;

        GameState(int **grid, int **old_grid, int score, int occupied_tiles,
                  int grid_size, int target_max_tile)
            : grid(grid), old_grid(old_grid), score(score),
              occupied_tiles(occupied_tiles), grid_size(grid_size),
              target_max_tile(target_max_tile)
        {
        }
};

GameState *initialize_game_state(int gridSize, int target_max_tile);

void draw(Display *display, GameState *state);
void update_game_grid(Display *display, GameState *gs);

void initialize_randomness_seed(int seed);
bool is_game_over(GameState *gs);
bool is_game_finished(GameState *gs);
void take_turn(GameState *gs, int direction);

void enter_2048_loop(Platform *platform, GameCustomization *customization);

class Clean2048 : public GameExecutor
{
      public:
        void enter_game_loop(Platform *p,
                             GameCustomization *customization) override
        {
                enter_2048_loop(p, customization);
        }

        Clean2048() {}
};
