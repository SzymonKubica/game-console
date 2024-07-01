#include "GUI_Paint.h"
#include "LCD_Driver.h"
#include <SPI.h>

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#define STICK_Y_PIN 16
#define STICK_X_PIN 17

// Initialise the grid and counter variables.
int **grid;
int score = 0;
int occupiedTiles = 0;

int leftButtonPin = 9;
int downButtonPin = 15;
int upButtonPin = 8;
int rightButtonPin = 12;

int old_grid[4][4];

void str_replace(char *str, char *oldWord, char *newWord)
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

void draw()
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
        sprintf(buffer, "  Score: %d", score);
        Paint_ClearWindows(leftMargin + 100 + score_x_offset,
                           score_y_offset + fontSize, 200 + score_x_offset,
                           score_y_offset + 2 * fontSize, WHITE);
        Paint_DrawString_EN(leftMargin + score_x_offset,
                            score_y_offset + fontSize, buffer, &Font16, WHITE,
                            BLACK);
        Paint_DrawString_EN(leftMargin, yOffset + fontSize,
                            " -------------------", &Font16, WHITE, BLACK);
        for (int i = 0; i < 4; i++) {
                char *buffer = (char *)malloc(22 * sizeof(char));
                sprintf(buffer, "|%4d|%4d|%4d|%4d|", grid[i][0], grid[i][1],
                        grid[i][2], grid[i][3]);
                str_replace(buffer, "   0", "    ");
                for (int j = 0; j < 4; j++) {
                        if (grid[i][j] != old_grid[i][j]) {
                                int borders = j + 1;
                                int gaps = j;
                                int gap_width = 4;
                                int digit_len = 1;
                                int old_value = old_grid[i][j];
                                if (old_value >= 1000) {
                                        digit_len = 4;
                                } else if (old_value >= 100) {
                                        digit_len = 3;
                                } else if (old_value >= 10) {
                                        digit_len = 2;
                                }
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
                        old_grid[i][j] = grid[i][j];
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
void drawGameOver()
{

        int yOffset = 40;
        int score_y_offset = 30;
        int score_x_offset = 40;
        int fontSize = 16;
        int fontWidth = 11;
        int leftMargin = 23;
        char buffer[21];
        sprintf(buffer, "  Score: %d", score);
        Paint_DrawString_EN(leftMargin + score_x_offset,
                            score_y_offset + fontSize, buffer, &Font16, WHITE,
                            BLACK);
        Paint_DrawString_EN(leftMargin, yOffset + fontSize,
                            " -----Game-Over-----", &Font16, WHITE, BLACK);
}

int generateNewTileValue() { return 2 + 2 * (int)random(2); }

int getRandomCoordinate() { return random(100) % 4; }

void spawnTile()
{
        bool success = false;
        while (!success) {
                int x = getRandomCoordinate();
                int y = getRandomCoordinate();

                if (grid[x][y] == 0) {
                        grid[x][y] = generateNewTileValue();
                        success = true;
                }
        }
        occupiedTiles++;
}

bool isEmptyRow(int *row)
{
        boolean isEmpty = true;
        for (int i = 0; i < 4; i++) {
                isEmpty &= (row[i] == 0);
        }
        return isEmpty;
}

void transpose()
{
        int **transposed = allocateGrid();
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        transposed[j][i] = grid[i][j];
                }
        }
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        grid[i][j] = transposed[i][j];
                }
        }
        freeGrid(transposed);
}

void merge(int direction)
{
        if (direction == UP || direction == DOWN) {
                transpose();
        }

        for (int i = 0; i < 4; i++) {
                mergeRow(i, direction);
        }

        if (direction == UP || direction == DOWN) {
                transpose();
        }
}

int getSuccessorIndex(int i, int currentIndex)
{
        int succ = currentIndex + 1;
        while (succ < 4 && grid[i][succ] == 0) {
                succ++;
        }
        return succ;
}

void reverse(int *row)
{
        int clone[4];
        for (int i = 0; i < 4; i++) {
                clone[i] = row[i];
        }
        for (int i = 0; i < 4; i++) {
                row[3 - i] = clone[i];
        }
}

void mergeRow(int i, int direction)
{
        int currentIndex = 0;
        int mergedRow[4] = {0, 0, 0, 0};
        int mergedNum = 0;

        if (direction == DOWN || direction == RIGHT) {
                reverse(grid[i]);
        }

        currentIndex = getSuccessorIndex(i, -1);

        if (currentIndex == 4) {
                // All tiles are empty.
                return;
        }

        // Now the current tile must be non-empty.
        while (currentIndex < 4) {
                int successorIndex = getSuccessorIndex(i, currentIndex);
                if (successorIndex < 4 &&
                    grid[i][currentIndex] == grid[i][successorIndex]) {
                        // Two matching tiles found, we perform a merge.
                        int sum =
                            grid[i][currentIndex] + grid[i][successorIndex];
                        score += sum;
                        occupiedTiles--;
                        mergedRow[mergedNum] = sum;
                        mergedNum++;
                        currentIndex = getSuccessorIndex(i, successorIndex);
                } else {
                        mergedRow[mergedNum] = grid[i][currentIndex];
                        mergedNum++;
                        currentIndex = successorIndex;
                }
        }

        for (int j = 0; j < 4; j++) {
                if (direction == DOWN || direction == RIGHT) {
                        grid[i][3 - j] = mergedRow[j];
                } else {
                        grid[i][j] = mergedRow[j];
                }
        }
}

void freeGrid(int **g)
{
        for (int i = 0; i < 4; i++) {
                free(g[i]);
        }
        free(g);
}
void takeTurn(int direction)
{
        int **oldGrid = allocateGrid();
        copy(grid, oldGrid);
        merge(direction);

        if (theGridChangedFrom(oldGrid)) {
                spawnTile();
        }
        freeGrid(oldGrid);
}

bool isBoardFull() { return occupiedTiles >= 16; }

bool isGameOver() { return isBoardFull() && noMovePossible(); }

bool theGridChangedFrom(int **oldGrid)
{
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        if (grid[i][j] != oldGrid[i][j]) {
                                return true;
                        }
                }
        }
        return false;
}

bool noMovePossible()
{
        // Preserve the state
        int **currentState = allocateGrid();
        copy(grid, currentState);
        int currentScore = score;
        int currentOccupied = occupiedTiles;

        boolean noMoves = true;
        for (int direction = 0; direction < 4; direction++) {
                merge(direction);
                noMoves &= !theGridChangedFrom(currentState);
                copy(currentState, grid);
        }
        freeGrid(currentState);

        // Restore the state
        score = currentScore;
        occupiedTiles = currentOccupied;
        return noMoves;
}

void copy(int **source, int **destination)
{
        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        destination[i][j] = source[i][j];
                }
        }
}

int **allocateGrid()
{
        int **g = (int **)malloc(4 * sizeof(int));
        for (int i = 0; i < 4; i++) {
                g[i] = (int *)malloc(4 * sizeof(int));
        }
        return g;
}

void setup(void)
{
        Config_Init();
        LCD_Init();

        LCD_SetBacklight(100);
        Serial.begin(115200);
        randomSeed(analogRead(0));

        pinMode(leftButtonPin, INPUT);
        pinMode(downButtonPin, INPUT);
        pinMode(upButtonPin, INPUT);
        pinMode(rightButtonPin, INPUT);

        grid = allocateGrid();
        Paint_Clear(BLACK);

        for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                        grid[i][j] = 0;
                }
        }
}

int leftButton;
int downButton;
int upButton;
int rightButton;

int check_joystick(bool *input_registered)
{
        int x_val = analogRead(STICK_X_PIN);
        int y_val = analogRead(STICK_Y_PIN);

        Serial.println("X stick value");
        Serial.println(x_val);
        Serial.println("Y stick value");
        Serial.println(y_val);

        if (x_val < 100) {
                *input_registered = true;
                return RIGHT;
        }

        if (x_val > 900) {
                *input_registered = true;
                return LEFT;
        }

        if (y_val < 100) {
                *input_registered = true;
                return UP;
        }

        if (y_val > 900) {
                *input_registered = true;
                return DOWN;
        }
}

void loop(void)
{

        // picture loop
        while (1) {
                spawnTile();

                draw();
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


                draw();

                while (!isGameOver()) {

                        leftButton = digitalRead(leftButtonPin);
                        downButton = digitalRead(downButtonPin);
                        upButton = digitalRead(upButtonPin);
                        rightButton = digitalRead(rightButtonPin);
                        int turn;
                        bool inputRegistered = false;
                        turn = check_joystick(&inputRegistered);
                        Serial.println(inputRegistered);

                        if (!leftButton) {
                                turn = LEFT;
                                inputRegistered = true;
                        }
                        if (!downButton) {
                                turn = DOWN;
                                inputRegistered = true;
                        }
                        if (!upButton) {
                                turn = UP;
                                inputRegistered = true;
                        }
                        if (!rightButton) {
                                turn = RIGHT;
                                inputRegistered = true;
                        }

                        if (inputRegistered) {
                                takeTurn(turn);
                                draw();
                                delay(150);
                        }
                        delay(50);
                }

                drawGameOver();
        }
}

// #if 1
//     Paint_NewImage(LCD_WIDTH, LCD_HEIGHT, 0, WHITE);
//     Paint_Clear(WHITE);
//
//     Paint_DrawString_EN(30, 10, "123", &Font24, YELLOW, RED);
//     Paint_DrawString_EN(30, 34, "ABC", &Font24, BLUE, CYAN);
//
//     Paint_DrawString_CN(10, 150, "微", &Font24CN,WHITE, RED);
//     Paint_DrawString_CN(45, 150, "雪", &Font24CN,WHITE, RED);
//     Paint_DrawString_CN(80, 150, "电", &Font24CN,WHITE, RED);
//     Paint_DrawString_CN(115, 150, "子", &Font24CN,WHITE, RED);
//
//     Paint_DrawImage(gImage_70X70, 10, 70, 70, 70);
//
//     Paint_DrawRectangle(100, 20, 160, 120, RED, DOT_PIXEL_2X2,
//     DRAW_FILL_EMPTY); Paint_DrawLine(100, 20, 160, 120, MAGENTA,
//     DOT_PIXEL_2X2, LINE_STYLE_SOLID); Paint_DrawLine(100, 120, 160, 20,
//     MAGENTA, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
//
//     Paint_DrawCircle(50, 220, 25, BLUE, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
//     Paint_DrawCircle(80, 220, 25, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
//     Paint_DrawCircle(110, 220, 25, RED, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
//     Paint_DrawCircle(65, 245, 25, YELLOW, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
//     Paint_DrawCircle(95, 245, 25, GREEN, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
//
//     delay(3000);

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
