#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <stdlib.h>
#include "../games/2048.h"
#include "configuration.h"

void initializeDisplay();
void drawGameCanvas(GameState *state);
void draw(GameState *state);
void drawGameOver(GameState *state);
void drawGameWon(GameState *state);
void updateGameGrid(GameState *gs);

void drawConfigurationMenu(GameConfiguration *config, GameConfiguration *previous_config, bool update);
void renderGenericConfigMenu(Configuration *config, ConfigurationDiff *diff, bool updateDetected);

#endif
