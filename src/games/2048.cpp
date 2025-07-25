#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <cstring>
#include <string>
#include "2048.h"

#include "../common/logging.hpp"
#include "../common/platform/interface/display.hpp"
#include "../common/platform/interface/platform.hpp"
#include "../common/user_interface.h"

#define TAG "2048"
#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

// TODO: move those to the display interface or some shared library.
// the code shouldn't be littered with those random static defines.
#define FONT_SIZE 16
//// We need to figure out how to deal with the font width
// for the emulator as obviously it is not pixel-accurate the same as what we
// have on the actual hardware
// #define FONT_WIDTH 11 todo: move this to some constants definition file so
// that we can dynamically switch between font sizes between targets.
#ifdef EMULATOR
#define FONT_WIDTH 10
#endif
#ifndef EMULATOR
#define FONT_WIDTH 11
#endif
#define GRID_BG_COLOR White

// this should be configurable
#define SCREEN_BORDER_WIDTH 3

// This should be sourced from a module that controlls those constants
#define INPUT_POLLING_DELAY 50
#define MOVE_REGISTERED_DELAY 150

static void copy_grid(int **source, int **destination, int size);

void initialize_randomness_seed(int seed) { srand(seed); }

static void handle_game_over(Display *display,
                             std::vector<Controller *> *controllers,
                             GameState *state);
static void handle_game_finished(Display *display,
                                 std::vector<Controller *> *controllers,
                                 GameState *state);
static void collect_game_configuration(Platform *p, GameConfiguration *config);
static bool input_registered(std::vector<Controller *> *controllers,
                             Direction *registered_dir);
static void pause_until_input(std::vector<Controller *> *controllers,
                              DelayProvider *delay_provider);

void enter_game_loop(Platform *p)
{
        GameConfiguration config;
        collect_game_configuration(p, &config);

        GameState *state =
            initialize_game_state(config.grid_size, config.target_max_tile);

        draw_game_canvas(p->display, state);
        update_game_grid(p->display, state);
        p->display->refresh();

        while (!is_game_over(state) || !is_game_finished(state)) {
                Direction dir;
                if (input_registered(p->controllers, &dir)) {
                        LOG_DEBUG(TAG, "Input received: %s",
                                  direction_to_str(dir))
                        take_turn(state, (int)dir);
                        update_game_grid(p->display, state);
                        p->delay_provider->delay_ms(MOVE_REGISTERED_DELAY);
                }
                p->delay_provider->delay_ms(INPUT_POLLING_DELAY);
                p->display->refresh();
        }

        if (is_game_over(state)) {
                draw_game_over(p->display, state);
        }
        if (is_game_finished(state)) {
                draw_game_won(p->display, state);
        }

        pause_until_input(p->controllers, p->delay_provider);
}

void pause_until_input(std::vector<Controller *> *controllers,
                       DelayProvider *delay_provider)
{
        Direction dir;
        while (!input_registered(controllers, &dir)) {
                delay_provider->delay_ms(INPUT_POLLING_DELAY);
        };
}

/**
 * Checks if any of the controllers has recorded user input. If so, the input
 * direction will be written into the `registered_dir` output parameter.
 */
bool input_registered(std::vector<Controller *> *controllers,
                      Direction *registered_dir)
{
        bool input_registered = false;
        for (Controller *controller : *controllers) {
                input_registered |= controller->poll_for_input(registered_dir);
        }
        return input_registered;
}

void collect_game_configuration(Platform *p, GameConfiguration *config)
{
        const int AVAILABLE_OPTIONS = 3;
        const int GRID_SIZES_LEN = 3;
        const int TARGET_MAX_TILES_LEN = 6;

        int available_grid_sizes[] = {3, 4, 5};
        int available_target_max_tiles[] = {128, 256, 512, 1024, 2048, 4096};
        ConfigOption available_config_options[] = {GRID_SIZE, TARGET_MAX_TILE,
                                                   READY_TO_GO};

        int curr_opt_idx = 0;
        int grid_size_idx = 1;
        int game_target_idx = 4;

        config->grid_size = available_grid_sizes[grid_size_idx];
        config->target_max_tile = available_target_max_tiles[game_target_idx];
        config->config_option = available_config_options[curr_opt_idx];

        render_config_menu(p->display, config, config, false);

        while (true) {
                Direction dir;
                bool ready = false;
                if (input_registered(p->controllers, &dir)) {
                        GameConfiguration old_config;
                        old_config.grid_size = config->grid_size;
                        old_config.target_max_tile = config->target_max_tile;
                        old_config.config_option = config->config_option;
                        switch (dir) {
                        case DOWN:
                                curr_opt_idx =
                                    (curr_opt_idx + 1) % AVAILABLE_OPTIONS;
                                break;
                        case UP:
                                if (curr_opt_idx == 0) {
                                        curr_opt_idx = AVAILABLE_OPTIONS - 1;
                                } else {
                                        curr_opt_idx--;
                                }
                                break;
                        case LEFT:
                                if (curr_opt_idx == GRID_SIZE) {
                                        if (grid_size_idx == 0) {
                                                grid_size_idx =
                                                    GRID_SIZES_LEN - 1;
                                        } else {
                                                grid_size_idx--;
                                        }
                                } else {
                                        if (game_target_idx == 0) {
                                                game_target_idx =
                                                    TARGET_MAX_TILES_LEN - 1;
                                        } else {
                                                game_target_idx--;
                                        }
                                }
                                break;
                        case RIGHT:
                                if (curr_opt_idx == GRID_SIZE) {
                                        grid_size_idx = (grid_size_idx + 1) %
                                                        GRID_SIZES_LEN;
                                } else if (curr_opt_idx == TARGET_MAX_TILE) {
                                        game_target_idx =
                                            (game_target_idx + 1) %
                                            TARGET_MAX_TILES_LEN;
                                } else {
                                        ready = true;
                                }
                                break;
                        }

                        config->grid_size = available_grid_sizes[grid_size_idx];
                        config->target_max_tile =
                            available_target_max_tiles[game_target_idx];
                        config->config_option =
                            available_config_options[curr_opt_idx];
                        render_config_menu(p->display, config, &old_config,
                                           true);
                        p->delay_provider->delay_ms(MOVE_REGISTERED_DELAY);
                        if (ready) {
                                break;
                        }
                }
                p->delay_provider->delay_ms(INPUT_POLLING_DELAY);
        }
}

// This is supposed to be the generic function used for collecting the generic
// input. In the final state the `collect_game_configuration` is to be replaced
// by this. This is to be migrated to some c++ source file so that it can be
// reused for different games. The idea here is that each game would define the
// config struct that it needs to collect and the config menu would get rendered
// automatically. This is work in progress and is currently not used anywhere
/*
void collect_generic_config(Configuration *config)
{
        LcdDisplay display = LcdDisplay{};
        // We start with an empty diff object
        ConfigurationDiff diff;
        render_generic_config_menu(&display, config, &diff, false);

        while (true) {
                Direction dir;
                bool input_registered = false;

                input_registered |= joystick_controller->poll_for_input(&dir);
                input_registered |= keypad_controller->poll_for_input(&dir);

                bool ready = false;
                if (input_registered) {
                        Configuration old_config;
                        switch (dir) {
                        case DOWN:
                                switch_edited_config_option_up(config, &diff);
                                break;
                        case UP:
                                switch_edited_config_option_down(config, &diff);
                                break;
                        case LEFT:
                                switch_current_config_option_down(config,
                                                                  &diff);
                                break;
                        case RIGHT:
                                if (config->current_config_value ==
                                    config->config_values_len - 1) {
                                        ready = true;
                                } else {
                                        switch_current_config_option_up(config,
                                                                        &diff);
                                }
                                break;
                        }

                        render_generic_config_menu(&display, config, &diff,
                                                   true);
                        delay(MOVE_REGISTERED_DELAY);
                        if (ready) {
                                break;
                        }
                }
                delay(INPUT_POLLING_DELAY);
        }
}
*/

/* Initialization Code */

static void spawn_tile(GameState *gs);

int **create_game_grid(int size);
GameState *initialize_game_state(int size, int target_max_tile)
{
        GameState *gs =
            new GameState(create_game_grid(size), create_game_grid(size), 0, 0,
                          size, target_max_tile);

        spawn_tile(gs);
        return gs;
}

void free_game_grid(int **grid, int size);
void free_game_state(GameState *gs)
{
        free_game_grid(gs->grid, gs->grid_size);
        free(gs);
}

// Allocates a new game grid as a two-dimensional array
int **create_game_grid(int size)
{
        int **g = (int **)malloc(size * sizeof(int *));
        for (int i = 0; i < size; i++) {
                g[i] = (int *)malloc(size * sizeof(int));
                for (int j = 0; j < size; j++) {
                        g[i][j] = 0; // Initialize all tiles to 0
                }
        }
        return g;
}

void free_game_grid(int **grid, int size)
{
        for (int i = 0; i < size; i++) {
                free(grid[i]);
        }
        free(grid);
}

/* Tile Spawning */

static int generate_new_tile_value()
{
        if (rand() % 10 == 1) {
                return 4;
        }
        return 2;
}
static int get_random_coordinate(int grid_size) { return rand() % grid_size; }

static void spawn_tile(GameState *gs)
{
        bool success = false;
        while (!success) {
                int x = get_random_coordinate(gs->grid_size);
                int y = get_random_coordinate(gs->grid_size);

                if (gs->grid[x][y] == 0) {
                        gs->grid[x][y] = generate_new_tile_value();
                        success = true;
                }
        }
        gs->occupied_tiles++;
}

/* Tile Merging Logic */

/* Helper functions for tile merging */

/// Merges the i-th row of tiles in the given direction (left/right).
static void merge_row(GameState *gs, int i, int direction);

/// Reverses a given row of `row_size` elements in place.
static void reverse(int *row, int row_size);

/// Transposes the game trid in place to allow for merging vertically.
static void transpose(GameState *gs);

/// Returns the index of the next non-empty tile after the `current_index` in
/// the i-th row in of the grid.
static int get_successor_index(GameState *gs, int i, int current_index);

// We only implement merging tiles left or right (row-wise), in order to merge
// the tiles in a vertical direction we first transpose the grid, merge and then
// transpose back.
static void merge(GameState *gs, int direction)
{
        if (direction == UP || direction == DOWN) {
                transpose(gs);
        }

        for (int i = 0; i < gs->grid_size; i++) {
                merge_row(gs, i, direction);
        }

        if (direction == UP || direction == DOWN) {
                transpose(gs);
        }
}

static void merge_row(GameState *gs, int i, int direction)
{
        int curr = 0;
        int *merged_row = (int *)malloc(gs->grid_size * sizeof(int));
        for (int j = 0; j < gs->grid_size; j++) {
                merged_row[j] = 0; // Initialize the merged row
        }
        int merged_num = 0;

        int size = gs->grid_size;

        // We always merge to the left (or up if we have previously transposed
        // the grid), in other cases we reverse the row, merge and reverse back.
        if (direction == DOWN || direction == RIGHT) {
                reverse(gs->grid[i], size);
        }

        // We find the first non-empty tile.
        curr = get_successor_index(gs, i, -1);

        if (curr == size) {
                // All tiles are empty.
                return;
        }

        // Now the current tile must be non-empty.
        while (curr < size) {
                int succ = get_successor_index(gs, i, curr);
                // Two matching tiles found, we perform a merge.
                int curr_val = gs->grid[i][curr];
                if (succ < size && curr_val == gs->grid[i][succ]) {
                        int sum = curr_val + gs->grid[i][succ];
                        gs->score += sum;
                        gs->occupied_tiles--;
                        merged_row[merged_num] = sum;
                        curr = get_successor_index(gs, i, succ);
                } else {
                        merged_row[merged_num] = curr_val;
                        curr = succ;
                }
                merged_num++;
        }

        for (int j = 0; j < size; j++) {
                if (direction == DOWN || direction == RIGHT) {
                        gs->grid[i][gs->grid_size - 1 - j] = merged_row[j];
                } else {
                        gs->grid[i][j] = merged_row[j];
                }
        }
        free(merged_row);
}

// Reverses a given row of four elements in place
static void reverse(int *row, int row_size)
{
        for (int i = 0; i < row_size / 2; i++) {
                int temp = row[i];
                row[i] = row[row_size - i - 1];
                row[row_size - i - 1] = temp;
        }
}

static int get_successor_index(GameState *gs, int i, int current_index)
{
        int succ = current_index + 1;
        while (succ < gs->grid_size && gs->grid[i][succ] == 0) {
                succ++;
        }
        return succ;
}

static void transpose(GameState *gs)
{
        for (int i = 0; i < gs->grid_size; i++) {
                for (int j = i; j < gs->grid_size; j++) {
                        int temp = gs->grid[i][j];
                        gs->grid[i][j] = gs->grid[j][i];
                        gs->grid[j][i] = temp;
                }
        }
}

/* Game Loop Logic */

// Helper functions for the game loop
static bool grid_changed_from(GameState *gs, int **oldGrid);
static bool is_board_full(GameState *gs);
static bool no_move_possible(GameState *gs);

bool is_game_over(GameState *gs)
{
        return is_board_full(gs) && no_move_possible(gs);
}

bool is_game_finished(GameState *gs)
{
        for (int i = 0; i < gs->grid_size; i++) {
                for (int j = 0; j < gs->grid_size; j++) {
                        if (gs->grid[i][j] == gs->target_max_tile) {
                                return true;
                        }
                }
        }
        return false;
}

static bool is_board_full(GameState *gs)
{
        return gs->occupied_tiles >= gs->grid_size * gs->grid_size;
}

void take_turn(GameState *gs, int direction)
{
        int **oldGrid = create_game_grid(gs->grid_size);
        copy_grid(gs->grid, oldGrid, gs->grid_size);
        merge(gs, direction);

        if (grid_changed_from(gs, oldGrid)) {
                spawn_tile(gs);
        }
        free_game_grid(oldGrid, gs->grid_size);
}

static bool grid_changed_from(GameState *gs, int **oldGrid)
{
        for (int i = 0; i < gs->grid_size; i++) {
                for (int j = 0; j < gs->grid_size; j++) {
                        if (gs->grid[i][j] != oldGrid[i][j]) {
                                return true;
                        }
                }
        }
        return false;
}

static bool no_move_possible(GameState *gs)
{
        /* A move is always possible if not all tiles are occupied. Hence,
           we short-circuit here. */
        if (gs->occupied_tiles < gs->grid_size * gs->grid_size) {
                return false;
        }

        /* When the grid is full a move is possible as long as there exist some
           adjacent tiles that have the same number */
        for (int i = 0; i < gs->grid_size; i++) {
                for (int j = 0; j < gs->grid_size - 1; j++) {
                        // row-wise adjacency
                        if (gs->grid[i][j] == gs->grid[i][j + 1]) {
                                return false;
                        }
                        // colunm-wise adjacency
                        if (gs->grid[j][i] == gs->grid[j + 1][i]) {
                                return false;
                        }
                }
        }
        return true;
}

static void copy_grid(int **source, int **destination, int size)
{
        for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                        destination[i][j] = source[i][j];
                }
        }
}

/* Grid Drawing */

/**
 * Draws the background slots for the grid tiles, this takes a long time and
 * should only be called once at the start of the game to draw the grid. After
 * that `update_game_grid` should be called to update the contents of the grid
 * slots with the numbers.
 */
static void draw_game_grid(Display *display, int grid_size);
void draw_game_canvas(Display *display, GameState *state)
{
        display->initialize();
        display->clear(Black);
        display->draw_rounded_border(DarkBlue);
        draw_game_grid(display, state->grid_size);
}

/**
 * Class storing all dimensional information required to properly render
 * and space out the grid slots that are used to display the game tiles.
 */
class GridDimensions
{
      public:
        int cell_height;
        int cell_width;
        int cell_x_spacing;
        int cell_y_spacing;
        /* Padding added to the left of the grid if the available
        space isn't evenly divided into grid_size */
        int padding;
        int grid_start_x;
        int grid_start_y;
        int score_cell_height;
        int score_cell_width;
        int score_start_x;
        int score_start_y;
        int score_title_x;
        int score_title_y;

        GridDimensions(int cell_height, int cell_width, int cell_x_spacing,
                       int cell_y_spacing, int padding, int grid_start_x,
                       int grid_start_y, int score_cell_height,
                       int score_cell_width, int score_start_x,
                       int score_start_y, int score_title_x, int score_title_y)
            : cell_height(cell_height), cell_width(cell_width),
              cell_x_spacing(cell_x_spacing), cell_y_spacing(cell_y_spacing),
              padding(padding), grid_start_x(grid_start_x),
              grid_start_y(grid_start_y), score_cell_height(score_cell_height),
              score_cell_width(score_cell_width), score_start_x(score_start_x),
              score_start_y(score_start_y), score_title_x(score_title_x),
              score_title_y(score_title_y)
        {
        }
};

GridDimensions *calculate_grid_dimensions(Display *display, int grid_size)
{
        int height = display->get_height();
        int width = display->get_width();
        int corner_radius = display->get_display_corner_radius();
        int usable_width = width - 2 * SCREEN_BORDER_WIDTH;
        int usable_height = height - 2 * corner_radius;

        int cell_height = FONT_SIZE + FONT_SIZE / 2;
        int cell_width = 4 * FONT_WIDTH + (FONT_WIDTH / 2);

        int cell_y_spacing =
            (usable_height - cell_height * grid_size) / (grid_size - 1);
        int cell_x_spacing =
            (usable_width - cell_width * grid_size) / (grid_size + 1);

        /* We need to calculate the remainder width and then add a half of it
           to the starting point to make the grid centered in case the usable
           height doesn't divide evenly into grid_size. */
        int remainder_width = (usable_width - (grid_size + 1) * cell_x_spacing -
                               grid_size * cell_width);

        /* We offset the grid downwards to allow it to overlap with the gap
           between the two bottom corners and save space for the score at the
           top of the grid. */
        int corner_offset = corner_radius / 4;

        int grid_start_x =
            SCREEN_BORDER_WIDTH + cell_x_spacing + remainder_width / 2;
        int grid_start_y = SCREEN_BORDER_WIDTH + corner_radius + corner_offset;

        // We first draw a slot for the score
        int score_cell_width =
            width - 2 * (SCREEN_BORDER_WIDTH + corner_radius);
        int score_cell_height = cell_height;

        int score_start_y =
            (grid_start_y - score_cell_height - SCREEN_BORDER_WIDTH) / 2 +
            SCREEN_BORDER_WIDTH;

        Point score_start = {.x = SCREEN_BORDER_WIDTH + corner_radius,
                             .y = score_start_y};

        int score_title_x = score_start.x + cell_x_spacing;

        int score_title_y = score_start.y + (score_cell_height - FONT_SIZE) / 2;

        return new GridDimensions(cell_height, cell_width, cell_x_spacing,
                                  cell_y_spacing, remainder_width, grid_start_x,
                                  grid_start_y, score_cell_height,
                                  score_cell_width, score_start.x,
                                  score_start.y, score_title_x, score_title_y);
}

static void draw_game_grid(Display *display, int grid_size)
{

        GridDimensions *gd = calculate_grid_dimensions(display, grid_size);
        LOG_DEBUG(TAG, "Calculated grid dimensions.");

        Point score_start = {.x = gd->score_start_x, .y = gd->score_start_y};
        display->draw_rounded_rectangle(
            score_start, gd->score_cell_width, gd->score_cell_height,
            gd->score_cell_height / 2, GRID_BG_COLOR);

        const char *buffer = "Score:";

        Point score_title = {.x = gd->score_title_x, .y = gd->score_title_y};
        display->draw_string(score_title, (char *)buffer, Size16, GRID_BG_COLOR,
                             Black);

        int cell_width_and_spacing = gd->cell_width + gd->cell_x_spacing;
        int cell_height_and_spacing = gd->cell_height + gd->cell_y_spacing;
        int cell_radius = gd->cell_height / 2;

        for (int i = 0; i < grid_size; i++) {
                for (int j = 0; j < grid_size; j++) {
                        Point start = {
                            .x = gd->grid_start_x + j * cell_width_and_spacing,
                            .y =
                                gd->grid_start_y + i * cell_height_and_spacing};

                        display->draw_rounded_rectangle(
                            start, gd->cell_width, gd->cell_height, cell_radius,
                            GRID_BG_COLOR);
                }
        }
}

static void str_replace(char *str, const char *oldWord, const char *newWord);
static int number_string_length(int number);

void update_game_grid(Display *display, GameState *gs)
{
        int grid_size = gs->grid_size;
        GridDimensions *gd = calculate_grid_dimensions(display, grid_size);

        int score_title_length = 6 * FONT_WIDTH;
        char score_buffer[20];
        sprintf(score_buffer, "%d", gs->score);

        int score_rounding_radius = gd->score_cell_height / 2;

        Point clear_start = {.x = gd->score_title_x + score_title_length +
                                  FONT_WIDTH,
                             .y = gd->score_title_y};
        Point clear_end = {.x = gd->score_start_x + gd->score_cell_width -
                                score_rounding_radius,
                           .y = gd->score_title_y + FONT_SIZE};

        display->clear_region(clear_start, clear_end, GRID_BG_COLOR);

        Point score_start = {.x = gd->score_title_x + score_title_length +
                                  FONT_WIDTH,
                             .y = gd->score_title_y};

        display->draw_string(score_start, score_buffer, Size16, GRID_BG_COLOR,
                             Black);

        // The maximum tile number in this version of 2048 is 4096, because of
        // this the maximum width of the cell text area is given below.
        int max_cell_text_width = 4 * FONT_WIDTH;

        for (int i = 0; i < grid_size; i++) {
                for (int j = 0; j < grid_size; j++) {
                        Point start = {
                            .x = gd->grid_start_x +
                                 j * (gd->cell_width + gd->cell_x_spacing),
                            .y = gd->grid_start_y +
                                 i * (gd->cell_height + gd->cell_y_spacing)};

                        if (gs->grid[i][j] != gs->old_grid[i][j]) {

                                char buffer[5];
                                sprintf(buffer, "%4d", gs->grid[i][j]);
                                str_replace(buffer, "   0", "    ");

                                // We need to center the four characters of text
                                // inside the cell.
                                int x_margin =
                                    (gd->cell_width - max_cell_text_width) / 2;
                                int y_margin =
                                    (gd->cell_height - FONT_SIZE) / 2;
                                int old_digit_len =
                                    number_string_length(gs->old_grid[i][j]);
                                int old_digit_text_width =
                                    old_digit_len * FONT_WIDTH;

                                // We clear the region where the old number was
                                // drawn, note that we need to be efficient, so
                                // instead of clearing all 4 possible
                                // characters, we only clear the characters of
                                // the actual number. So if the number is
                                // composed of two digits, we clear a region
                                // that spans two digits.
                                Point clear_start = {.x = start.x + x_margin +
                                                          max_cell_text_width -
                                                          old_digit_text_width,
                                                     .y = start.y + y_margin};
                                Point clear_end = {.x = start.x + x_margin +
                                                        max_cell_text_width,
                                                   .y = start.y + y_margin +
                                                        FONT_SIZE};
                                display->clear_region(clear_start, clear_end,
                                                      GRID_BG_COLOR);

                                Point start_with_margin = {
                                    .x = start.x + x_margin,
                                    .y = start.y + y_margin};
                                display->draw_string(start_with_margin, buffer,
                                                     Size16, GRID_BG_COLOR,
                                                     Black);
                                gs->old_grid[i][j] = gs->grid[i][j];
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

static void str_replace(char *str, const char *oldWord, const char *newWord)
{
        std::string wrapped_str(str);
        std::string to_replace(oldWord);
        std::string replacement(newWord);

        size_t pos = wrapped_str.find(to_replace);
        if (pos != std::string::npos) {
                wrapped_str.replace(pos, to_replace.length(), replacement);
        }
        std::strcpy(str, wrapped_str.c_str());
}

void draw_game_over(Display *display, GameState *state)
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

void draw_game_won(Display *display, GameState *state)
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
