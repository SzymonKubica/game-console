#include "input.h"

/* The joystick reports the current position using two potentiometers. Those
 * are read using analog pins that return values in range 0-1023. The two
 * constants below control how quickly the joystick registers input.
 */
#define HIGH_THRESHOLD 900
#define LOW_THRESHOLD 100

void checkJoystick(Direction *turn, bool *input_registered,
                   int (*analogRead)(unsigned char))
{
        int x_val = analogRead(STICK_X_PIN);
        int y_val = analogRead(STICK_Y_PIN);

        if (x_val < LOW_THRESHOLD) {
                *input_registered = true;
                *turn = Direction::RIGHT;
        } else if (x_val > HIGH_THRESHOLD) {
                *input_registered = true;
                *turn = Direction::LEFT;
        } else if (y_val < LOW_THRESHOLD) {
                *input_registered = true;
                *turn = Direction::UP;
        } else if (y_val > HIGH_THRESHOLD) {
                *input_registered = true;
                *turn = Direction::DOWN;
        }
}

void checkButtons(Direction *turn, bool *input_registered,
                  int (*digitalRead)(unsigned char))
{
        int leftButton = digitalRead(LEFT_BUTTON_PIN);
        int downButton = digitalRead(DOWN_BUTTON_PIN);
        int upButton = digitalRead(UP_BUTTON_PIN);
        int rightButton = digitalRead(RIGHT_BUTTON_PIN);

        if (!leftButton) {
                *turn = Direction::LEFT;
                *input_registered = true;
        }
        if (!downButton) {
                *turn = Direction::DOWN;
                *input_registered = true;
        }
        if (!upButton) {
                *turn = Direction::UP;
                *input_registered = true;
        }
        if (!rightButton) {
                *turn = Direction::RIGHT;
                *input_registered = true;
        }
}
