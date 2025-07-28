#include "src/common/platform/arduino/joystick_controller.hpp"
#include "src/common/platform/arduino/keypad_controller.hpp"
#include "src/common/platform/arduino/lcd_display.hpp"
#include "src/common/platform/arduino/arduino_delay.cpp"

#include "src/games/game_menu.hpp"
#include "src/games/2048.hpp"

LcdDisplay display;
JoystickController *joystick_controller;
KeypadController *keypad_controller;

void setup(void)
{
        // Initialise serial port for debugging
        Serial.begin(115200);

        // Set up controllers
        pinMode(STICK_BUTTON_PIN, INPUT);
        joystick_controller =
            new JoystickController((int (*)(unsigned char))&analogRead);

        pinMode(LEFT_BUTTON_PIN, INPUT);
        pinMode(DOWN_BUTTON_PIN, INPUT);
        pinMode(UP_BUTTON_PIN, INPUT);
        pinMode(RIGHT_BUTTON_PIN, INPUT);
        keypad_controller =
            new KeypadController((int (*)(unsigned char))&digitalRead);

        // Initialize the hardware LCD display
        display = LcdDisplay{};
        display.setup();

        // Initializes the source of randomness from the
        // noise present on the first digital pin
        // Todo: move this to some randomness provider as it is not specific
        // to the 2048 game.
        initialize_randomness_seed(analogRead(0));
}

void loop(void)
{
        std::vector<Controller *> controllers(2);
        controllers[0] = joystick_controller;
        controllers[1] = keypad_controller;

        DelayProvider *delay_provider = new ArduinoDelay((void (*)(int))&delay);

        Platform platform = {.display = &display,
                             .controllers = &controllers,
                             .delay_provider = delay_provider};
        select_game(&platform);
}
