#include "lcd_display.hpp"

void LcdDisplay::initialize() {}
void LcdDisplay::clear(int color) {}
void LcdDisplay::draw_rounded_border(int color) {}
void LcdDisplay::draw_circle(Point center, int radius, int color,
                             int border_width, bool filled)
{
}
void LcdDisplay::draw_rectangle(Point start, int width, int height, int color,
                                int border_width, bool filled)
{
}
void LcdDisplay::draw_rounded_rectangle(Point start, int width, int height,
                                        int radius, int color)
{
}
void LcdDisplay::draw_string(Point start, char *string_buffer,
                             FontSize font_size, int bg_color, int fg_color)
{
}
void LcdDisplay::clear_region(Point top_left, Point bottom_right,
                              int clear_color)
{
}
