#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <stdlib.h>
#include "configuration.hpp"
#include "platform/interface/display.hpp"

enum UserInterfaceRenderingMode {
        /**
         * Minimalistic UI rendering mode. It leverages normal rectangles with
         * no fill instead of filled rounded ones. It also skips rendering of the
         * rounded screen borders.
         *
         * This is useful to make the UX more responsive on the actual LCD
         * display on the target device. The display is quite slow (it is limited
         * by the clock rate of the Arduino SPI interface), and redrawing large
         * blobs of UI elements is slow and can get annoying if users decide
         * to traverse many UI screens. If this rendering mode is enabled, the
         * experience will be much more snappy.
         */
        Minimalistic = 0,
        /**
         * Original UI look and feel, most UI elements are rendered as filled
         * rounded rectangles.
         */
        Detailed = 1,
};

typedef struct UserInterfaceCustomization {
        /**
         * Accent color of the UI elements. This applies to the menu selectors,
         * rounded borders and user input carets.
         */
        Color accent_color;
        UserInterfaceRenderingMode rendering_mode;
} UserInterfaceCustomization;

void setup_display();

void render_config_menu(Display *display, Configuration *config,
                                ConfigurationDiff *diff, bool updateDetected, Color accent_color = DarkBlue);

ConfigurationDiff *empty_diff();
#endif
