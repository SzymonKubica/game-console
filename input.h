typedef enum Direction {
        UP,
        DOWN,
        LEFT,
        RIGHT
} Direction;

// Checks whether the joystick is registering user input at the moment.
// If it is, it sets the `input_registered` flag to true and returns the
// registered input through the output parameter `turn`.
void checkJoystick(Direction *input, bool *input_registered);
void checkButtons(Direction *input, bool *input_registered);


