#ifdef EMULTAOR
#include "../interface/delay.hpp"
#include <chrono>
#include <thread>

// TODO: rename the entire directory 'sfml' to 'emulator'
class EmulatorDelay : public DelayProvider
{
        void delay_ms(int ms) override
        {
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
};
#endif
