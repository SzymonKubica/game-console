#pragma once
#include "../common/platform/interface/platform.hpp"
#include "../common/user_interface_customization.hpp"

class GameExecutor
{
      public:
        virtual void game_loop(Platform *p,
                               UserInterfaceCustomization *customization) = 0;
};
