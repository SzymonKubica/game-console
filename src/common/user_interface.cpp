#include "user_interface.h"
#include "configuration.hpp"
#include "../lib/GUI_Paint.h"
#include "../lib/LCD_Driver.h"
#include "display/color.hpp"
#include "display/display.hpp"
#include <cassert>
#include <cstdio>

/* Constants for configuring the UI. */
#define FONT_SIZE 16
#define FONT_WIDTH 11
#define HEADING_FONT_SIZE 24
#define HEADING_FONT_WIDTH 17
#define TOP_LEFT_CORNER_X 23
#define TOP_LEFT_CORNER_Y 38

#define DISPLAY_CORNER_RADIUS 40
#define SCREEN_BORDER_WIDTH 3

#define GRID_BG_COLOR White

/*******************************************************************************
  User Interface
*******************************************************************************/

static void draw_game_grid(Display *display, int grid_size);
void draw_game_canvas(Display *display, GameState *state)
{
        display->initialize();
        display->clear(Black);
        display->draw_rounded_border(DarkBlue);
        Point top_left = {.x = 70, .y = 70};
        draw_game_grid(display, state->grid_size);
}

/// Struct modelling all dimensional information required to properly render
/// and space out the grid slots that are used to display the game tiles.
typedef struct GridDimensions {
        int cell_height;
        int cell_width;
        int cell_x_spacing;
        int cell_y_spacing;
        int padding; // Padding added to the left of the grid if the available
                     // space isn't evenly divided into grid_size
        int grid_start_x;
        int grid_start_y;
        int score_cell_height;
        int score_cell_width;
        int score_start_x;
        int score_start_y;
        int score_title_x;
        int score_title_y;
} GridDimensions;

GridDimensions *calculate_grid_dimensions(int grid_size)
{
        GridDimensions *gd = (GridDimensions *)malloc(sizeof(GridDimensions));
        // HEIGHT and WIDTH are swapped because the display is mounted
        // horizontally. We subtract 4 times the border width to add padding
        // around the grid.
        int usable_width = LCD_HEIGHT - 2 * SCREEN_BORDER_WIDTH;
        int usable_height = LCD_WIDTH - 2 * DISPLAY_CORNER_RADIUS;

        int cell_height = FONT_SIZE + FONT_SIZE / 2;
        int cell_width = 4 * FONT_WIDTH + FONT_WIDTH / 2;

        int cell_y_spacing =
            (usable_height - cell_height * grid_size) / (grid_size - 1);
        int cell_x_spacing =
            (usable_width - cell_width * grid_size) / (grid_size + 1);

        // We need to calculate the remainder width and then add a half of it
        // to the starting point to make the grid centered in case the usable
        // height doesn't divide evenly into grid_size.
        int remainder_width = (usable_width - (grid_size + 1) * cell_x_spacing -
                               grid_size * cell_width);

        // We offset the grid downwards to allow it to overlap with the gap
        // between the two bottom corners and save space for the score at the
        // top of the grid.
        int corner_offset = DISPLAY_CORNER_RADIUS / 4;

        int grid_start_x =
            SCREEN_BORDER_WIDTH + cell_x_spacing + remainder_width / 2;
        int grid_start_y =
            SCREEN_BORDER_WIDTH + DISPLAY_CORNER_RADIUS + corner_offset;

        // We first draw a slot for the score
        int score_cell_width =
            LCD_HEIGHT - 2 * (SCREEN_BORDER_WIDTH + DISPLAY_CORNER_RADIUS);
        int score_cell_height = cell_height;

        int score_start_y =
            (grid_start_y - score_cell_height - SCREEN_BORDER_WIDTH) / 2 +
            SCREEN_BORDER_WIDTH;

        Point score_start = {.x = SCREEN_BORDER_WIDTH + DISPLAY_CORNER_RADIUS,
                             .y = score_start_y};

        int score_title_x = score_start.x + cell_x_spacing;

        int score_title_y = score_start.y + (score_cell_height - FONT_SIZE) / 2;

        gd->cell_height = cell_height;
        gd->cell_width = cell_width;
        gd->cell_x_spacing = cell_x_spacing;
        gd->cell_y_spacing = cell_y_spacing;
        gd->padding = remainder_width;
        gd->grid_start_x = grid_start_x;
        gd->grid_start_y = grid_start_y;

        gd->score_cell_height = score_cell_height;
        gd->score_cell_width = score_cell_width;
        gd->score_start_x = score_start.x;
        gd->score_start_y = score_start.y;
        gd->score_title_x = score_title_x;
        gd->score_title_y = score_title_y;
}

/**
 * Draws the background slots for the grid tiles, this takes a long time and
 * should only be called once at the start of the game to draw the grid. After
 * that update_game_grid should be called to update the contents of the grid slots
 * with the numbers.
 */
static void draw_game_grid(Display *display, int grid_size)
{

        GridDimensions *gd = calculate_grid_dimensions(grid_size);

        Point score_start = {.x = gd->score_start_x, .y = gd->score_start_y};
        display->draw_rounded_rectangle(
            score_start, gd->score_cell_width, gd->score_cell_height,
            gd->score_cell_height / 2, GRID_BG_COLOR);

        char *buffer = "Score:";

        Point score_title = {.x = gd->score_title_x, .y = gd->score_title_y};
        display->draw_string(score_title, buffer, Size16, GRID_BG_COLOR, Black);

        for (int i = 0; i < grid_size; i++) {
                for (int j = 0; j < grid_size; j++) {
                        Point start = {
                            .x = gd->grid_start_x +
                                 j * (gd->cell_width + gd->cell_x_spacing),
                            .y = gd->grid_start_y +
                                 i * (gd->cell_height + gd->cell_y_spacing)};

                        display->draw_rounded_rectangle(
                            start, gd->cell_width, gd->cell_height,
                            gd->cell_height / 2, GRID_BG_COLOR);
                }
        }

        free(gd);
}

static void str_replace(char *str, char *oldWord, char *newWord);
static int number_string_length(int number);

void update_game_grid(Display *display, GameState *gs)
{
        int grid_size = gs->grid_size;
        GridDimensions *gd = calculate_grid_dimensions(grid_size);

        int score_title_length = 6 * FONT_WIDTH;
        char score_buffer[20];
        sprintf(score_buffer, "%d", gs->score);

        int score_rounding_radius = gd->score_cell_height / 2;

        Point clear_start = {.x = gd->score_title_x + score_title_length +
                                  FONT_WIDTH,
                             gd->score_title_y};
        Point clear_end = {.x = gd->score_start_x + gd->score_cell_width -
                                score_rounding_radius,
                           gd->score_title_y + FONT_SIZE};

        display->clear_region(clear_start, clear_end, GRID_BG_COLOR);

        Point score_start = {.x = gd->score_title_x + score_title_length +
                                  FONT_WIDTH,
                             .y = gd->score_title_y};

        display->draw_string(score_start, score_buffer, Size16, GRID_BG_COLOR,
                             Black);

        for (int i = 0; i < grid_size; i++) {
                for (int j = 0; j < grid_size; j++) {
                        Point start = {
                            .x = gd->grid_start_x +
                                 j * (gd->cell_width + gd->cell_x_spacing),
                            .y = gd->grid_start_y +
                                 i * (gd->cell_height + gd->cell_y_spacing)};

                        if (gs->grid[i][j] != gs->old_grid[i][j]) {
                                char *buffer = (char *)malloc(5 * sizeof(char));
                                sprintf(buffer, "%4d", gs->grid[i][j]);
                                str_replace(buffer, "   0", "    ");
                                // We need to center the four characters of text
                                // inside of the cell.
                                int x_margin =
                                    (gd->cell_width - 4 * FONT_WIDTH) / 2;
                                int y_margin =
                                    (gd->cell_height - FONT_SIZE) / 2;
                                int digit_len =
                                    number_string_length(gs->old_grid[i][j]);

                                Point clear_start = {.x = start.x + x_margin +
                                                          (4 - digit_len) *
                                                              FONT_WIDTH,
                                                     .y = start.y + y_margin};
                                Point clear_end = {
                                    .x = start.x + x_margin + 4 * FONT_WIDTH,
                                    .y = start.y + y_margin + FONT_SIZE};
                                display->clear_region(clear_start, clear_end,
                                                      GRID_BG_COLOR);

                                Point start_with_margin = {.x = start.x +
                                                                x_margin,
                                                           start.y + y_margin};
                                display->draw_string(start_with_margin, buffer,
                                                     Size16, GRID_BG_COLOR,
                                                     Black);
                                gs->old_grid[i][j] = gs->grid[i][j];
                                free(buffer);
                        }
                }
        }
        free(gd);
}

static int number_string_length(int number)
{
        if (number >= 1000) {
                return 4;
        } else if (number >= 100) {
                return 3;
        } else if (number >= 10) {
                return 2;
        }
        return 1;
}

static void str_replace(char *str, char *oldWord, char *newWord)
{
        char *pos, temp[1000];
        int index = 0;
        int owlen;

        owlen = strlen(oldWord);

        while ((pos = strstr(str, oldWord)) != NULL) {
                strcpy(temp, str);
                index = pos - str;
                str[index] = '\0';
                strcat(str, newWord);
                strcat(str, temp + index + owlen);
        }
}

void draw_game_over(Display *display, GameState *state)
{
        display->draw_rounded_border(Red);

        char *msg = "Game Over";

        int x_pos = (LCD_HEIGHT - strlen(msg) * FONT_WIDTH) / 2;
        int y_pos = (LCD_WIDTH - FONT_SIZE) / 2;

        Point text_position = {.x = x_pos, .y = y_pos};

        display->draw_string(text_position, msg, Size16, Black, Red);
}

void draw_game_won(Display *display, GameState *state)
{
        display->draw_rounded_border(Green);

        char *msg = "You Won!";

        int x_pos = (LCD_HEIGHT - strlen(msg) * FONT_WIDTH) / 2;
        int y_pos = (LCD_WIDTH - FONT_SIZE) / 2;
        Point text_position = {.x = x_pos, .y = y_pos};

        display->draw_string(text_position, msg, Size16, Black, Green);
}

/// Draws the config menu given the old and new config values
/// The old config given in `previous_config` is needed to determine which parts
/// of the UI need to be updated
void draw_configuration_menu(Display *display, GameConfiguration *config,
                           GameConfiguration *previous_config, bool update)
{
        // First set up all aata required to print
        char *heading = "2048";
        char *grid_size_str = "Grid size:";
        char *target_tile = "Game target:";
        char *start = "Start Game";

        int text_max_length = strlen(target_tile) + 2 + 4; // max 4096 target
        int left_margin = (LCD_HEIGHT - text_max_length * FONT_WIDTH) / 2;

        int spacing = (LCD_WIDTH - 6 * FONT_SIZE - HEADING_FONT_SIZE) / 3;

        int heading_x_pos =
            (LCD_HEIGHT - strlen(heading) * HEADING_FONT_WIDTH) / 2;

        // First calculate the positions of the two configuration cells.
        int grid_size_y_pos = 2 * spacing + FONT_SIZE;
        int target_tile_y_pos = 2 * spacing + 4 * FONT_SIZE;
        int start_tile_y_pos = 2 * spacing + 7 * FONT_SIZE;

        Point grid_size_cell_start = {.x = left_margin - FONT_WIDTH / 2,
                                      .y = grid_size_y_pos - FONT_SIZE / 2};

        Point target_tile_cell_start = {.x = left_margin - FONT_WIDTH / 2,
                                        .y = target_tile_y_pos - FONT_SIZE / 2};

        Point start_tile_cell_start = {.x = left_margin - FONT_WIDTH / 2,
                                       .y = start_tile_y_pos - FONT_SIZE / 2};

        Point grid_size_modifiable_cell_start = {
            .x = (int)(left_margin + (strlen(target_tile) + 1) * FONT_WIDTH),
            .y = grid_size_y_pos - FONT_SIZE / 4};

        Point target_tile_modifiable_cell_start = {
            .x = (int)(left_margin + (strlen(target_tile) + 1) * FONT_WIDTH),
            .y = target_tile_y_pos - FONT_SIZE / 4};

        int option_cell_width = (text_max_length + 1) * FONT_WIDTH;
        int modifiable_cell_width = (5) * FONT_WIDTH;

        Color cell_bg_color = DarkBlue;

        if (!update) {
                display->initialize();
                display->clear(Black);

                Point heading_start = {.x = heading_x_pos, .y = spacing};
                display->draw_string(heading_start, heading, Size24, Black,
                                     White);

                // Draw the background for the two configuration cells.
                display->draw_rounded_rectangle(
                    grid_size_cell_start, option_cell_width, FONT_SIZE * 2,
                    FONT_SIZE, cell_bg_color);
                display->draw_rounded_rectangle(
                    target_tile_cell_start, option_cell_width, FONT_SIZE * 2,
                    FONT_SIZE, cell_bg_color);
                display->draw_rounded_rectangle(
                    start_tile_cell_start, option_cell_width, FONT_SIZE * 2,
                    FONT_SIZE, cell_bg_color);

                Point grid_size_str_start = {.x = left_margin,
                                             .y = grid_size_y_pos};
                display->draw_string(grid_size_str_start, grid_size_str, Size16,
                                     cell_bg_color, White);
                Point target_tile_str_start = {.x = left_margin,
                                               .y = target_tile_y_pos};
                display->draw_string(target_tile_str_start, target_tile, Size16,
                                     cell_bg_color, White);

                int start_game_margin =
                    (LCD_HEIGHT - strlen(start) * FONT_WIDTH) / 2;
                display->draw_string(
                    {.x = start_game_margin, .y = start_tile_y_pos}, start,
                    Size16, cell_bg_color, White);
        }

        if (!update || (update && config->config_option !=
                                      previous_config->config_option)) {
                // First clear both circles
                int selector_circle_radius = 5;
                Paint_DrawCircle(left_margin + option_cell_width + FONT_WIDTH,
                                 grid_size_y_pos + FONT_SIZE / 2,
                                 selector_circle_radius, BLACK, DOT_PIXEL_1X1,
                                 DRAW_FILL_FULL);
                Paint_DrawCircle(left_margin + option_cell_width + FONT_WIDTH,
                                 target_tile_y_pos + FONT_SIZE / 2,
                                 selector_circle_radius, BLACK, DOT_PIXEL_1X1,
                                 DRAW_FILL_FULL);
                Paint_DrawCircle(left_margin + option_cell_width + FONT_WIDTH,
                                 start_tile_y_pos + FONT_SIZE / 2,
                                 selector_circle_radius, BLACK, DOT_PIXEL_1X1,
                                 DRAW_FILL_FULL);
                if (config->config_option == 0) {
                        Paint_DrawCircle(left_margin + option_cell_width +
                                             FONT_WIDTH,
                                         grid_size_y_pos + FONT_SIZE / 2,
                                         selector_circle_radius, cell_bg_color,
                                         DOT_PIXEL_1X1, DRAW_FILL_FULL);
                } else if (config->config_option == 1) {
                        Paint_DrawCircle(left_margin + option_cell_width +
                                             FONT_WIDTH,
                                         target_tile_y_pos + FONT_SIZE / 2,
                                         selector_circle_radius, cell_bg_color,
                                         DOT_PIXEL_1X1, DRAW_FILL_FULL);
                } else {
                        Paint_DrawCircle(left_margin + option_cell_width +
                                             FONT_WIDTH,
                                         start_tile_y_pos + FONT_SIZE / 2,
                                         selector_circle_radius, cell_bg_color,
                                         DOT_PIXEL_1X1, DRAW_FILL_FULL);
                }
        }

        if (!update || config->grid_size != previous_config->grid_size) {
                char grid_size_buffer[5];
                sprintf(grid_size_buffer, "%4d", config->grid_size);
                display->draw_rounded_rectangle(
                    grid_size_modifiable_cell_start,
                    modifiable_cell_width, FONT_SIZE + FONT_SIZE / 2,
                    (FONT_SIZE + FONT_SIZE / 2) / 2, White);
                display->draw_string(
                    {.x = grid_size_modifiable_cell_start.x + FONT_WIDTH / 2,
                     .y = grid_size_modifiable_cell_start.y + FONT_SIZE / 4},
                    grid_size_buffer, Size16, White, Black);
        }

        if (!update ||
            config->target_max_tile != previous_config->target_max_tile) {
                char game_target_buffer[5];
                sprintf(game_target_buffer, "%4d", config->target_max_tile);

                // Draw / clear the modifiable cells
                display->draw_rounded_rectangle(target_tile_modifiable_cell_start,
                                     modifiable_cell_width,
                                     FONT_SIZE + FONT_SIZE / 2,
                                     (FONT_SIZE + FONT_SIZE / 2) / 2, White);

                display->draw_string({.x=
                    grid_size_modifiable_cell_start.x + FONT_WIDTH / 2,
                    .y = target_tile_modifiable_cell_start.y + FONT_SIZE / 4},
                    game_target_buffer, Size16, White, Black);
        }
}

void render_generic_config_menu(Configuration *config, ConfigurationDiff *diff,
                             bool update)
{

        int text_max_length = find_max_config_option_name_length(config) + 2;
        int left_margin = (LCD_HEIGHT - text_max_length * FONT_WIDTH) / 2;
        int spacing = (LCD_WIDTH - config->config_values_len * FONT_SIZE -
                       HEADING_FONT_SIZE) /
                      3;

        // TODO: add rendering similar to the 2048-specific menu
        //
        // 1. Step one: abstract away an interface that is required for drawing
        //    strings, rounded rects and clearing the screen
        // 2. Make the lib implement it
        // 3. use ncurses or some other rendering thing to implement it
        // 4. figure out how to test it without the actual UI.
}
