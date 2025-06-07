#include "sfml_display.hpp"
#include <SFML/Graphics.hpp>

void SfmlDisplay::setup()
{
        sf::RenderWindow window(sf::VideoMode({200, 200}), "SFML works!");
        this->window = &window;
};

void SfmlDisplay::initialize() {}

void SfmlDisplay::clear(Color color)
{
        this->window->clear();
        this->window->display();
};

void SfmlDisplay::draw_rounded_border(Color color) {}

void SfmlDisplay::draw_circle(Point center, int radius, Color color,
                              int border_width, bool filled)
{
        sf::CircleShape circle(radius);
        circle.setPosition(
            {(float)(center.x - radius), (float)(center.y - radius)});
        // TODO implement color mapping
        // circle.setFillColor(NULL);
        this->window->draw(circle);
        this->window->display();
};

void SfmlDisplay::draw_rectangle(Point start, int width, int height,
                                 Color color, int border_width, bool filled) {};
void SfmlDisplay::draw_rounded_rectangle(Point start, int width, int height,
                                         int radius, Color color) {};
void SfmlDisplay::draw_string(Point start, char *string_buffer,
                              FontSize font_size, Color bg_color,
                              Color fg_color) {};
void SfmlDisplay::clear_region(Point top_left, Point bottom_right,
                               Color clear_color) {};

int SfmlDisplay::get_height() {}

int SfmlDisplay::get_width() {}

int SfmlDisplay::get_display_corner_radius() {};
