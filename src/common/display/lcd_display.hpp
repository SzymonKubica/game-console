#pragma once
#include "display.hpp"
#include "../user_interface.h"

class LcdDisplay : public Display {
  public:
        /**
         * Initializes the display, this is for actions such as erasing the
         * previously rendered shapes in a physical Arduino display.
         */
        virtual void initialize() override;
        /**
         * Clears the display. This is done by redrawing the entire screen with
         * the specified color.
         */
        virtual void clear(int color) override;
        /**
         * Draws a rounded border around the screen. This is needed due to the
         * specifics of the physical display used by the game console: the LCD
         * screen has rounded corners and we need a utility function that draws
         * a border perfectly encircling the screen. The emulated
         * implementations of the display do not necessarily need to provide
         * this functionality.
         */
        virtual void draw_rounded_border(int color) override;
        /**
         * Draws a circle with specified color, border width and fill.
         */
        virtual void draw_circle(Point center, int radius, int color,
                                 int border_width, bool filled) override;
        /**
         * Draws a rectangle with specified color, border width and fill.
         */
        virtual void draw_rectangle(Point start, int width, int height,
                                    int color, int border_width,
                                    bool filled) override;
        /**
         * Draws a rounded rectangle with specified color. This is useful for
         * drawing nicely-looking game menu items.
         */
        virtual void draw_rounded_rectangle(Point start, int width, int height,
                                            int radius, int color) override;
        /**
         * Prints a string on the display, allows for specifying the font size,
         * color and background color.
         */
        virtual void draw_string(Point start, char *string_buffer,
                                 FontSize font_size, int bg_color,
                                 int fg_color) override;
        /**
         * Clears a rectangular region of the display. This is done by redrawing
         * the rectangle using the specified color. Note that on the physical
         * display this operation is potentially slow, hence we need to redraw
         * small regions at a time if we want the game to remain usable.
         */
        virtual void clear_region(Point top_left, Point bottom_right,
                                  int clear_color) override;
};
