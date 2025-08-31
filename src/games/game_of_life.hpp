#pragma once
#include "game_executor.hpp"
#include "common_transitions.hpp"
#include "../common/logging.hpp"
#include "../common/configuration.hpp"

void enter_game_of_life_loop(Platform *platform,
                             GameCustomization *customization);

typedef struct GameOfLifeConfiguration {
        bool prepopulate_grid;
        bool use_toroidal_array;
        /**
         * Simulation steps taken per second
         */
        int simulation_speed;
        /**
         * Controls how many steps the user is allowed to rewind the simulation
         */
        int rewind_buffer_size;
        /**
         * Randomness seed that is saved in the persistent storage. This is
         * needed because each time we run the game, `rand` begins initialized
         * with the same seed, this leads to the same pattern of cells generated
         * every time `prepopulate_grid` is used after booting up the console.
         * In order to avoid this, we 'spin' the randomness seed on each user
         * input when navigating around the game grid. This is predicated on the
         * assumption that users' input patterns will be random and ensure that
         * when we load the seed from the saved storage, the randomly populated
         * grid does not always look the same.
         */
        int randomness_seed;
} GameOfLifeConfiguration;

/**
 * Collects the game of life configuration from the user.
 *
 * This is exposed publicly so that the default game configuration saving module
 * can call it, get the new default setttings and save them in the persistent
 * storage.
 */
void collect_game_of_life_configuration(Platform *p,
                                        GameOfLifeConfiguration *game_config,
                                        GameCustomization *customization);

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
