
#define STICK_Y_PIN 16
#define STICK_X_PIN 17

#define LEFT_BUTTON_PIN 9
#define DOWN_BUTTON_PIN 15
#define UP_BUTTON_PIN 8
#define RIGHT_BUTTON_PIN 12

typedef enum Direction {
        UP = 0,
        RIGHT = 1,
        DOWN = 2,
        LEFT = 3
} Direction;

// Checks whether the joystick is registering user input at the moment.
// If it is, it sets the `input_registered` flag to true and returns the
// registered input through the output parameter `turn`. The last argument is
// the function for reading analog input that is only available inside of the
// *.ino files and so needs to be passed into the cpp file as a function pointer.
void checkJoystickInput(Direction *input, bool *input_registered, int (*analogRead)(unsigned char));

// Checks whether the buttons are registering user input at the moment.
// If it is, it sets the `input_registered` flag to true and returns the
// registered input through the output parameter `turn`. The last argument is
// the function for reading analog input that is only available inside of the
// *.ino files and so needs to be passed into the cpp file as a function pointer.
void checkButtonsInput(Direction *input, bool *input_registered, int (*digitalRead)(unsigned char));


