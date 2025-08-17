#pragma once
#include "game_executor.hpp"
#include "common_transitions.hpp"
#include "../common/logging.hpp"
#include "../common/configuration.hpp"

typedef struct MinesweeperConfiguration {
        int mines_num;
} MinesweeperConfiguration;

void enter_minesweeper_loop(Platform *platform,
                            GameCustomization *customization);

void collect_minesweeper_configuration(Platform *p,
                                       MinesweeperConfiguration *game_config,
                                       GameCustomization *customization);

class Minesweeper : public GameExecutor
{
      public:
        void enter_game_loop(Platform *p,
                             GameCustomization *customization) override
        {
                while (true) {
                        try {
                                enter_minesweeper_loop(p, customization);
                        } catch (const ConfigurationLoopExitException &e) {
                                LOG_INFO("minesweeper", "User requested exit in the game "
                                              "config loop.");
                                return;
                        }
                        Direction dir;
                        Action act;
                        pause_until_input(p->directional_controllers,
                                          p->action_controllers, &dir, &act,
                                          p->delay_provider);

                        if (act == Action::BLUE) {
                                LOG_DEBUG("minesweeper",
                                          "Exiting Minesweeper game loop.")
                                break;
                        }
                }
        }

        Minesweeper() {}
};
