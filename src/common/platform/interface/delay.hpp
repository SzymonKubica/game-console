#pragma once

#define INPUT_POLLING_DELAY 50
#define MOVE_REGISTERED_DELAY 150

class DelayProvider
{
      public:
        virtual void delay_ms(int ms) = 0;
};
