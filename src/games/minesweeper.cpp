#include "../common/platform/interface/platform.hpp"
#include "game_executor.hpp"

#include "../common/configuration.hpp"
#include "../common/logging.hpp"
#include "../common/constants.hpp"

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

static void collect_game_configuration(Platform *p,
                                       MinesweeperConfiguration *game_config,
                                       GameCustomization *customization);
static MinesweeperGridDimensions *
calculate_grid_dimensions(int display_width, int display_height,
                          int display_rounded_corner_radius);
static void draw_game_canvas(Platform *p, MinesweeperGridDimensions *dimensions,
                             GameCustomization *customization);

void minesweeper_game_loop(Platform *p, GameCustomization *customization)
{
        LOG_DEBUG(TAG, "Entering Minesweeper game loop");
        MinesweeperConfiguration config;

        collect_game_configuration(p, &config, customization);

        MinesweeperGridDimensions *gd = calculate_grid_dimensions(
            p->display->get_width(), p->display->get_height(),
            p->display->get_display_corner_radius());

        draw_game_canvas(p, gd, customization);

        p->display->refresh();

        while (true) {
                p->delay_provider->delay_ms(INPUT_POLLING_DELAY);
                p->display->refresh();
        }
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
        // Grid size is the first config option in the game struct above.
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

        int border_width = 3;

        /* WARNING: the order of rendering the border first and the actual
           rectangle matters here. If we were to draw the border later, it would
           have overwritten the inside of the rectantle and made it black. */
        p->display->draw_rectangle({.x = x_margin, .y = y_margin}, actual_width,
                                   actual_height, Gray, border_width, false);

        /* We don't draw the individual rectangles to make rendering faster
           on the physical Arduino LCD display. */
        p->display->draw_rectangle({.x = x_margin, .y = y_margin}, actual_width,
                                   actual_height, customization->accent_color,
                                   0, true);
}
