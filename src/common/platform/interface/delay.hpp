#pragma once

class DelayProvider
{
      public:
        virtual void delay_ms(int ms) = 0;
};
