#pragma once
#include "game_executor.hpp"
#include "common_transitions.hpp"
#include "../common/logging.hpp"
#include "../common/configuration.hpp"

void enter_game_of_life_loop(Platform *platform,
                             GameCustomization *customization);

typedef struct GameOfLifeConfiguration {
        bool prepopulate_grid;
        // Simulation steps taken per second
        bool use_toroidal_array;
        int simulation_speed;
        int rewind_buffer_size;
} GameOfLifeConfiguration;

/**
 * Assembles the generic configuration struct that can be used to collect user
 * input specifying the game of life configuration.
 *
 * This is exposed publicly so
 * that the default game configuration saving module can call it, get the new
 * default setttings and save them in the persistent storage.
 */
Configuration *assemble_game_of_life_configuration(PersistentStorage *storage);

/**
 * Extracts the specific game of life config struct after the generic config was
 * collected from the user.
 *
 * This is exposed
 * publicly so that the default game configuration saving module can call it,
 * get the new default setttings and save them in the persistent storage.
 */
void extract_game_config(GameOfLifeConfiguration *game_config,
                         Configuration *config);

class GameOfLife : public GameExecutor
{
      public:
        void enter_game_loop(Platform *p,
                             GameCustomization *customization) override
        {
                while (true) {
                        enter_game_of_life_loop(p, customization);
                        Direction dir;
                        Action act;
                        pause_until_input(p->directional_controllers,
                                          p->action_controllers, &dir, &act,
                                          p->delay_provider);

                        if (act == Action::BLUE) {
                                LOG_DEBUG("game_of_life",
                                          "Exiting game of life loop.")
                                break;
                        }
                }
        }

        GameOfLife() {}
};
