#include "src/common/controller/controller.hpp"
#include "src/common/controller/joystick_controller.hpp"
#include "src/common/controller/keypad_controller.hpp"
#include "src/common/configuration.hpp"
#include "src/common/user_interface.h"
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

// This is supposed to be the generic function used for collecting the generic
// input. In the final state the `collect_game_configuration` is to be replaced
// by this. This is to be migrated to some c++ source file so that it can be
// reused for different games. The idea here is that each game would define the
// config struct that it needs to collect and the config menu would get rendered
// automatically.
void collect_generic_config(Configuration *config)
{
        LcdDisplay display = LcdDisplay{};
        // We start with an empty diff object
        ConfigurationDiff diff;
        render_generic_config_menu(&display, config, &diff, false);

        while (true) {
                Direction dir;
                bool input_registered = false;

                input_registered |= joystick_controller->poll_for_input(&dir);
                input_registered |= keypad_controller->poll_for_input(&dir);

                bool ready = false;
                if (input_registered) {
                        Configuration old_config;
                        switch (dir) {
                        case DOWN:
                                switch_edited_config_option_up(config, &diff);
                                break;
                        case UP:
                                switch_edited_config_option_down(config, &diff);
                                break;
                        case LEFT:
                                switch_current_config_option_down(config,
                                                                  &diff);
                                break;
                        case RIGHT:
                                if (config->current_config_value ==
                                    config->config_values_len - 1) {
                                        ready = true;
                                } else {
                                        switch_current_config_option_up(config,
                                                                        &diff);
                                }
                                break;
                        }

                        render_generic_config_menu(&display, config, &diff,
                                                   true);
                        delay(MOVE_REGISTERED_DELAY);
                        if (ready) {
                                break;
                        }
                }
                delay(INPUT_POLLING_DELAY);
        }
}
