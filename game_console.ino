#include "GUI_Paint.h"
#include "LCD_Driver.h"
#include "game2048.h"
#include <SPI.h>

/*

TODO items:
- clean up and decouple the display control logic.

*/

/*******************************************************************************
  Constants
*******************************************************************************/

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#define STICK_Y_PIN 16
#define STICK_X_PIN 17

#define LEFT_BUTTON_PIN 9
#define DOWN_BUTTON_PIN 15
#define UP_BUTTON_PIN 8
#define RIGHT_BUTTON_PIN 12

#define INPUT_POLLING_DELAY 50
#define MOVE_REGISTERED_DELAY 150


GameState *state;
int old_grid[4][4];

/*******************************************************************************
  Setup and Main Loop
*******************************************************************************/

void setup(void)
{
        // Initialise the display
        Config_Init();
        LCD_Init();
        LCD_SetBacklight(100);

        // Initialise serial port for debugging
        Serial.begin(115200);

        // Initializes the source of randomness from the noise present on the
        // first digital pin
        initializeRandomnessSeed(analogRead(0));


        pinMode(LEFT_BUTTON_PIN, INPUT);
        pinMode(DOWN_BUTTON_PIN, INPUT);
        pinMode(UP_BUTTON_PIN, INPUT);
        pinMode(RIGHT_BUTTON_PIN, INPUT);

        state = initializeGameState(4);

        Paint_Clear(BLACK);
}

void loop(void)
{
        // picture loop
        while (1) {
                spawnTile(state);

                draw(state);
                Paint_Clear(BLACK);
                int yOffset = 32;
                int fontSize = 16;
                int fontWidth = 11;
                int leftMargin = 23;
                Paint_ClearWindows(leftMargin, fontSize + 30,
                                   leftMargin + 21 * fontWidth,
                                   yOffset + fontSize * (10), WHITE);
                Paint_DrawCircle(leftMargin, fontSize + 30, 6, RED,
                                 DOT_PIXEL_1X1, DRAW_FILL_FULL);
                Paint_DrawCircle(leftMargin + 21 * fontWidth, fontSize + 30, 6,
                                 RED, DOT_PIXEL_1X1, DRAW_FILL_FULL);
                Paint_DrawCircle(leftMargin, yOffset + fontSize * (10), 6, RED,
                                 DOT_PIXEL_1X1, DRAW_FILL_FULL);
                Paint_DrawCircle(leftMargin + 21 * fontWidth,
                                 yOffset + fontSize * (10), 6, RED,
                                 DOT_PIXEL_1X1, DRAW_FILL_FULL);

                draw(state);
                while (!isGameOver(state)) {
                        int turn;
                        bool inputRegistered = false;

                        checkJoystick(&turn, &inputRegistered);
                        checkButtons(&turn, &inputRegistered);

                        if (inputRegistered) {
                                takeTurn(state, turn);
                                draw(state);
                                delay(MOVE_REGISTERED_DELAY);
                        }
                        delay(INPUT_POLLING_DELAY);
                }
                drawGameOver(state);
        }
}

/*******************************************************************************
  User Interface
*******************************************************************************/

void strReplace(char *str, char *oldWord, char *newWord)
{
        char *pos, temp[1000];
        int index = 0;
        int owlen;

        owlen = strlen(oldWord);

        while ((pos = strstr(str, oldWord)) != NULL) {
                strcpy(temp, str);
                index = pos - str;
                str[index] = '\0';
                strcat(str, newWord);
                strcat(str, temp + index + owlen);
        }
}

void draw(GameState *state)
{
        Paint_NewImage(LCD_WIDTH, LCD_HEIGHT, 270, WHITE);
        // graphic commands to redraw the complete screen should be placed here
        int yOffset = 40;
        int score_y_offset = 30;
        int score_x_offset = 40;
        int fontSize = 16;
        int fontWidth = 11;
        int leftMargin = 23;
        char buffer[21];
        sprintf(buffer, "  Score: %d", state->score);
        // This clears the screen behind the score
        Paint_ClearWindows(leftMargin + 100 + score_x_offset,
                           score_y_offset + fontSize, 200 + score_x_offset,
                           score_y_offset + 2 * fontSize, WHITE);
        // This prints the score
        Paint_DrawString_EN(leftMargin + score_x_offset,
                            score_y_offset + fontSize, buffer, &Font16, WHITE,
                            BLACK);

        Paint_DrawString_EN(leftMargin, yOffset + fontSize,
                            " -------------------", &Font16, WHITE, BLACK);
        for (int i = 0; i < 4; i++) {
                // This buffer is used for all game rows
                char *buffer = (char *)malloc(22 * sizeof(char));
                sprintf(buffer, "|%4d|%4d|%4d|%4d|", state->grid[i][0], state->grid[i][1],
                        state->grid[i][2], state->grid[i][3]);
                strReplace(buffer, "   0", "    ");
                for (int j = 0; j < 4; j++) {
                        if (state->grid[i][j] != old_grid[i][j]) {
                                int borders = j + 1;
                                int gaps = j;
                                int gap_width = 4;
                                int old_value = old_grid[i][j];
                                int digit_len = number_string_length(old_value);
                                int clear_start =
                                    fontWidth * (borders + gap_width * gaps +
                                                 gap_width - digit_len);
                                int clear_end =
                                    clear_start -
                                    (gap_width - digit_len) * fontWidth +
                                    fontWidth * gap_width;
                                Paint_ClearWindows(
                                    leftMargin + clear_start,
                                    yOffset + fontSize * (2 * (i + 1)),
                                    leftMargin + clear_end,
                                    yOffset + fontSize * (2 * (i + 1)) +
                                        fontSize,
                                    WHITE);
                        }
                }

                for (int j = 0; j < 4; j++) {
                        old_grid[i][j] = state->grid[i][j];
                }

                Paint_DrawString_EN(leftMargin,
                                    yOffset + fontSize * (2 * (i + 1)), buffer,
                                    &Font16, WHITE, BLACK);
                free(buffer);
                Paint_DrawString_EN(
                    leftMargin, yOffset + fontSize * (2 * (i + 1) + 1),
                    " -------------------", &Font16, WHITE, BLACK);
        }
}

int number_string_length(int number)
{
        if (number >= 1000) {
                return 4;
        } else if (number >= 100) {
                return 3;
        } else if (number >= 10) {
                return 2;
        }
}

void drawGameOver(GameState *state)
{

        int yOffset = 40;
        int score_y_offset = 30;
        int score_x_offset = 40;
        int fontSize = 16;
        int fontWidth = 11;
        int leftMargin = 23;
        char buffer[21];
        sprintf(buffer, "  Score: %d", state->score);
        Paint_DrawString_EN(leftMargin + score_x_offset,
                            score_y_offset + fontSize, buffer, &Font16, WHITE,
                            BLACK);
        Paint_DrawString_EN(leftMargin, yOffset + fontSize,
                            " -----Game-Over-----", &Font16, WHITE, BLACK);
}

int checkJoystick(int *turn, bool *input_registered)
{
        int x_val = analogRead(STICK_X_PIN);
        int y_val = analogRead(STICK_Y_PIN);

        Serial.println("X stick value");
        Serial.println(x_val);
        Serial.println("Y stick value");
        Serial.println(y_val);

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

/*******************************************************************************
  END FILE
*******************************************************************************/
