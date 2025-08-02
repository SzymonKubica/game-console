#pragma once
#include "game_executor.hpp"
#include "common_transitions.hpp"
#include "../common/logging.hpp"

void enter_game_of_life_loop(Platform *platform,
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
