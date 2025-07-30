#pragma once
#include "game_executor.hpp"
#include "common_transitions.hpp"
#include "../common/logging.hpp"

void enter_minesweeper_loop(Platform *platform,
                            GameCustomization *customization);

class Minesweeper : public GameExecutor
{
      public:
        void enter_game_loop(Platform *p,
                             GameCustomization *customization) override
        {
                while (true) {
                        enter_minesweeper_loop(p, customization);
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
