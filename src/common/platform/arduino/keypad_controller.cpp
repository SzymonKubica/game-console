#include "keypad_controller.hpp"

bool KeypadController::poll_for_input(Direction *input) {
        int leftButton = this->digital_read(LEFT_BUTTON_PIN);
        int downButton = this->digital_read(DOWN_BUTTON_PIN);
        int upButton = this->digital_read(UP_BUTTON_PIN);
        int rightButton = this->digital_read(RIGHT_BUTTON_PIN);

        if (!leftButton) {
                *input = Direction::LEFT;
                return true;
        }
        if (!downButton) {
                *input = Direction::DOWN;
                return true;
        }
        if (!upButton) {
                *input = Direction::UP;
                return true;
        }
        if (!rightButton) {
                *input = Direction::RIGHT;
                return true;
        }
        return false;
}

void KeypadController::setup() {
}
