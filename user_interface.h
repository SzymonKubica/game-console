#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <string.h>
#include <stdlib.h>
#include "GUI_Paint.h"
#include "LCD_Driver.h"
#include "game2048.h"

void initializeDisplay();
void drawGameCanvas(GameState *state);
void draw(GameState *state);
void drawGameOver(GameState *state);
void drawGameWon(GameState *state);
void updateGameGrid(GameState *gs);

void drawConfigurationMenu(GameConfiguration *config, GameConfiguration *previous_config, bool update);

#endif
