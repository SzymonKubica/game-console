#include "../common/platform/interface/platform.hpp"
#include "game_executor.hpp"

#include "../common/configuration.hpp"
#include "../common/logging.hpp"

#define TAG "minesweeper"

typedef struct MinesweeperConfiguration {
        int mines_num;
} MinesweeperConfiguration;

void collect_game_configuration(Platform *p,
                                MinesweeperConfiguration *game_config,
                                GameCustomization *customization);

void enter_minesweeper_loop(Platform *platform,
                            GameCustomization *customization)
{
        LOG_DEBUG(TAG, "Entering Minesweeper game loop");
        MinesweeperConfiguration config;

        collect_game_configuration(platform, &config, customization);
}

Configuration *assemble_minesweeper_configuration();
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
}

Configuration *assemble_minesweeper_configuration()
{
        Configuration *config =
            static_cast<Configuration *>(malloc(sizeof(Configuration)));

        config->name = "Minesweeper";

        // Initialize the first config option: game gridsize
        ConfigurationOption *mines_count = static_cast<ConfigurationOption *>(
            malloc(2 * sizeof(ConfigurationOption)));
        mines_count->name = "Number of mines";
        std::vector<int> available_values = {10, 15, 25};
        populate_int_option_values(mines_count, available_values);
        mines_count->currently_selected = 1;

        config->options_len = 1;
        config->curr_selected_option = 0;
        config->options = static_cast<ConfigurationOption **>(
            malloc(config->options_len * sizeof(ConfigurationOption *)));
        config->options[0] = mines_count;
        config->confirmation_cell_text = "Start Game";
        return config;
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
