#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <string.h>
#include <stdlib.h>
#include "GUI_Paint.h"
#include "LCD_Driver.h"
#include "game2048.h"

// Paints the white canvas for the game grid and the four red
// dots in the corners
void drawGameCanvas();

void draw(GameState *state);

void drawGameOver(GameState *state);
void initializeDisplay();

#endif
