#include "../interface/delay.hpp"
class ArduinoDelay : public DelayProvider
{
        void delay_ms(int ms) override { delay(ms); }
};
