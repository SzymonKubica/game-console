#ifdef EMULATOR
#include "sfml_display.hpp"
#include <SFML/Graphics.hpp>

#define DISPLAY_HEIGHT 240
#define DISPLAY_WIDTH 280

void SfmlDisplay::setup() {};

void SfmlDisplay::initialize() {}

sf::Color map_color(Color color);
void SfmlDisplay::clear(Color color)
{
        texture->clear(map_color(color));
        texture->display();
        refresh();
};

void SfmlDisplay::draw_rounded_border(Color color) {}

void SfmlDisplay::draw_circle(Point center, int radius, Color color,
                              int border_width, bool filled)
{
        // Note: the circle is always filled, given the current use cases this
        // is fine, but we need to tighten up the API in the future as we
        // start onboarding more complex game rendering.
        sf::CircleShape circle(radius);
        circle.setPosition(
            {(float)(center.x - radius), (float)(center.y - radius)});
        circle.setFillColor(map_color(color));
        texture->draw(circle);
        texture->display();
        refresh();
};

void SfmlDisplay::draw_rectangle(Point start, int width, int height,
                                 Color color, int border_width, bool filled)
{

        sf::RectangleShape rectangle({(float)width, (float)height});
        rectangle.setPosition({(float)start.x, (float)start.y});
        rectangle.setFillColor(map_color(color));
        texture->draw(rectangle);
        texture->display();
        refresh();
};
void SfmlDisplay::draw_rounded_rectangle(Point start, int width, int height,
                                         int radius, Color color)
{
        Point top_left_corner = {.x = start.x + radius, .y = start.y + radius};

        Point bottom_right_corner = {.x = start.x + width - radius,
                                     .y = start.y + height - radius};

        int x_positions[2] = {top_left_corner.x, bottom_right_corner.x};
        int y_positions[2] = {top_left_corner.y, bottom_right_corner.y};

        // Draw the four rounded corners.
        for (int x : x_positions) {
                for (int y : y_positions) {
                        draw_circle({.x = x, .y = y}, radius, color, 1, true);
                }
        }

        // This needs to be cleaned up
        draw_rectangle({.x = top_left_corner.x, .y = start.y},
                       width - 2 * radius, height, color, 1, true);

        // small rectangle on the left
        draw_rectangle({.x = start.x, .y = top_left_corner.y}, width + 1,
                       height - 2 * radius, color, 1, true);

        // +1 is because the endY bound is not included
        // The big rectangle in the middle
        draw_rectangle({.x = top_left_corner.x, .y = start.y + height - radius},
                       width - 2 * radius, radius + 1, color, 1, true);
};
void SfmlDisplay::draw_string(Point start, char *string_buffer,
                              FontSize font_size, Color bg_color,
                              Color fg_color)
{
        // The font probably needs to live in the project resources for portability
        const sf::Font font(
            "/usr/share/fonts/TTF/JetBrainsMonoNerdFont-Regular.ttf");
        sf::Text text(font, string_buffer, font_size);
        text.setFillColor(map_color(fg_color));
        text.setPosition({(float)start.x, (float)start.y});
        texture->draw(text);
        texture->display();
        refresh();
};
void SfmlDisplay::clear_region(Point top_left, Point bottom_right,
                               Color clear_color)
{
        draw_rectangle(top_left, bottom_right.x - top_left.x,
                       bottom_right.y - top_left.y, clear_color, 0, true);
};

int SfmlDisplay::get_height() { return DISPLAY_HEIGHT; }

int SfmlDisplay::get_width() { return DISPLAY_WIDTH; }

int SfmlDisplay::get_display_corner_radius() { return 40; };

void SfmlDisplay::refresh()
{
        // Now we start rendering to the window, clear it first
        // texture->display();
        window->clear();
        // Draw the texture
        sf::Sprite sprite(texture->getTexture());
        window->draw(sprite);

        // End the current frame and display its contents on screen
        window->display();
};

sf::Color map_color(Color color)
{
        switch (color) {
        case White:
                return sf::Color::White;
        case Black:
                return sf::Color::Black;
        case Blue:
                return sf::Color::Blue;
        case BRed:
                return sf::Color::Red;
        case GRed:
                return sf::Color::Red;
        case Gblue:
                return sf::Color::Blue;
        case Red:
                return sf::Color::Red;
        case Green:
                return sf::Color::Green;
        case Cyan:
                return sf::Color::Cyan;
        case BRRed:
                return sf::Color::Red;
        case DarkBlue:
                return sf::Color::Blue;
        case LightBlue:
                return sf::Color::Blue;
        case GrayBlue:
                return sf::Color::Blue;
        case LightGreen:
                return sf::Color::Green;
        case LGrayBlue:
                return sf::Color::Blue;
        case LBBlue:
                return sf::Color::Blue;
        }
        return sf::Color::White;
}
#endif
