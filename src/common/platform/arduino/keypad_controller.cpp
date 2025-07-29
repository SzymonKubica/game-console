#include "keypad_controller.hpp"

bool KeypadController::poll_for_input(Action *input) {
        int leftButton = this->digital_read(LEFT_BUTTON_PIN);
        int downButton = this->digital_read(DOWN_BUTTON_PIN);
        int upButton = this->digital_read(UP_BUTTON_PIN);
        int rightButton = this->digital_read(RIGHT_BUTTON_PIN);

        if (!leftButton) {
                *input = Action::BLUE;
                return true;
        }
        if (!downButton) {
                *input = Action::GREEN;
                return true;
        }
        if (!upButton) {
                *input = Action::YELLOW;
                return true;
        }
        if (!rightButton) {
                *input = Action::RED;
                return true;
        }
        return false;
}

void KeypadController::setup() {
}
