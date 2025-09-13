#pragma once

#include "../common/platform/interface/display.hpp"
#include "../common/platform/interface/platform.hpp"
#include "../common/logging.hpp"
#include "../common/configuration.hpp"

#include "common_transitions.hpp"
#include "game_executor.hpp"

typedef struct Game2048Configuration {
        int grid_size;
        int target_max_tile;
} Game2048Configuration;

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

/**
 * Similar to `collect_configuration` from `configuration.hpp`, it returns true
 * if the configuration was successfully collected. Otherwise, if the user
 * requested exit by pressing the blue button, it returns false and this needs
 * to be handled by the main game loop.
 */
bool collect_2048_config(Platform *p, Game2048Configuration *game_config,
                                UserInterfaceCustomization *customization);

GameState *initialize_game_state(int gridSize, int target_max_tile);

void draw(Display *display, GameState *state);
void update_game_grid(Display *display, GameState *gs);

void initialize_randomness_seed(int seed);
bool is_game_over(GameState *gs);
bool is_game_finished(GameState *gs);
void take_turn(GameState *gs, int direction);

/**
 * Returns true if the user wants to play again. If they press blue on the
 * configuration screen it means that they want to exit, in which case this
 * function would return false.
 */
bool enter_2048_loop(Platform *platform, UserInterfaceCustomization *customization);

class Clean2048 : public GameExecutor
{
      public:
        void game_loop(Platform *p,
                             UserInterfaceCustomization *customization) override
        {
                while (enter_2048_loop(p, customization)) {
                        LOG_INFO("2048", "Finished the main 2048 game loop, "
                                         "pausing until input from the user.");
                        Direction dir;
                        Action act;
                        pause_until_input(p->directional_controllers,
                                          p->action_controllers, &dir, &act,
                                          p->delay_provider);

                        if (act == Action::BLUE) {
                                LOG_DEBUG("2048", "Exiting 2048 game loop.")
                                break;
                        }
                }
        }

        Clean2048() {}
};
