#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <stdlib.h>
#include "../games/2048.h"
#include "configuration.hpp"
#include "platform/interface/display.hpp"

void setup_display();

void render_config_menu(Display *display, GameConfiguration *config,
                             GameConfiguration *previous_config, bool update);
void render_generic_config_menu(Display *display, Configuration *config,
                                ConfigurationDiff *diff, bool updateDetected);

Configuration *assemble_2048_game_menu_configuration();

ConfigurationDiff *get_initial_no_diff();
#endif
