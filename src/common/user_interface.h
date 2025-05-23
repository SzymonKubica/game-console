#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <stdlib.h>
#include "../games/2048.h"
#include "configuration.hpp"
#include "display/display.hpp"

void setup_display();
void draw_game_canvas(Display *display, GameState *state);
void draw(Display *display, GameState *state);
void draw_game_over(Display *display, GameState *state);
void draw_game_won(Display *display, GameState *state);
void update_game_grid(Display *display, GameState *gs);

void render_config_menu(Display *display, GameConfiguration *config,
                             GameConfiguration *previous_config, bool update);
void render_generic_config_menu(Display *display, Configuration *config,
                                ConfigurationDiff *diff, bool updateDetected);

#endif
