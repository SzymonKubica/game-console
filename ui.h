#include "GUI_Paint.h"
#include "LCD_Driver.h"
#include <string.h>
#include <stdlib.h>
#include "game2048.h"

// Paints the white canvas for the game grid and the four red
// dots in the corners
void drawGameCanvas();

void strReplace(char *str, char *oldWord, char *newWord);

void draw(GameState *state);

void drawGameOver(GameState *state);

