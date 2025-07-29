#pragma once
#include "../common/platform/interface/platform.hpp"

typedef struct GameCustomization {
        Color accent_color;
} GameCustomization;

class GameExecutor
{
      public:
        virtual void enter_game_loop(Platform *p,
                                     GameCustomization *customization) = 0;
};
