#ifdef EMULATOR
#include "sfml_display.hpp"
#include <SFML/Graphics.hpp>
#include "../../constants.hpp"

// TODO: move those defines to common constants so that changes to them affect
// both the emulator and the production build.
#define DISPLAY_HEIGHT 240
#define DISPLAY_WIDTH 280

#define SCREEN_BORDER_WIDTH 3

void SfmlDisplay::setup() {};

void SfmlDisplay::initialize() {}

sf::Color map_to_sf_color(Color color);
void SfmlDisplay::clear(Color color)
{
        texture->clear(map_to_sf_color(color));
        texture->display();
        refresh();
};

/**
 * Draws a rounded border around the game display, this might not be the most
 * efficient solution using the SFML API, but it uses the same logic as the
 * LCD display implementation for consistency.
 */
void SfmlDisplay::draw_rounded_border(Color color)
{
        int rounding_radius = DISPLAY_CORNER_RADIUS;
        int margin = SCREEN_BORDER_WIDTH;
        int line_width = 3;
        int width = get_width();
        int height = get_height();
        Point top_left_corner = {.x = rounding_radius + margin,
                                 .y = rounding_radius + margin};
        Point bottom_right_corner = {.x = width - rounding_radius - margin,
                                     .y = height - rounding_radius - margin};

        int x_positions[2] = {top_left_corner.x, bottom_right_corner.x};
        int y_positions[2] = {top_left_corner.y, bottom_right_corner.y};

        clear(Black);

        // Draw the four rounded corners.
        for (int x : x_positions) {
                for (int y : y_positions) {
                        draw_circle({.x = x, .y = y}, rounding_radius, color, 3,
                                    false);
                }
        }

        // Draw the four lines connecting the circles.
        // This is the first vertical line on the left
        draw_rectangle({.x = 0, .y = top_left_corner.y}, 0,
                       bottom_right_corner.y - top_left_corner.y, color, 3,
                       true);

        // This is the second vertical line on the left
        draw_rectangle({.x = width, .y = top_left_corner.y}, 0,
                       bottom_right_corner.y - top_left_corner.y, color, 3,
                       true);

        // This is the first horizontal line at the top
        draw_rectangle({.x = top_left_corner.x, .y = 0},
                       bottom_right_corner.x - top_left_corner.x, 0, color, 3,
                       true);

        // This is the second horizontal line at the bottom
        draw_rectangle({.x = top_left_corner.x, .y = height},
                       bottom_right_corner.x - top_left_corner.x, 0, color, 3,
                       true);

        int circle_diameter = 2 * rounding_radius;
        // Erase the middle bits of the four circles
        // This clears bottom half of the top left circle
        clear_region({.x = margin, .y = top_left_corner.y - margin},
                     {.x = margin + line_width + circle_diameter,
                      .y = top_left_corner.y + rounding_radius + line_width},
                     Black);

        // This clears bottom half of the top right circle
        clear_region({.x = width - margin - line_width - circle_diameter - 1,
                      .y = top_left_corner.y - margin},
                     {.x = width - margin,
                      .y = top_left_corner.y + rounding_radius + line_width},
                     Black);

        // This clears top half of the bottom left circle
        clear_region({.x = margin,
                      .y = bottom_right_corner.y - rounding_radius -
                           line_width - margin},
                     {.x = margin + line_width + circle_diameter,
                      .y = bottom_right_corner.y + margin},
                     Black);

        // This clears top half of the bottom right circle
        clear_region({.x = width - margin - line_width - circle_diameter - 1,
                      .y = bottom_right_corner.y - rounding_radius - margin},
                     {.x = width - margin, .y = bottom_right_corner.y + margin},
                     Black);

        // The four remaining lines
        clear_region({.x = top_left_corner.x - margin, .y = margin},
                     {.x = top_left_corner.x + rounding_radius + line_width,
                      .y = margin + line_width + rounding_radius},
                     Black);

        clear_region(
            {.x = top_left_corner.x - margin,
             .y = height - margin - line_width - 1 - rounding_radius},
            {.x = top_left_corner.x + rounding_radius + line_width + margin,
             .y = height - margin},
            Black);

        clear_region(
            {.x = bottom_right_corner.x - rounding_radius - line_width - 1,
             .y = margin},
            {.x = bottom_right_corner.x + margin,
             .y = margin + line_width + rounding_radius},
            Black);

        clear_region(
            {.x = bottom_right_corner.x - rounding_radius - line_width - 1,
             .y = height - margin - line_width - 1 - rounding_radius},
            {.x = bottom_right_corner.x - line_width - 1, .y = height - margin},
            Black);
}

void SfmlDisplay::draw_circle(Point center, int radius, Color color,
                              int border_width, bool filled)
{
        // Note: the circle is always filled, given the current use cases this
        // is fine, but we need to tighten up the API in the future as we
        // start onboarding more complex game rendering.
        sf::CircleShape circle(radius);
        circle.setPosition(
            {(float)(center.x - radius), (float)(center.y - radius)});

        if (filled) {
                circle.setFillColor(map_to_sf_color(color));
        } else {
                circle.setFillColor(map_to_sf_color(Black));
        }

        circle.setOutlineColor(map_to_sf_color(color));
        circle.setOutlineThickness(border_width);
        texture->draw(circle);
        texture->display();
        refresh();
};

void SfmlDisplay::draw_rectangle(Point start, int width, int height,
                                 Color color, int border_width, bool filled)
{

        sf::RectangleShape rectangle({(float)width, (float)height});
        rectangle.setPosition({(float)start.x, (float)start.y});
        if (filled) {
                rectangle.setFillColor(map_to_sf_color(color));
        } else {
                rectangle.setFillColor(sf::Color::Transparent);
        }
        rectangle.setOutlineColor(map_to_sf_color(color));
        rectangle.setOutlineThickness(border_width);
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
                        draw_circle({.x = x, .y = y}, radius, color, 0, true);
                }
        }

        // This needs to be cleaned up
        draw_rectangle({.x = top_left_corner.x, .y = start.y},
                       width - 2 * radius, height, color, 0, true);

        // small rectangle on the left
        draw_rectangle({.x = start.x, .y = top_left_corner.y}, width + 1,
                       height - 2 * radius, color, 0, true);

        // +1 is because the endY bound is not included
        // The big rectangle in the middle
        draw_rectangle({.x = top_left_corner.x, .y = start.y + height - radius},
                       width - 2 * radius, radius + 1, color, 0, true);
};
void SfmlDisplay::draw_string(Point start, char *string_buffer,
                              FontSize font_size, Color bg_color,
                              Color fg_color)
{
        // TODO
        // The font probably needs to live in the project resources for
        // portability
        const sf::Font font(
            "/usr/share/fonts/TTF/JetBrainsMonoNerdFont-Regular.ttf");
        sf::Text text(font, string_buffer, font_size);
        text.setFillColor(map_to_sf_color(fg_color));
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
        /* We need this polling when refreshign the display. Without it, linux
        desktop environments (e.g. gnome) think that the game window is not
        responsive and try to get us to force-close it. */
        while (const std::optional event = window->pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                        window->close();
                        // This is the only way we can terminate the game loop
                        // in the current setup, we need to send an exception
                        // and catch it in the emulator entrypoint.
                        throw std::runtime_error("Window closed");
                }
        }

        // Now we start rendering to the window, clear it first
        // texture->display();
        window->clear();
        // Draw the texture
        sf::Sprite sprite(texture->getTexture());
        window->draw(sprite);

        // End the current frame and display its contents on screen
        window->display();
};

sf::Color map_to_sf_color(Color color)
{
        switch (color) {
        case White:
                return sf::Color::White;
        case Black:
                return sf::Color::Black;
        case Blue:
                return sf::Color::Blue;
        case BRed:
                return sf::Color::Magenta;
        case Yellow:
                return sf::Color::Yellow;
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
