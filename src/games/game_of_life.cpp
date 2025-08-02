#include "../common/logging.hpp"
#include "../common/configuration.hpp"
#include "../common/constants.hpp"
#include "game_executor.hpp"
#include <cstring>

#define TAG "game_of_life"
#define GAME_CELL_WIDTH 3

typedef struct GameOfLifeConfiguration {
        bool prepopulate_grid;
        int simulation_speed;
} GameOfLifeConfiguration;

/**
 * Stores all information required for rendering the finite grid for game of
 * life simulation. Note that this is similar to the struct that we are using
 * for Minesweeper so we might want to abstract away this commonality in the
 * future.
 */
typedef struct GameOfLifeGridDimensions {
        int rows;
        int cols;
        int top_vertical_margin;
        int left_horizontal_margin;
        int actual_width;
        int actual_height;

        GameOfLifeGridDimensions(int r, int c, int tvm, int lhm, int aw, int ah)
            : rows(r), cols(c), top_vertical_margin(tvm),
              left_horizontal_margin(lhm), actual_width(aw), actual_height(ah)
        {
        }
} GameOfLifeGridDimensions;

static void collect_game_configuration(Platform *p,
                                       GameOfLifeConfiguration *game_config,
                                       GameCustomization *customization);
void extract_game_config(GameOfLifeConfiguration *game_config,
                         Configuration *config);
Configuration *assemble_game_of_life_configuration();

GameOfLifeGridDimensions *
calculate_grid_dimensions(int display_width, int display_height,
                          int display_rounded_corner_radius);

void draw_game_canvas(Platform *p, GameOfLifeGridDimensions *dimensions,
                      GameCustomization *customization);

void enter_game_of_life_loop(Platform *p, GameCustomization *customization)
{

        LOG_DEBUG(TAG, "Entering Minesweeper game loop");
        GameOfLifeConfiguration config;

        collect_game_configuration(p, &config, customization);

        GameOfLifeGridDimensions *gd = calculate_grid_dimensions(
            p->display->get_width(), p->display->get_height(),
            p->display->get_display_corner_radius());
        int rows = gd->rows;
        int cols = gd->cols;

        draw_game_canvas(p, gd, customization);
        LOG_DEBUG(TAG, "Game of Life canvas drawn.");
}

void collect_game_configuration(Platform *p,
                                GameOfLifeConfiguration *game_config,
                                GameCustomization *customization)
{
        Configuration *config = assemble_game_of_life_configuration();
        enter_configuration_collection_loop(p, config,
                                            customization->accent_color);
        extract_game_config(game_config, config);
        free_configuration(config);
}

Configuration *assemble_game_of_life_configuration()
{
        Configuration *config = new Configuration();
        config->name = "Game of Life";

        // Initialize the first config option: game gridsize
        ConfigurationOption *choices = new ConfigurationOption();
        choices->name = "Spawn randomly";
        std::vector<const char *> available_values = {"Yes", "No"};
        populate_string_option_values(choices, available_values);
        choices->currently_selected = 1;

        ConfigurationOption *simulation_speed = new ConfigurationOption();
        simulation_speed->name = "Speed";
        std::vector<int> available_speeds = {1, 2, 3};
        populate_int_option_values(simulation_speed, available_speeds);
        simulation_speed->currently_selected = 1;

        config->options_len = 2;
        config->options = new ConfigurationOption *[config->options_len];
        config->options[0] = choices;
        config->options[1] = simulation_speed;
        config->curr_selected_option = 0;
        config->confirmation_cell_text = "Start Game";
        return config;
}

void extract_game_config(GameOfLifeConfiguration *game_config,
                         Configuration *config)
{

        ConfigurationOption prepopulate_grid = *config->options[0];
        int curr_choice_idx = prepopulate_grid.currently_selected;

        const char *choice = static_cast<const char **>(
            prepopulate_grid.available_values)[curr_choice_idx];
        if (strlen(choice) == 3 && strncmp(choice, "Yes", 3) == 0) {
                game_config->prepopulate_grid = true;
        }
        if (strlen(choice) == 2 && strncmp(choice, "No", 2) == 0) {
                game_config->prepopulate_grid = false;
        }
        // Grid size is the first config option in the game struct
        // above.
        ConfigurationOption simulation_speed = *config->options[1];

        int curr_speed_idx = simulation_speed.currently_selected;
        game_config->simulation_speed = static_cast<int *>(
            simulation_speed.available_values)[curr_speed_idx];
}

GameOfLifeGridDimensions *
calculate_grid_dimensions(int display_width, int display_height,
                          int display_rounded_corner_radius)
{
        // Bind input params to short names for improved readability.
        int w = display_width;
        int h = display_height;
        int r = display_rounded_corner_radius;

        int usable_width = w - r;
        int usable_height = h - r;

        int max_cols = usable_width / GAME_CELL_WIDTH;
        int max_rows = usable_height / GAME_CELL_WIDTH;

        int actual_width = max_cols * GAME_CELL_WIDTH;
        int actual_height = max_rows * GAME_CELL_WIDTH;

        // We calculate centering margins
        int left_horizontal_margin = (w - actual_width) / 2;
        int top_vertical_margin = (h - actual_height) / 2;

        LOG_DEBUG(TAG,
                  "Calculated grid dimensions: %d rows, %d cols, "
                  "left margin: %d, top margin: %d, actual width: %d, "
                  "actual height: %d",
                  max_rows, max_cols, left_horizontal_margin,
                  top_vertical_margin, actual_width, actual_height);

        return new GameOfLifeGridDimensions(
            max_rows, max_cols, top_vertical_margin, left_horizontal_margin,
            actual_width, actual_height);
}

void draw_game_canvas(Platform *p, GameOfLifeGridDimensions *dimensions,
                      GameCustomization *customization)

{
        p->display->initialize();
        p->display->clear(Black);
        p->display->draw_rounded_border(customization->accent_color);

        int x_margin = dimensions->left_horizontal_margin;
        int y_margin = dimensions->top_vertical_margin;

        int actual_width = dimensions->actual_width;
        int actual_height = dimensions->actual_height;

        int border_width = 2;
        // We need to make the border rectangle and the canvas slightly
        // bigger to ensure that it does not overlap with the game area.
        // Otherwise the caret rendering erases parts of the border as
        // it moves around (as the caret intersects with the border
        // partially)
        int border_offset = 1;

        p->display->draw_rectangle(
            {.x = x_margin - border_offset, .y = y_margin - border_offset},
            actual_width + 2 * border_offset, actual_height + 2 * border_offset,
            customization->accent_color, border_width, false);

        int text_below_grid_y = y_margin + actual_height + 2 * border_offset;
        int r = FONT_SIZE / 4;
        int d = 2 * r;
        int circle_y_axis = text_below_grid_y + FONT_SIZE / 2 + r / 4;
        const char *select = "Spawn";
        int select_len = strlen(select) * FONT_WIDTH;
        const char *exit = "Exit";
        int exit_len = strlen(exit) * FONT_WIDTH;
        const char *pause = "Pause";
        int pause_len = strlen(exit) * FONT_WIDTH;
        // We calculate the even spacing for the two indicators
        int explanations_num = 3;
        int circles_width = explanations_num * d;
        int text_circle_spacing_width = d;
        int total_width = select_len + exit_len + pause_len + circles_width +
                          (explanations_num - 1) * text_circle_spacing_width;
        int available_width = p->display->get_width() - 2 * x_margin;
        int remainder_space = available_width - total_width;
        int even_separator = remainder_space / (explanations_num + 1);

        int green_circle_x = x_margin + even_separator;
        p->display->draw_circle({.x = green_circle_x, .y = circle_y_axis}, r,
                                Green, 0, true);

        int spawn_text_x = green_circle_x + d;
        p->display->draw_string({.x = spawn_text_x, .y = text_below_grid_y},
                                (char *)select, FontSize::Size16, Black, White);

        int yellow_circle_x = spawn_text_x + select_len + even_separator;
        p->display->draw_circle({.x = yellow_circle_x, .y = circle_y_axis}, r,
                                Yellow, 0, true);
        int pause_text_x = yellow_circle_x + d;
        p->display->draw_string({.x = pause_text_x, .y = text_below_grid_y},
                                (char *)pause, FontSize::Size16, Black, White);

        int red_circle_x = pause_text_x + select_len + even_separator;
        p->display->draw_circle({.x = red_circle_x, .y = circle_y_axis}, r, Red,
                                0, true);
        int exit_text_x = red_circle_x + d;
        p->display->draw_string({.x = exit_text_x, .y = text_below_grid_y},
                                (char *)exit, FontSize::Size16, Black, White);
}
