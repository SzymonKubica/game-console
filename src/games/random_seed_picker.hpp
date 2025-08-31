#pragma once
#include "game_executor.hpp"
#include "common_transitions.hpp"
#include "../common/logging.hpp"
#include "../common/configuration.hpp"

void enter_random_seed_picker_loop(Platform *platform,
                                   GameCustomization *customization);

typedef struct RandomSeedPickerConfiguration {
        // Starting point of the seed
        int base_seed;
        // Number of times we call `rand` with the initial seed and then set
        // it with `srand`. Allows fora wider range of random seeds
        int reseed_iterations;
} RandomSeedPickerConfiguration;

/**
 * Collects the game of life configuration from the user.
 *
 * This is exposed publicly so that the default game configuration saving module
 * can call it, get the new default setttings and save them in the persistent
 * storage.
 */
void collect_random_seed_picker_configuration(
    Platform *p, RandomSeedPickerConfiguration *game_config,
    GameCustomization *customization);

class RandomSeedPicker : public GameExecutor
{
      public:
        void enter_game_loop(Platform *p,
                             GameCustomization *customization) override
        {
                while (true) {
                        enter_random_seed_picker_loop(p, customization);
                        Direction dir;
                        Action act;
                        pause_until_input(p->directional_controllers,
                                          p->action_controllers, &dir, &act,
                                          p->delay_provider);

                        if (act == Action::BLUE) {
                                LOG_DEBUG("random_seed_picker",
                                          "Exiting random seed picker loop.")
                                break;
                        }
                }
        }

        RandomSeedPicker() {}
};
