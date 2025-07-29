#pragma once
#include "game_executor.hpp"

void enter_minesweeper_loop(Platform *platform,
                            GameCustomization *customization);

class Minesweeper : public GameExecutor
{
      public:
        void enter_game_loop(Platform *p,
                             GameCustomization *customization) override
        {
                enter_minesweeper_loop(p, customization);
        }

        Minesweeper() {}
};
