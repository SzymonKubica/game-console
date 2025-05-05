#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <stdlib.h>
#include "../games/2048.h"
#include "configuration.hpp"
#include "display/display.hpp"



void setup_display();
void draw_game_canvas(Display *display, GameState *state);
void draw(Display *display,GameState *state);
void drawGameOver(Display *display,GameState *state);
void drawGameWon(Display *display,GameState *state);
void update_game_grid(Display *display,GameState *gs);

void draw_configuration_menu(Display *display,GameConfiguration *config, GameConfiguration *previous_config, bool update);
void renderGenericConfigMenu(Display *display,Configuration *config, ConfigurationDiff *diff, bool updateDetected);





#endif
