#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <stdlib.h>
#include "../games/2048.hpp"
#include "configuration.hpp"
#include "platform/interface/display.hpp"

void setup_display();

void render_config_menu_legacy(Display *display, GameConfiguration *config,
                             GameConfiguration *previous_config, bool update);
void render_config_menu(Display *display, Configuration *config,
                                ConfigurationDiff *diff, bool updateDetected);

ConfigurationDiff *empty_diff();
#endif
