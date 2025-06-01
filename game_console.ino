#include "src/common/controller/controller.hpp"
#include "src/common/controller/joystick_controller.hpp"
#include "src/common/controller/keypad_controller.hpp"
#include "src/common/display/display.hpp"
#include "src/common/display/lcd_display.hpp"
#include "src/common/timing/delay.hpp"

#include "src/games/2048.h"

LcdDisplay display;
JoystickController *joystick_controller;
KeypadController *keypad_controller;

class ArduinoDelay : public DelayProvider
{
        void delay_ms(int ms) override { delay(ms); }
};

void setup(void)
{
        // Initialise serial port for debugging
        Serial.begin(115200);

        // Set up controllers
        pinMode(STICK_BUTTON_PIN, INPUT);
        joystick_controller =
            new JoystickController((int (*)(unsigned char))&analogRead),

        pinMode(LEFT_BUTTON_PIN, INPUT);
        pinMode(DOWN_BUTTON_PIN, INPUT);
        pinMode(UP_BUTTON_PIN, INPUT);
        pinMode(RIGHT_BUTTON_PIN, INPUT);
        keypad_controller =
            new KeypadController((int (*)(unsigned char))&digitalRead),

        // Initialize the hardware LCD display
            display = LcdDisplay{};
        display.setup();

        // Initializes the source of randomness from the
        // noise present on the first digital pin
        initialize_randomness_seed(analogRead(0));
}

void loop(void)
{
        enter_game_loop(&display, joystick_controller, keypad_controller,
                        new ArduinoDelay());
}
