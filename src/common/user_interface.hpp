#pragma once

#include <stdlib.h>
#include "user_interface_customization.hpp"
#include "configuration.hpp"
#include "platform/interface/display.hpp"

void setup_display();

void render_config_menu(Display *display, Configuration *config,
                        ConfigurationDiff *diff, bool text_update_only,
                        UserInterfaceCustomization *customization);

ConfigurationDiff *empty_diff();
