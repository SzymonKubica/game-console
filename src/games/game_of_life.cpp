#include "../common/logging.hpp"
#include "../common/configuration.hpp"
#include "../common/constants.hpp"
#include "game_executor.hpp"
#include <cstring>

#define TAG "game_of_life"
#define GAME_CELL_WIDTH 10

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

typedef enum GameOfLifeCell {
        EMPTY = 0,
        ALIVE = 1,
} GameOfLifeCell;

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
void draw_caret(Display *display, Point *grid_position,
                GameOfLifeGridDimensions *dimensions, Color caret_color);
void erase_caret(Display *display, Point *grid_position,
                 GameOfLifeGridDimensions *dimensions,
                 Color grid_background_color);
void draw_game_cell(Display *display, Point *grid_position,
                    GameOfLifeGridDimensions *dimensions, Color color);

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

        Point caret_position = {.x = 0, .y = 0};
        draw_caret(p->display, &caret_position, gd,
                   customization->accent_color);

        std::vector<std::vector<GameOfLifeCell>> grid(
            rows, std::vector<GameOfLifeCell>(cols));

        while (true) {
                Direction dir;
                Action act;
                GameOfLifeCell curr = grid[caret_position.y][caret_position.x];
                if (directional_input_registered(p->directional_controllers,
                                                 &dir)) {
                        if (curr == EMPTY) {
                                erase_caret(p->display, &caret_position, gd,
                                            Black);
                        } else if (grid[caret_position.y][caret_position.x] ==
                                   ALIVE) {
                                erase_caret(p->display, &caret_position, gd,
                                            White);
                        }
                        translate_within_bounds(&caret_position, dir, gd->rows,
                                                gd->cols);
                        draw_caret(p->display, &caret_position, gd,
                                   customization->accent_color);

                        p->delay_provider->delay_ms(MOVE_REGISTERED_DELAY);
                        /* We continue here to skip the additional input
                           polling delay at the end of the loop and make
                           the input snappy. */
                        continue;
                }
                if (action_input_registered(p->action_controllers, &act)) {
                        switch (act) {
                        case YELLOW:
                                break;
                        case RED:
                                break;
                        case BLUE:
                                break;
                        case GREEN:
                                Color new_cell_color;
                                if (curr == EMPTY) {
                                        grid[caret_position.y]
                                            [caret_position.x] = ALIVE;
                                        new_cell_color = White;
                                } else if (curr == ALIVE) {
                                        grid[caret_position.y]
                                            [caret_position.x] = EMPTY;
                                        new_cell_color = Black;
                                }
                                draw_game_cell(p->display, &caret_position, gd,
                                               new_cell_color);
                                // we need to redraw the caret as we have just
                                // drawn a cell by clearing the region
                                draw_caret(p->display, &caret_position, gd,
                                           customization->accent_color);

                                break;
                        }
                        // When drawing the cell we need to wait longer
                        p->delay_provider->delay_ms(3* MOVE_REGISTERED_DELAY);
                }
        }
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

void draw_caret(Display *display, Point *grid_position,
                GameOfLifeGridDimensions *dimensions, Color caret_color)
{

        // We need to ensure that the caret is rendered INSIDE the text
        // cell and its border doesn't overlap the neighbouring cells.
        // Otherwise, we'll get weird rendering artifacts.
        int border_offset = 1;
        Point actual_position = {
            .x = dimensions->left_horizontal_margin +
                 grid_position->x * GAME_CELL_WIDTH + border_offset,
            .y = dimensions->top_vertical_margin +
                 grid_position->y * GAME_CELL_WIDTH + border_offset};

        display->draw_rectangle(
            actual_position, GAME_CELL_WIDTH - 2 * border_offset,
            GAME_CELL_WIDTH - 2 * border_offset, caret_color, 1, false);
}

void draw_game_cell(Display *display, Point *grid_position,
                    GameOfLifeGridDimensions *dimensions, Color color)
{
        Point actual_position = {.x = dimensions->left_horizontal_margin +
                                      grid_position->x * GAME_CELL_WIDTH,
                                 .y = dimensions->top_vertical_margin +
                                      grid_position->y * GAME_CELL_WIDTH};

        display->clear_region(actual_position,
                              {.x = actual_position.x + GAME_CELL_WIDTH,
                               .y = actual_position.y + GAME_CELL_WIDTH},
                              color);
}

void erase_caret(Display *display, Point *grid_position,
                 GameOfLifeGridDimensions *dimensions,
                 Color grid_background_color)
{

        // We need to ensure that the caret is rendered INSIDE the text
        // cell and its border doesn't overlap the neighbouring cells.
        // Otherwise, we'll get weird rendering artifacts.
        int border_offset = 1;
        Point actual_position = {
            .x = dimensions->left_horizontal_margin +
                 grid_position->x * GAME_CELL_WIDTH + border_offset,
            .y = dimensions->top_vertical_margin +
                 grid_position->y * GAME_CELL_WIDTH + border_offset};

        display->draw_rectangle(actual_position,
                                GAME_CELL_WIDTH - 2 * border_offset,
                                GAME_CELL_WIDTH - 2 * border_offset,
                                grid_background_color, 1, false);
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

        /* Rendering of help indicators below the grid */
        int text_below_grid_y = y_margin + actual_height + 2 * border_offset;
        int r = FONT_SIZE / 4;
        int d = 2 * r;
        int circle_y_axis = text_below_grid_y + FONT_SIZE / 2 + r / 4;
        const char *select = "Spawn";
        int select_len = strlen(select) * FONT_WIDTH;
        const char *exit = "Exit";
        int exit_len = strlen(exit) * FONT_WIDTH;
        const char *pause = "Pause";
        int pause_len = strlen(pause) * FONT_WIDTH;
        // We calculate the even spacing for the two indicators
        int explanations_num = 3;
        int circles_width = explanations_num * d;
        int text_circle_spacing_width = d;
        int total_width = select_len + exit_len + pause_len + circles_width +
                          (explanations_num - 1) * text_circle_spacing_width;
        int available_width = p->display->get_width() - 2 * x_margin;
        int remainder_space = available_width - total_width;
        int even_separator = remainder_space / explanations_num;

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

        /* Rendering of help indicators above the grid */
        int text_grid_spacing = 4;
        int text_above_grid_y =
            y_margin - border_offset - FONT_SIZE - text_grid_spacing;
        int circle_y_axis_above_grid =
            text_above_grid_y + (FONT_SIZE / 2 + r / 2);

        const char *toggle = "Rewind mode on/off";
        int toggle_len = strlen(toggle) * FONT_WIDTH;

        int total_width_above_grid = toggle_len + d + text_circle_spacing_width;
        int centering_margin = (available_width - total_width_above_grid) / 2;

        int blue_circle_x = x_margin + centering_margin;
        p->display->draw_circle(
            {.x = blue_circle_x, .y = circle_y_axis_above_grid}, r, DarkBlue, 0,
            true);
        int toggle_text_x = blue_circle_x + d;
        p->display->draw_string({.x = toggle_text_x, .y = text_above_grid_y},
                                (char *)toggle, FontSize::Size16, Black, White);
}
