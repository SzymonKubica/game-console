#include "joystick_controller.hpp"
#include "../compatibility.hpp"

bool JoystickController::poll_for_input(Direction *input)
{

        int x_val = this->analog_read(STICK_X_PIN);
        int y_val = this->analog_read(STICK_Y_PIN);

        if (x_val < LOW_THRESHOLD) {
                *input = Direction::RIGHT;
                return true;
        }
        if (x_val > HIGH_THRESHOLD) {
                *input = Direction::LEFT;
                return true;
        }
        if (y_val < LOW_THRESHOLD) {
                *input = Direction::UP;
                return true;
        }
        if (y_val > HIGH_THRESHOLD) {
                *input = Direction::DOWN;
                return true;
        }
        return false;
}

void JoystickController::setup() { this->pin_mode(STICK_BUTTON_PIN, INPUT); }
