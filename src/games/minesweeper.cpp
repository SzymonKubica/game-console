#include "../common/platform/interface/platform.hpp"
#include "game_executor.hpp"

#include "../common/configuration.hpp"
#include "../common/logging.hpp"
#include "../common/constants.hpp"

#include "common_transitions.hpp"

#define TAG "minesweeper"

typedef struct MinesweeperConfiguration {
        int mines_num;
} MinesweeperConfiguration;

typedef struct MinesweeperGridDimensions {
        int rows;
        int cols;
        int top_vertical_margin;
        int left_horizontal_margin;
        int actual_width;
        int actual_height;

        MinesweeperGridDimensions(int r, int c, int tvm, int lhm, int aw,
                                  int ah)
            : rows(r), cols(c), top_vertical_margin(tvm),
              left_horizontal_margin(lhm), actual_width(aw), actual_height(ah)
        {
        }
} MinesweeperGridDimensions;

typedef struct MinesweeperGridCell {
        bool is_bomb;
        bool is_flagged;
        bool is_uncovered;
        int adjacent_bombs;

        MinesweeperGridCell()
            : is_bomb(false), is_flagged(false), is_uncovered(false),
              adjacent_bombs(0)
        {
        }

} MinesweeperGridCell;

static void collect_game_configuration(Platform *p,
                                       MinesweeperConfiguration *game_config,
                                       GameCustomization *customization);
static MinesweeperGridDimensions *
calculate_grid_dimensions(int display_width, int display_height,
                          int display_rounded_corner_radius);
static void draw_game_canvas(Platform *p, MinesweeperGridDimensions *dimensions,
                             GameCustomization *customization);

static void erase_caret(Display *display, Point *grid_position,
                        MinesweeperGridDimensions *dimensions,
                        Color grid_background_color);
static void draw_caret(Display *display, Point *grid_position,
                       MinesweeperGridDimensions *dimensions);
/**
 * Performs a recursive uncovering waterfall: tries to uncover the current
 * cell, if the cell has 0 adjacent mines it uncovers all of its neighbours.
 */
static void uncover_grid_cells_starting_from(
    Display *display, Point *grid_position,
    MinesweeperGridDimensions *dimensions,
    std::vector<std::vector<MinesweeperGridCell>> *grid, int *total_uncovered);
static void
uncover_grid_cell(Display *display, Point *grid_position,
                  MinesweeperGridDimensions *dimensions,
                  std::vector<std::vector<MinesweeperGridCell>> *grid,
                  int *total_uncovered);
static void flag_grid_cell(Display *display, Point *grid_position,
                           MinesweeperGridDimensions *dimensions,
                           std::vector<std::vector<MinesweeperGridCell>> *grid);
static void
unflag_grid_cell(Display *display, Point *grid_position,
                 MinesweeperGridDimensions *dimensions,
                 std::vector<std::vector<MinesweeperGridCell>> *grid,
                 Color grid_background_color);

static void place_bombs(std::vector<std::vector<MinesweeperGridCell>> *grid,
                        int bomb_number);
void enter_minesweeper_loop(Platform *p, GameCustomization *customization)
{
        LOG_DEBUG(TAG, "Entering Minesweeper game loop");
        MinesweeperConfiguration config;

        collect_game_configuration(p, &config, customization);

        MinesweeperGridDimensions *gd = calculate_grid_dimensions(
            p->display->get_width(), p->display->get_height(),
            p->display->get_display_corner_radius());
        int rows = gd->rows;
        int cols = gd->cols;

        draw_game_canvas(p, gd, customization);
        LOG_DEBUG(TAG, "Minesweeper game canvas drawn.");

        p->display->refresh();

        std::vector<std::vector<MinesweeperGridCell>> grid(
            rows, std::vector<MinesweeperGridCell>(cols));

        // place_bombs(&grid, config.mines_num);

        Point caret_position = {.x = 0, .y = 0};
        draw_caret(p->display, &caret_position, gd);

        int total_uncovered = 0;

        bool is_game_over = false;
        while (!is_game_over &&
               !(total_uncovered == cols * rows - config.mines_num)) {
                Direction dir;
                Action act;
                if (directional_input_registered(p->directional_controllers,
                                                 &dir)) {
                        LOG_DEBUG(TAG, "Directional input received: %s",
                                  direction_to_str(dir));

                        /* Once the cells become uncovered, the background is
                        set to black. Because of this, we need to change the
                        erase color */
                        if (grid[caret_position.y][caret_position.x]
                                .is_uncovered) {
                                erase_caret(p->display, &caret_position, gd,
                                            Black);
                                // We need to 'uncover' the cell again to ensure
                                // that the numbers don't get cropped after the
                                // caret overlaps with them.
                                uncover_grid_cell(p->display, &caret_position,
                                                  gd, &grid, &total_uncovered);
                        } else {
                                erase_caret(p->display, &caret_position, gd,
                                            customization->accent_color);
                        }
                        translate_within_bounds(&caret_position, dir, gd->rows,
                                                gd->cols);
                        draw_caret(p->display, &caret_position, gd);

                        p->delay_provider->delay_ms(MOVE_REGISTERED_DELAY);
                        /* We continue here to skip the additional input polling
                           delay at the end of the loop and make the input
                           snappy. */
                        continue;
                }
                if (action_input_registered(p->action_controllers, &act)) {
                        LOG_DEBUG(TAG, "Action input received: %s",
                                  action_to_str(act));

                        MinesweeperGridCell cell =
                            grid[caret_position.y][caret_position.x];
                        switch (act) {
                        case Action::RED:
                                if (!cell.is_uncovered) {
                                        if (!cell.is_flagged) {
                                                flag_grid_cell(p->display,
                                                               &caret_position,
                                                               gd, &grid);

                                        } else {
                                                unflag_grid_cell(
                                                    p->display, &caret_position,
                                                    gd, &grid,
                                                    customization
                                                        ->accent_color);
                                                draw_caret(p->display,
                                                           &caret_position, gd);
                                        }
                                }
                                break;
                        case Action::GREEN:
                                if (cell.is_bomb) {
                                        is_game_over = true;
                                }
                                if (!cell.is_flagged) {
                                        uncover_grid_cells_starting_from(
                                            p->display, &caret_position, gd,
                                            &grid, &total_uncovered);
                                }
                                break;
                        default:
                                LOG_DEBUG(TAG, "Irrelevant action input: %s",
                                          action_to_str(act));
                                break;
                        }
                        p->delay_provider->delay_ms(MOVE_REGISTERED_DELAY);
                        /* We continue here to skip the additional input polling
                           delay at the end of the loop and make the input
                           snappy. */
                        continue;
                }
                p->delay_provider->delay_ms(INPUT_POLLING_DELAY);
        }

        // When the game is lost, we make all bombs explode.
        if (is_game_over) {
                for (int y = 0; y < rows; y++) {
                        for (int x = 0; x < cols; x++) {
                                MinesweeperGridCell cell = grid[y][x];
                                if (cell.is_bomb) {
                                        Point point = {.x = x, .y = y};
                                        uncover_grid_cell(p->display, &point,
                                                          gd, &grid,
                                                          &total_uncovered);
                                }
                        }
                }
                pause_until_input(p->directional_controllers,
                                  p->delay_provider);
                draw_game_over(p->display);
                p->delay_provider->delay_ms(MOVE_REGISTERED_DELAY);
        } else {
                pause_until_input(p->directional_controllers,
                                  p->delay_provider);
                draw_game_won(p->display);
                p->delay_provider->delay_ms(MOVE_REGISTERED_DELAY);
        }
        pause_until_input(p->directional_controllers, p->delay_provider);
}

void place_bombs(std::vector<std::vector<MinesweeperGridCell>> *grid,
                 int bomb_number)
{
        int rows = grid->size();
        int cols = (*grid->begin().base()).size();
        for (int i = 0; i < bomb_number; i++) {
                while (true) {
                        int x = rand() % cols;
                        int y = rand() % rows;

                        if (!(*grid)[y][x].is_bomb) {
                                (*grid)[y][x].is_bomb = true;
                                (*grid)[y][x].adjacent_bombs = 0;

                                Point current = {.x = x, .y = y};
                                for (Point nb : *get_neighbours_inside_grid(
                                         &current, rows, cols)) {
                                        (*grid)[nb.y][nb.x].adjacent_bombs++;
                                }

                                break;
                        }
                }
        }
}

void erase_caret(Display *display, Point *grid_position,
                 MinesweeperGridDimensions *dimensions,
                 Color grid_background_color)
{
        // We need to ensure that the caret is rendered INSIDE the text cell
        // and its border doesn't overlap the neighbouring cells. Otherwise,
        // we'll get weird rendering artifacts.
        int border_offset = 1;
        Point actual_position = {
            .x = dimensions->left_horizontal_margin +
                 grid_position->x * FONT_WIDTH + border_offset,
            .y = dimensions->top_vertical_margin +
                 grid_position->y * FONT_SIZE + border_offset};

        display->draw_rectangle(actual_position, FONT_WIDTH - 2 * border_offset,
                                FONT_SIZE - 2 * border_offset,
                                grid_background_color, 1, false);
}

void draw_caret(Display *display, Point *grid_position,
                MinesweeperGridDimensions *dimensions)
{

        // We need to ensure that the caret is rendered INSIDE the text cell
        // and its border doesn't overlap the neighbouring cells. Otherwise,
        // we'll get weird rendering artifacts.
        int border_offset = 1;
        Point actual_position = {
            .x = dimensions->left_horizontal_margin +
                 grid_position->x * FONT_WIDTH + border_offset,
            .y = dimensions->top_vertical_margin +
                 grid_position->y * FONT_SIZE + border_offset};

        display->draw_rectangle(actual_position, FONT_WIDTH - 2 * border_offset,
                                FONT_SIZE - 2 * border_offset, White, 1, false);
}
void uncover_grid_cell(Display *display, Point *grid_position,
                       MinesweeperGridDimensions *dimensions,
                       std::vector<std::vector<MinesweeperGridCell>> *grid,
                       int *total_uncovered)
{

        Point actual_position = {.x = dimensions->left_horizontal_margin +
                                      grid_position->x * FONT_WIDTH,
                                 .y = dimensions->top_vertical_margin +
                                      grid_position->y * FONT_SIZE};

        char text[2];

        MinesweeperGridCell cell = (*grid)[grid_position->y][grid_position->x];
        // We need this check as we 're-uncover' cells after the caret passes
        // over them to remove rendering overlap artifacts.
        if (!cell.is_uncovered) {
                (*total_uncovered)++;
                (*grid)[grid_position->y][grid_position->x].is_uncovered = true;
        }
        Color text_color = White;
        if (cell.is_bomb) {
                sprintf(text, "*");
        } else if (cell.adjacent_bombs == 0) {
                sprintf(text, " ");
        } else {
                sprintf(text, "%d", cell.adjacent_bombs);
                /* We override the rendering color depending on the number of
                   bombs around the cell to make it easier to read the UI. */
                switch (cell.adjacent_bombs) {
                case 1:
                        text_color = Cyan;
                        break;
                case 2:
                        text_color = Green;
                        break;
                case 3:
                        text_color = Red;
                        break;
                case 4:
                        text_color = BRed;
                        break;
                }
        }
        display->draw_rectangle(actual_position, FONT_WIDTH, FONT_SIZE, Black,
                                0, true);

        display->draw_string(actual_position, text, FontSize::Size16, Black,
                             text_color);
}

void uncover_grid_cells_starting_from(
    Display *display, Point *grid_position,
    MinesweeperGridDimensions *dimensions,
    std::vector<std::vector<MinesweeperGridCell>> *grid, int *total_uncovered)
{

        uncover_grid_cell(display, grid_position, dimensions, grid,
                          total_uncovered);

        int rows = grid->size();
        int cols = (*grid->begin().base()).size();
        MinesweeperGridCell current_cell =
            (*grid)[grid_position->y][grid_position->x];

        if (!current_cell.is_bomb && current_cell.adjacent_bombs == 0) {
                for (Point nb :
                     *get_neighbours_inside_grid(grid_position, rows, cols)) {
                        MinesweeperGridCell neighbour_cell =
                            (*grid)[nb.y][nb.x];

                        if (!neighbour_cell.is_uncovered &&
                            !neighbour_cell.is_flagged) {
                                uncover_grid_cells_starting_from(
                                    display, &nb, dimensions, grid,
                                    total_uncovered);
                        }
                }
        }
}

void flag_grid_cell(Display *display, Point *grid_position,
                    MinesweeperGridDimensions *dimensions,
                    std::vector<std::vector<MinesweeperGridCell>> *grid)
{

        (*grid)[grid_position->y][grid_position->x].is_flagged = true;
        Point actual_position = {.x = dimensions->left_horizontal_margin +
                                      grid_position->x * FONT_WIDTH,
                                 .y = dimensions->top_vertical_margin +
                                      grid_position->y * FONT_SIZE};

        char text[2];
        sprintf(text, "f");
        display->draw_string(actual_position, text, FontSize::Size16, Black,
                             White);
}

void unflag_grid_cell(Display *display, Point *grid_position,
                      MinesweeperGridDimensions *dimensions,
                      std::vector<std::vector<MinesweeperGridCell>> *grid,
                      Color grid_background_color)
{

        (*grid)[grid_position->y][grid_position->x].is_flagged = false;
        Point actual_position = {.x = dimensions->left_horizontal_margin +
                                      grid_position->x * FONT_WIDTH,
                                 .y = dimensions->top_vertical_margin +
                                      grid_position->y * FONT_SIZE};

        display->draw_rectangle(actual_position, FONT_WIDTH, FONT_SIZE,
                                grid_background_color, 0, true);
}

Configuration *assemble_minesweeper_configuration();
void free_minesweeper_configuration(Configuration *config);
void extract_game_config(MinesweeperConfiguration *game_config,
                         Configuration *config);

void collect_game_configuration(Platform *p,
                                MinesweeperConfiguration *game_config,
                                GameCustomization *customization)
{
        Configuration *config = assemble_minesweeper_configuration();
        enter_configuration_collection_loop(p, config,
                                            customization->accent_color);
        extract_game_config(game_config, config);
        free_minesweeper_configuration(config);
}

Configuration *assemble_minesweeper_configuration()
{
        Configuration *config = new Configuration();
        config->name = "Minesweeper";

        // Initialize the first config option: game gridsize
        ConfigurationOption *mines_count = new ConfigurationOption();
        mines_count->name = "Number of mines";
        std::vector<int> available_values = {10, 15, 25};
        populate_int_option_values(mines_count, available_values);
        mines_count->currently_selected = 1;

        config->options_len = 1;
        config->options = new ConfigurationOption *[config->options_len];
        config->options[0] = mines_count;
        config->curr_selected_option = 0;
        config->confirmation_cell_text = "Start Game";
        return config;
}

void free_minesweeper_configuration(Configuration *config)
{
        for (int i = 0; i < config->options_len; i++) {
                ConfigurationOption *option = config->options[i];
                free(option->available_values);
                delete config->options[i];
        }
        delete config;
}

void extract_game_config(MinesweeperConfiguration *game_config,
                         Configuration *config)
{
        // Grid size is the first config option in the game struct
        // above.
        ConfigurationOption mines_num = *config->options[0];

        int curr_mines_count_idx = mines_num.currently_selected;
        game_config->mines_num = static_cast<int *>(
            mines_num.available_values)[curr_mines_count_idx];
}

MinesweeperGridDimensions *
calculate_grid_dimensions(int display_width, int display_height,
                          int display_rounded_corner_radius)
{
        // Bind input params to short names for improved readability.
        int w = display_width;
        int h = display_height;
        int r = display_rounded_corner_radius;

        int usable_width = w - r;
        int usable_height = h - r;

        int max_cols = usable_width / FONT_WIDTH;
        int max_rows = usable_height / FONT_SIZE;

        int actual_width = max_cols * FONT_WIDTH;
        int actual_height = max_rows * FONT_SIZE;

        // We calculate centering margins
        int left_horizontal_margin = (w - actual_width) / 2;
        int top_vertical_margin = (h - actual_height) / 2;

        LOG_DEBUG(TAG,
                  "Calculated grid dimensions: %d rows, %d cols, "
                  "left margin: %d, top margin: %d, actual width: %d, "
                  "actual height: %d",
                  max_rows, max_cols, left_horizontal_margin,
                  top_vertical_margin, actual_width, actual_height);

        return new MinesweeperGridDimensions(
            max_rows, max_cols, top_vertical_margin, left_horizontal_margin,
            actual_width, actual_height);
}

void draw_game_canvas(Platform *p, MinesweeperGridDimensions *dimensions,
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
            Gray, border_width, false);

        /* We don't draw the individual rectangles to make rendering
           faster on the physical Arduino LCD display. */
        p->display->draw_rectangle(
            {.x = x_margin - border_offset, .y = y_margin - border_offset},
            actual_width + 2 * border_offset, actual_height + 2 * border_offset,
            customization->accent_color, 0, true);
}
