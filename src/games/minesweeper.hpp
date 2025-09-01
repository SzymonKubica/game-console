#pragma once
#include "game_executor.hpp"
#include "common_transitions.hpp"
#include "../common/logging.hpp"

typedef struct MinesweeperConfiguration {
        int mines_num;
} MinesweeperConfiguration;

/**
 * Returns true if the user wants to play again. If they press blue on the
 * configuration screen it means that they want to exit, in which case this
 * function would return false.
 */
bool enter_minesweeper_loop(Platform *platform,
                            GameCustomization *customization);

/**
 * Similar to `collect_configuration` from `configuration.hpp`, it returns true
 * if the configuration was successfully collected. Otherwise, if the user
 * requested exit by pressing the blue button, it returns false and this needs
 * to be handled by the main game loop.
 */
bool collect_minesweeper_configuration(Platform *p,
                                       MinesweeperConfiguration *game_config,
                                       GameCustomization *customization);

class Minesweeper : public GameExecutor
{
      public:
        void enter_game_loop(Platform *p,
                             GameCustomization *customization) override
        {
                while (enter_minesweeper_loop(p, customization)) {
                        LOG_DEBUG(
                            "minesweeper",
                            "Minesweeper game loop finished. Pausing for input "
                            "to allow the user to view the finished grid.")
                        Direction dir;
                        Action act;
                        pause_until_input(p->directional_controllers,
                                          p->action_controllers, &dir, &act,
                                          p->delay_provider);

                        if (act == Action::BLUE) {
                                break;
                        }
                }
        }

        Minesweeper() {}
};
