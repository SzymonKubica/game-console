
#include "common_transitions.hpp"
#include "../common/constants.hpp"

void draw_game_over(Display *display)
{
        display->draw_rounded_border(Red);

        const char *msg = "Game Over";

        int height = display->get_height();
        int width = display->get_width();
        int x_pos = (width - strlen(msg) * FONT_WIDTH) / 2;
        int y_pos = (height - FONT_SIZE) / 2;

        Point text_position = {.x = x_pos, .y = y_pos};

        display->draw_string(text_position, (char *)msg, Size16, Black, Red);
}

void draw_game_won(Display *display)
{
        display->draw_rounded_border(Green);

        const char *msg = "You Won!";

        int height = display->get_height();
        int width = display->get_width();
        int x_pos = (width - strlen(msg) * FONT_WIDTH) / 2;
        int y_pos = (height - FONT_SIZE) / 2;
        Point text_position = {.x = x_pos, .y = y_pos};

        display->draw_string(text_position, (char *)msg, Size16, Black, Green);
}

void pause_until_input(std::vector<DirectionalController *> *controllers,
                       DelayProvider *delay_provider)
{
        Direction dir;
        while (!directional_input_registered(controllers, &dir)) {
                delay_provider->delay_ms(INPUT_POLLING_DELAY);
        };
}
