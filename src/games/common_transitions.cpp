#include "common_transitions.hpp"
#include "../common/constants.hpp"

void display_input_clafification(Display *display)
{

        {
                const char *msg = "Press blue to exit.";

                int height = display->get_height();
                int width = display->get_width();
                int x_pos = (width - strlen(msg) * FONT_WIDTH) / 2;
                int y_pos = (height - FONT_SIZE) / 2 + 2 * FONT_SIZE;
                Point text_position = {.x = x_pos, .y = y_pos};

                display->draw_string(text_position, (char *)msg, Size16, Black,
                                     White);
        }

        {
                const char *msg = "Tilt stick to try again.";

                int height = display->get_height();
                int width = display->get_width();
                int x_pos = (width - strlen(msg) * FONT_WIDTH) / 2;
                int y_pos = (height - FONT_SIZE) / 2 + 3 * FONT_SIZE;
                Point text_position = {.x = x_pos, .y = y_pos};

                display->draw_string(text_position, (char *)msg, Size16, Black,
                                     White);
        }
}

void draw_game_over(Display *display, UserInterfaceCustomization *customization)
{
        if (customization->rendering_mode == Detailed) {
                display->draw_rounded_border(Red);
        } else {
                // In the minimialistic UI mode we only clear the screen.
                display->clear(Black);
        }

        const char *msg = "Game Over";

        int height = display->get_height();
        int width = display->get_width();
        int x_pos = (width - strlen(msg) * FONT_WIDTH) / 2;
        int y_pos = (height - FONT_SIZE) / 2;

        Point text_position = {.x = x_pos, .y = y_pos};

        display->draw_string(text_position, (char *)msg, Size16, Black, Red);
        display_input_clafification(display);
}

void draw_game_won(Display *display, UserInterfaceCustomization *customization)
{
        if (customization->rendering_mode == Detailed) {
                display->draw_rounded_border(Green);
        } else {
                // In the minimialistic UI mode we only clear the screen.
                display->clear(Black);
        }

        const char *msg = "You Won!";

        int height = display->get_height();
        int width = display->get_width();
        int x_pos = (width - strlen(msg) * FONT_WIDTH) / 2;
        int y_pos = (height - FONT_SIZE) / 2;
        Point text_position = {.x = x_pos, .y = y_pos};

        display->draw_string(text_position, (char *)msg, Size16, Black, Green);

        display_input_clafification(display);
}

void pause_until_any_directional_input(
    std::vector<DirectionalController *> *controllers,
    DelayProvider *delay_provider)
{
        Direction dir;
        while (!directional_input_registered(controllers, &dir)) {
                delay_provider->delay_ms(INPUT_POLLING_DELAY);
        };
}

void pause_until_input(std::vector<DirectionalController *> *controllers,
                       std::vector<ActionController *> *action_controllers,
                       Direction *direction, Action *action,
                       DelayProvider *delay_provider)
{
        while (!directional_input_registered(controllers, direction) &&
               !action_input_registered(action_controllers, action)) {
                delay_provider->delay_ms(INPUT_POLLING_DELAY);
        };
}
