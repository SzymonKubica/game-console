#include <cstring>

#include "../common/logging.hpp"
#include "../common/configuration.hpp"
#include "../common/constants.hpp"
#include "../common/maths_utils.hpp"
#include "game_executor.hpp"

#define TAG "game_of_life"
#define GAME_CELL_WIDTH 8

#define GAME_LOOP_DELAY 100

#ifdef EMULATOR
#define EXPLANATION_ABOVE_GRID_OFFEST 4
#else
#define EXPLANATION_ABOVE_GRID_OFFEST 0
#endif

// The number of evolutions that the user can go back in time. Because of memory
// constaints we allow for a smaller buffer on the target device.
#ifdef EMULATOR
#define REWIND_BUF_SIZE 20
#else
#define REWIND_BUF_SIZE 3
#endif

typedef struct GameOfLifeConfiguration {
        bool prepopulate_grid;
        // Simulation steps taken per second
        bool use_toroidal_array;
        int simulation_speed;
        // int rewind_buffer_size; TODO: reenable once we are able to source the
        // config
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

typedef enum SimulationMode {
        RUNNING = 0,
        PAUSED = 1,
        REWIND = 2,
} SimulationMode;

typedef struct EvolutionDiff {
        Point *position;
        GameOfLifeCell new_state;
} EvolutionDiff;

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
void draw_rewind_mode_indicator(Platform *p,
                                GameOfLifeGridDimensions *dimensions,
                                GameCustomization *customization);
void clear_rewind_mode_indicator(Platform *p,
                                 GameOfLifeGridDimensions *dimensions,
                                 GameCustomization *customization);
void draw_caret(Display *display, Point *grid_position,
                GameOfLifeGridDimensions *dimensions, Color caret_color);
void erase_caret(Display *display, Point *grid_position,
                 GameOfLifeGridDimensions *dimensions,
                 Color grid_background_color);
void draw_game_cell(Display *display, Point *grid_position,
                    GameOfLifeGridDimensions *dimensions, Color color);

std::vector<EvolutionDiff> *
take_simulation_step(std::vector<std::vector<GameOfLifeCell>> *grid,
                     bool use_toroidal_array);

void render_diffs(Display *display, std::vector<EvolutionDiff> *diffs,
                  GameOfLifeGridDimensions *dimensions);

void apply_diffs(std::vector<EvolutionDiff> *diffs,
                 std::vector<std::vector<GameOfLifeCell>> *grid);

void unrender_diffs(Display *display, std::vector<EvolutionDiff> *diffs,
                    GameOfLifeGridDimensions *dimensions);

void unapply_diffs(std::vector<EvolutionDiff> *diffs,
                   std::vector<std::vector<GameOfLifeCell>> *grid);

void free_diffs(std::vector<EvolutionDiff> *diffs);

void spawn_cells_randomly(Display *display,
                          std::vector<std::vector<GameOfLifeCell>> *grid,
                          GameOfLifeGridDimensions *dimensions);

void add_diffs_to_rewind_buffer(
    std::vector<std::vector<EvolutionDiff> *> *rewind_buffer,
    int *rewind_buf_idx, std::vector<EvolutionDiff> *diffs);

void handle_rewind(Direction dir,
                   std::vector<std::vector<EvolutionDiff> *> *rewind_buffer,
                   int latest_state_idx, int *rewind_buf_idx,
                   std::vector<std::vector<GameOfLifeCell>> *grid,
                   GameOfLifeGridDimensions *gd, Display *display);

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

        Point caret_pos = {.x = 0, .y = 0};
        draw_caret(p->display, &caret_pos, gd, customization->accent_color);

        std::vector<std::vector<GameOfLifeCell>> grid(
            rows, std::vector<GameOfLifeCell>(cols));

        /* A ring buffer of evolution diffs that are used to allow for going
           back in time. Note that each user input also counts as a simulation
           step so will be included in the rewind buffer. */
        std::vector<std::vector<EvolutionDiff> *> rewind_buffer(REWIND_BUF_SIZE,
                                                                nullptr);
        int rewind_buf_idx = -1;
        // Keeps track of the latest diff in the rewind buffer. Prevents us from
        // wrapping back to it.
        int rewind_initial_idx = -1;

        if (config.prepopulate_grid) {
                spawn_cells_randomly(p->display, &grid, gd);
        }

        int evolution_period =
            (1000 / config.simulation_speed) / GAME_LOOP_DELAY;
        int iteration = 0;

        bool exit_requested = false;
        SimulationMode mode = PAUSED;
        while (!exit_requested) {
                if (mode == RUNNING && iteration == evolution_period - 1) {
                        LOG_DEBUG(TAG, "Taking a simulation step");
                        std::vector<EvolutionDiff> *diffs =
                            take_simulation_step(&grid,
                                                 config.use_toroidal_array);

                        LOG_DEBUG(TAG, "Got %zu diffs", diffs->size());
                        LOG_DEBUG(TAG, "Diffs rendered successfully!");
                        render_diffs(p->display, diffs, gd);
                        add_diffs_to_rewind_buffer(&rewind_buffer,
                                                   &rewind_buf_idx, diffs);
                }
                Direction dir;
                Action act;
                GameOfLifeCell curr = grid[caret_pos.y][caret_pos.x];
                if (directional_input_registered(p->directional_controllers,
                                                 &dir)) {
                        // TODO: clean up control flow to remove this deeply
                        // nested logic.
                        if (mode == REWIND) {
                                handle_rewind(
                                    dir, &rewind_buffer, rewind_initial_idx,
                                    &rewind_buf_idx, &grid, gd, p->display);
                        } else {
                                if (curr == EMPTY) {
                                        erase_caret(p->display, &caret_pos, gd,
                                                    Black);
                                } else if (curr == ALIVE) {
                                        erase_caret(p->display, &caret_pos, gd,
                                                    White);
                                }

                                if (config.use_toroidal_array) {
                                        translate_toroidal_array(&caret_pos,
                                                                 dir, gd->rows,
                                                                 gd->cols);
                                } else {
                                        translate_within_bounds(&caret_pos, dir,
                                                                gd->rows,
                                                                gd->cols);
                                }
                                draw_caret(p->display, &caret_pos, gd,
                                           customization->accent_color);
                        }
                }
                if (action_input_registered(p->action_controllers, &act)) {
                        switch (act) {
                        case YELLOW:
                                if (mode == PAUSED) {
                                        mode = RUNNING;
                                        LOG_DEBUG(TAG, "Simulation running...");
                                } else if (mode == REWIND) {
                                        mode = PAUSED;
                                        LOG_DEBUG(
                                            TAG,
                                            "Simulation paused after rewind.");
                                        clear_rewind_mode_indicator(
                                            p, gd, customization);
                                } else {
                                        mode = PAUSED;
                                        LOG_DEBUG(TAG, "Simulation paused.");
                                }
                                p->delay_provider->delay_ms(
                                    MOVE_REGISTERED_DELAY);
                                break;
                        case RED:
                                exit_requested = true;
                                break;
                        case BLUE:
                                if (mode == REWIND) {
                                        mode = RUNNING;
                                        clear_rewind_mode_indicator(
                                            p, gd, customization);
                                        LOG_DEBUG(TAG, "Simulation running...");
                                } else if (rewind_buf_idx != -1) {
                                        // We can only rewind if the buffer has
                                        // at least one entry.
                                        mode = REWIND;
                                        draw_rewind_mode_indicator(
                                            p, gd, customization);
                                        // We need to record the latest index in
                                        // the rewind buffer.
                                        rewind_initial_idx = rewind_buf_idx;
                                        LOG_DEBUG(TAG, "Rewind mode enabled.");
                                }
                                break;
                        case GREEN:
                                Color new_cell_color;
                                if (curr == EMPTY) {
                                        grid[caret_pos.y][caret_pos.x] = ALIVE;
                                        new_cell_color = White;
                                } else if (curr == ALIVE) {
                                        grid[caret_pos.y][caret_pos.x] = EMPTY;
                                        new_cell_color = Black;
                                }
                                EvolutionDiff diff = EvolutionDiff{
                                    .position =
                                        new Point{caret_pos.x, caret_pos.y},
                                    .new_state = grid[caret_pos.y][caret_pos.x],
                                };
                                std::vector<EvolutionDiff> *diffs =
                                    new std::vector<EvolutionDiff>();
                                diffs->push_back(diff);
                                add_diffs_to_rewind_buffer(
                                    &rewind_buffer, &rewind_buf_idx, diffs);
                                draw_game_cell(p->display, &caret_pos, gd,
                                               new_cell_color);
                                // we need to redraw the caret as we have just
                                // drawn a cell by clearing the region
                                draw_caret(p->display, &caret_pos, gd,
                                           customization->accent_color);

                                p->delay_provider->delay_ms(
                                    MOVE_REGISTERED_DELAY);
                                break;
                        }
                }
                iteration += 1;
                iteration %= evolution_period;
                p->delay_provider->delay_ms(GAME_LOOP_DELAY);
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
        ConfigurationOption *spawn_randomly = new ConfigurationOption();
        spawn_randomly->name = "Spawn randomly";
        std::vector<const char *> yes_or_no = {"Yes", "No"};
        populate_string_option_values(spawn_randomly, yes_or_no);
        spawn_randomly->currently_selected = 1;

        ConfigurationOption *simulation_speed = new ConfigurationOption();
        simulation_speed->name = "Evolutions/second";
        std::vector<int> available_speeds = {1, 2, 4};
        populate_int_option_values(simulation_speed, available_speeds);
        simulation_speed->currently_selected = 1;

        ConfigurationOption *toroidal_array = new ConfigurationOption();
        toroidal_array->name = "Toroidal array";
        populate_string_option_values(toroidal_array, yes_or_no);
        toroidal_array->currently_selected = 0;

        /*
        TODO: find a way to scroll through the config options as we need more
        than 3 options to configure the game properly. ConfigurationOption
        *rewind_buffer_size = new ConfigurationOption();
        rewind_buffer_size->name = "Evolutions/second";
        std::vector<int> available_sizes = {1, 2, 4};
        populate_int_option_values(rewind_buffer_size, available_speeds);
        rewind_buffer_size->currently_selected = 0;
        */

        config->options_len = 3;
        config->options = new ConfigurationOption *[config->options_len];
        config->options[0] = spawn_randomly;
        config->options[1] = simulation_speed;
        config->options[2] = toroidal_array;
        config->curr_selected_option = 0;
        config->confirmation_cell_text = "Start Game";
        return config;
}

bool extract_yes_or_no_option(const char *value)
{
        if (strlen(value) == 3 && strncmp(value, "Yes", 3) == 0) {
                return true;
        }
        return false;
}

void extract_game_config(GameOfLifeConfiguration *game_config,
                         Configuration *config)
{

        ConfigurationOption prepopulate_grid = *config->options[0];
        int curr_choice_idx = prepopulate_grid.currently_selected;
        const char *choice = static_cast<const char **>(
            prepopulate_grid.available_values)[curr_choice_idx];
        game_config->prepopulate_grid = extract_yes_or_no_option(choice);

        ConfigurationOption simulation_speed = *config->options[1];
        int curr_speed_idx = simulation_speed.currently_selected;
        game_config->simulation_speed = static_cast<int *>(
            simulation_speed.available_values)[curr_speed_idx];

        ConfigurationOption use_toroidal_array = *config->options[2];
        int use_toroidal_array_choice_idx =
            use_toroidal_array.currently_selected;
        const char *toroidal_array_choice = static_cast<const char **>(
            use_toroidal_array.available_values)[use_toroidal_array_choice_idx];
        game_config->use_toroidal_array =
            extract_yes_or_no_option(toroidal_array_choice);
}

std::vector<EvolutionDiff> *
take_simulation_step(std::vector<std::vector<GameOfLifeCell>> *grid,
                     bool use_toroidal_array)
{
        std::vector<EvolutionDiff> *diffs = new std::vector<EvolutionDiff>();

        // This assumes that the grid is rectangular.
        int rows = grid->size();
        int cols = (*grid)[0].size();

        for (int y = 0; y < rows; y++) {
                for (int x = 0; x < cols; x++) {
                        LOG_VERBOSE(TAG,
                                    "Processing cell at (%d, %d) with state %d",
                                    x, y, (*grid)[y][x]);
                        int alive_nb = 0;
                        Point curr = {.x = x, .y = y};

                        std::vector<Point> neighbours;

                        if (use_toroidal_array) {
                                neighbours = get_neighbours_toroidal_array(
                                    &curr, rows, cols);
                        } else {
                                neighbours = get_neighbours_inside_grid(
                                    &curr, rows, cols);
                        }

                        for (Point nb : neighbours) {
                                if ((*grid)[nb.y][nb.x] == ALIVE) {
                                        alive_nb++;
                                }
                        }

                        GameOfLifeCell new_state = EMPTY;
                        GameOfLifeCell current_state = (*grid)[y][x];

                        if (current_state == ALIVE) {
                                // Underpopulation or overpopulation
                                if (alive_nb < 2 || alive_nb > 3) {
                                        new_state = EMPTY;
                                } else {
                                        // Lives on to the next generation as it
                                        // has exactly 2 or 3 neighbours.
                                        new_state = ALIVE;
                                }
                        } else if (alive_nb == 3) {
                                new_state = ALIVE; // Reproduction
                        }
                        if (new_state != current_state) {
                                EvolutionDiff diff = {.position =
                                                          new Point{x, y},
                                                      .new_state = new_state};
                                diffs->push_back(diff);
                        }
                        LOG_VERBOSE(TAG, "Diffs len %d", (int)diffs->size());
                }
        }
        apply_diffs(diffs, grid);
        return diffs;
}

void apply_diffs(std::vector<EvolutionDiff> *diffs,
                 std::vector<std::vector<GameOfLifeCell>> *grid)
{
        for (EvolutionDiff &diff : *diffs) {
                (*grid)[diff.position->y][diff.position->x] = diff.new_state;
        }
}

void render_diffs(Display *display, std::vector<EvolutionDiff> *diffs,
                  GameOfLifeGridDimensions *dimensions)
{

        for (EvolutionDiff &diff : *diffs) {
                Color color;
                if (diff.new_state == ALIVE) {
                        color = White; // Alive cells are rendered in white
                } else {
                        color = Black; // Dead cells are rendered in black
                }
                draw_game_cell(display, diff.position, dimensions, color);
        }
}

GameOfLifeCell flip_state(GameOfLifeCell state)
{
        switch (state) {
        case EMPTY:
                return ALIVE;
        case ALIVE:
                return EMPTY;
        }
        // Unreachable (assuming noone casts random integers as GameOfLifeCell).
        return EMPTY;
}

/**
 * Used for rewind, it flips the state of the diffs. This acts as if the action
 * of applying the diff on the grid was reversed.
 */
void unapply_diffs(std::vector<EvolutionDiff> *diffs,
                   std::vector<std::vector<GameOfLifeCell>> *grid)
{
        for (EvolutionDiff &diff : *diffs) {
                (*grid)[diff.position->y][diff.position->x] =
                    flip_state(diff.new_state);
        }
}

/**
 * Same as unapply but for rendering.
 */
void unrender_diffs(Display *display, std::vector<EvolutionDiff> *diffs,
                    GameOfLifeGridDimensions *dimensions)
{

        for (EvolutionDiff &diff : *diffs) {
                Color color;
                if (flip_state(diff.new_state) == ALIVE) {
                        color = White; // Alive cells are rendered in white
                } else {
                        color = Black; // Dead cells are rendered in black
                }
                draw_game_cell(display, diff.position, dimensions, color);
        }
}

void add_diffs_to_rewind_buffer(
    std::vector<std::vector<EvolutionDiff> *> *rewind_buffer,
    int *rewind_buf_idx, std::vector<EvolutionDiff> *diffs)
{

        // We need to increment the index and wrap it around as we are using
        // a ring buffer.
        *rewind_buf_idx = (*rewind_buf_idx + 1) % REWIND_BUF_SIZE;
        int idx = *rewind_buf_idx;
        LOG_DEBUG(TAG, "Current rewind buffer index is now %d",
                  *rewind_buf_idx);
        LOG_DEBUG(TAG, "Adding diffs to rewind buffer at index %d",
                  *rewind_buf_idx);
        if ((*rewind_buffer)[idx] != nullptr) {
                LOG_DEBUG(
                    TAG,
                    "Rewind buffer already has diffs at index %d, freeing them",
                    *rewind_buf_idx);
                free_diffs((*rewind_buffer)[idx]);
        }
        (*rewind_buffer)[idx] = diffs;
}

void handle_rewind(Direction dir,
                   std::vector<std::vector<EvolutionDiff> *> *rewind_buffer,
                   int latest_state_idx, int *rewind_buf_idx,
                   std::vector<std::vector<GameOfLifeCell>> *grid,
                   GameOfLifeGridDimensions *gd, Display *display)
{
        // Ignore irrelevant input.
        if (dir == UP || dir == DOWN) {
                return;
        }

        // First we short-circuit if the user tries to go back too far.
        bool forward_in_time = dir == RIGHT;
        bool back_in_time = dir == LEFT;
        // Rewind cannot go back in time past the REWIND_BUF_SIZE as it would
        // wrap around to the latest state.
        if (back_in_time &&
            *rewind_buf_idx == (latest_state_idx + 1) % REWIND_BUF_SIZE) {
                return;
        }

        if (forward_in_time) {
                auto diffs = (*rewind_buffer)[*rewind_buf_idx];
                apply_diffs(diffs, grid);
                render_diffs(display, diffs, gd);

                // Rewind cannot go into the future.
                if (*rewind_buf_idx == latest_state_idx) {
                        return;
                }
                *rewind_buf_idx = (*rewind_buf_idx + 1) % REWIND_BUF_SIZE;
        } else if (back_in_time) {
                auto diffs = (*rewind_buffer)[*rewind_buf_idx];

                unapply_diffs(diffs, grid);
                unrender_diffs(display, diffs, gd);
                // We need to use proper modulo as % is weird with
                // negative numbers.
                *rewind_buf_idx =
                    mathematical_modulo((*rewind_buf_idx - 1), REWIND_BUF_SIZE);

                /* If the rewind ring buffer has not been fully populated yet,
                   trying to rewind back in time would wrap around to the end of
                   the array and try to render nullptr diffs (as the indices
                   larger than the latest_state_idx haven't been initialized
                   yet). We need to short-circuit if that happens. */
                auto next_diffs = (*rewind_buffer)[*rewind_buf_idx];
                if (next_diffs == nullptr) {
                        LOG_DEBUG(TAG,
                                  "Rewind buffer is empty at index %d, "
                                  "skipping index update",
                                  *rewind_buf_idx);
                        // We need to increment the index to go back to safety
                        *rewind_buf_idx =
                            (*rewind_buf_idx + 1) % REWIND_BUF_SIZE;
                        return;
                }
        }
}

void free_diffs(std::vector<EvolutionDiff> *diffs)
{
        for (auto diff : *diffs) {
                delete diff.position;
        }
        diffs->clear();
        delete diffs;
}

void spawn_cells_randomly(Display *display,
                          std::vector<std::vector<GameOfLifeCell>> *grid,
                          GameOfLifeGridDimensions *dimensions)
{
        for (int y = 0; y < dimensions->rows; y++) {
                for (int x = 0; x < dimensions->cols; x++) {
                        if (rand() % 2 == 0) {
                                (*grid)[y][x] = ALIVE;
                                Point position = {.x = x, .y = y};
                                draw_game_cell(display, &position, dimensions,
                                               White);
                        }
                }
        }
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

        int x_margin = dimensions->left_horizontal_margin;
        int y_margin = dimensions->top_vertical_margin;

        int actual_width = dimensions->actual_width;
        int actual_height = dimensions->actual_height;

        int border_width = 1;
        // We need to make the border rectangle and the canvas slightly
        // bigger to ensure that it does not overlap with the game area.
        // Otherwise the caret rendering erases parts of the border as
        // it moves around (as the caret intersects with the border
        // partially)
        int border_offset = 2;

        /* Rendering of help indicators below the grid */
        int text_below_grid_y = y_margin + actual_height + 1 * border_offset;
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
        // Because of slightly different font dimensions, we need this offset
        // override to ensure proper vertical space above the game grid.
        int text_above_grid_y = y_margin - border_offset - FONT_SIZE -
                                EXPLANATION_ABOVE_GRID_OFFEST;
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

        // We draw the border at the end to ensure that it doesn't get cropped
        // by draw string operations above.
        p->display->draw_rectangle(
            {.x = x_margin - border_offset, .y = y_margin - border_offset},
            actual_width + 2 * border_offset, actual_height + 2 * border_offset,
            customization->accent_color, border_width, false);
}

void draw_rewind_mode_indicator(Platform *p,
                                GameOfLifeGridDimensions *dimensions,
                                GameCustomization *customization)
{

        int x_margin = dimensions->left_horizontal_margin;
        int y_margin = dimensions->top_vertical_margin;
        int actual_width = dimensions->actual_width;
        int actual_height = dimensions->actual_height;
        int border_width = 1;
        // We need to make the border rectangle and the canvas slightly
        // bigger to ensure that it does not overlap with the game area.
        // Otherwise the caret rendering erases parts of the border as
        // it moves around (as the caret intersects with the border
        // partially)
        int border_offset = 2;

        Color indicator_border_color;

        /* The indicator border is supposed to resmble the blue color of the
           button that toggles the rewind mode. However, if the accent color is
           blue, we need to use a different color for the border to make it
           visible against the background. In this case, we use cyan as the
           border color. */

        if (customization->accent_color == DarkBlue) {
                indicator_border_color = Cyan;
        } else {
                indicator_border_color = DarkBlue;
        }

        p->display->draw_rectangle(
            {.x = x_margin - border_offset, .y = y_margin - border_offset},
            actual_width + 2 * border_offset, actual_height + 2 * border_offset,
            indicator_border_color, border_width, false);
}

void clear_rewind_mode_indicator(Platform *p,
                                 GameOfLifeGridDimensions *dimensions,
                                 GameCustomization *customization)
{

        int x_margin = dimensions->left_horizontal_margin;
        int y_margin = dimensions->top_vertical_margin;
        int actual_width = dimensions->actual_width;
        int actual_height = dimensions->actual_height;
        int border_width = 1;
        // We need to make the border rectangle and the canvas slightly
        // bigger to ensure that it does not overlap with the game area.
        // Otherwise the caret rendering erases parts of the border as
        // it moves around (as the caret intersects with the border
        // partially)
        int border_offset = 2;

        Color indicator_border_color;

        p->display->draw_rectangle(
            {.x = x_margin - border_offset, .y = y_margin - border_offset},
            actual_width + 2 * border_offset, actual_height + 2 * border_offset,
            customization->accent_color, border_width, false);
}
