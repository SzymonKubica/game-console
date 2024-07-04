int checkJoystick(int *turn, bool *input_registered)
{
        int x_val = analogRead(STICK_X_PIN);
        int y_val = analogRead(STICK_Y_PIN);

        if (x_val < 100) {
                *input_registered = true;
                *turn = RIGHT;
        }

        if (x_val > 900) {
                *input_registered = true;
                *turn = LEFT;
        }

        if (y_val < 100) {
                *input_registered = true;
                *turn = UP;
        }

        if (y_val > 900) {
                *input_registered = true;
                *turn = DOWN;
        }
}

int checkButtons(int *turn, bool *inputRegistered)
{
        int leftButton = digitalRead(LEFT_BUTTON_PIN);
        int downButton = digitalRead(DOWN_BUTTON_PIN);
        int upButton = digitalRead(UP_BUTTON_PIN);
        int rightButton = digitalRead(RIGHT_BUTTON_PIN);

        if (!leftButton) {
                *turn = LEFT;
                *inputRegistered = true;
        }
        if (!downButton) {
                *turn = DOWN;
                *inputRegistered = true;
        }
        if (!upButton) {
                *turn = UP;
                *inputRegistered = true;
        }
        if (!rightButton) {
                *turn = RIGHT;
                *inputRegistered = true;
        }
}
