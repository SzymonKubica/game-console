#include "input.h"
#include "game2048.h"
#include "user_interface.h"

/*
TODO items:
- clean up the display logic
*/

#define INPUT_POLLING_DELAY 50
#define MOVE_REGISTERED_DELAY 150

GameState *state;

void setup(void)
{
        // Initialise serial port for debugging
        Serial.begin(115200);

        // Set up button pins as inputs
        pinMode(LEFT_BUTTON_PIN, INPUT);
        pinMode(DOWN_BUTTON_PIN, INPUT);
        pinMode(UP_BUTTON_PIN, INPUT);
        pinMode(RIGHT_BUTTON_PIN, INPUT);

        initializeDisplay();

        // Initializes the source of randomness from the
        // noise present on the first digital pin
        initializeRandomnessSeed(analogRead(0));

        state = initializeGameState(4);
}

void loop(void)
{
        draw(state);
        drawGameCanvas(state);
        drawGameGrid(state);

        while (!isGameOver(state)) {
                Direction dir;
                bool input_registered = false;
                checkJoystickInput(&dir, &input_registered,
                                   (int (*)(unsigned char)) & analogRead);
                checkButtonsInput(&dir, &input_registered,
                                  (int (*)(unsigned char)) & digitalRead);

                if (input_registered) {
                        takeTurn(state, (int)dir);
                        drawGameGrid(state);
                        delay(MOVE_REGISTERED_DELAY);
                }
                delay(INPUT_POLLING_DELAY);
        }
        drawGameOver(state);
}
