#pragma once
#include "../common/platform/interface/platform.hpp"
#include "../common/user_interface.hpp"



class GameExecutor
{
      public:
        virtual void
        enter_game_loop(Platform *p,
                        UserInterfaceCustomization *customization) = 0;
};
