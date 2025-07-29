#include "../common/platform/interface/platform.hpp"
#include "game_executor.hpp"

#include "../common/configuration.hpp"
#include "../common/logging.hpp"
#include "../common/constants.hpp"

#define TAG "minesweeper"

typedef struct MinesweeperConfiguration {
        int mines_num;
} MinesweeperConfiguration;

void collect_game_configuration(Platform *p,
                                MinesweeperConfiguration *game_config,
                                GameCustomization *customization);
void draw_game_canvas(Platform *p, MinesweeperConfiguration *config,
                      GameCustomization *customization);

void enter_minesweeper_loop(Platform *platform,
                            GameCustomization *customization)
{
        LOG_DEBUG(TAG, "Entering Minesweeper game loop");
        MinesweeperConfiguration config;

        collect_game_configuration(platform, &config, customization);
        draw_game_canvas(platform, &config, customization);
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

void draw_game_canvas(Platform *p, MinesweeperConfiguration *config,
                      GameCustomization *customization)

{
        p->display->initialize();
        p->display->clear(Black);
        p->display->draw_rounded_border(customization->accent_color);
        int max_cols = p->display->get_width() / FONT_WIDTH;
        int max_rows = p->display->get_height() / FONT_SIZE;

        char canvas[max_rows][max_cols];

        for (int i = 0; i < max_rows; i++) {
                for (int j = 0; j < max_cols; j++) {
                        p->display->draw_rounded_rectangle(
                            {.x = j * FONT_WIDTH, .y = i * FONT_SIZE},
                            FONT_WIDTH, FONT_SIZE, 2, DarkBlue);
                }
        }
        while (true) {}
}
