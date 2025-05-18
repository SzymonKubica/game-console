#pragma once
#include "display.hpp"
#include "../user_interface.h"

/**
 * @brief LcdDisplay class that implements the Display interface for the
 * physical LCD display used in the game console.
 *
 * This class provides methods for drawing shapes, strings, and clearing the
 * display. All calls are forwarded to the library responsible for driving the
 * display. Because of this, this module depends on the `src/lib/` modules.
 */
class LcdDisplay : public Display
{
      public:
        /**
         * Performs the setup of the display. This is intended for performing
         * initialization of the modules that are responsible for driving the
         * particular implementation of the display. In case of the hardware
         * display, this is supposed to initialize the display driver and erease
         * its previous contents. Note that this should be called inside of
         * the `setup` Arduino function and is intended to be executed only once.
         */
        virtual void setup() override;
        /**
         * Initializes the display, this is for actions such as erasing the
         * previously rendered shapes in a physical Arduino display. Intended
         * for use in the body of the `loop` Aruino function.
         */
        virtual void initialize() override;
        /**
         * Clears the display. This is done by redrawing the entire screen with
         * the specified color.
         */
        virtual void clear(Color color) override;
        /**
         * Draws a rounded border around the screen. This is needed due to the
         * specifics of the physical display used by the game console: the LCD
         * screen has rounded corners and we need a utility function that draws
         * a border perfectly encircling the screen. The emulated
         * implementations of the display do not necessarily need to provide
         * this functionality.
         */
        virtual void draw_rounded_border(Color color) override;
        /**
         * Draws a circle with specified color, border width and fill.
         */
        virtual void draw_circle(Point center, int radius, Color color,
                                 int border_width, bool filled) override;
        /**
         * Draws a rectangle with specified color, border width and fill.
         */
        virtual void draw_rectangle(Point start, int width, int height,
                                    Color color, int border_width,
                                    bool filled) override;
        /**
         * Draws a rounded rectangle with specified color. This is useful for
         * drawing nicely-looking game menu items.
         */
        virtual void draw_rounded_rectangle(Point start, int width, int height,
                                            int radius, Color color) override;
        /**
         * Prints a string on the display, allows for specifying the font size,
         * color and background color.
         */
        virtual void draw_string(Point start, char *string_buffer,
                                 FontSize font_size, Color bg_color,
                                 Color fg_color) override;
        /**
         * Clears a rectangular region of the display. This is done by redrawing
         * the rectangle using the specified color. Note that on the physical
         * display this operation is potentially slow, hence we need to redraw
         * small regions at a time if we want the game to remain usable.
         */
        virtual void clear_region(Point top_left, Point bottom_right,
                                  Color clear_color) override;

        /**
         * Returns the height of the display.
         */
        virtual int get_height() override;

        /**
         * Returns the width of the display.
         */
        virtual int get_width() override;
};
