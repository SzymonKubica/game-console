#include "2048.hpp"
#include "game_menu.hpp"
#include "../common/configuration.hpp"
#include "../common/logging.hpp"
#include "settings.hpp"
#include "game_of_life.hpp"
#include "minesweeper.hpp"

#define TAG "settings"

Configuration *assemble_settings_menu_configuration();
void extract_menu_setting(Game *selected_game, Configuration *config);

void enter_settings_loop(Platform *platform, GameCustomization *customization)
{
        Configuration *config = assemble_settings_menu_configuration();

        enter_configuration_collection_loop(platform, config,
                                            customization->accent_color);

        Game selected_game;
        extract_menu_setting(&selected_game, config);

        std::vector<int> offsets = get_settings_storage_offsets();

        int offset = offsets[selected_game];
        LOG_DEBUG(TAG, "Computed configuration storage offset for game %s: %d",
                  map_game_to_str(selected_game), offset)

        switch (selected_game) {
        case MainMenu: {
                GameMenuConfiguration config;
                collect_game_configuration(platform, &config);
                platform->persistent_storage->put(offset, config);
        } break;
        case Clean2048: {
                Game2048Configuration config;
                collect_2048_configuration(platform, &config, customization);
                platform->persistent_storage->put(offset, config);
        } break;
        case Minesweeper: {
                MinesweeperConfiguration config;
                collect_minesweeper_configuration(platform, &config,
                                                  customization);
                platform->persistent_storage->put(offset, config);
        } break;
        case GameOfLife: {
                GameOfLifeConfiguration config;
                collect_game_of_life_configuration(platform, &config,
                                                   customization);
                platform->persistent_storage->put(offset, config);
        } break;
        default:
                break;
        }
}

std::vector<int> get_settings_storage_offsets()
{
        std::vector<int> offsets(7);
        offsets[MainMenu] = 0;
        offsets[Clean2048] =
            offsets[MainMenu] + sizeof(GameMenuConfiguration);
        offsets[Minesweeper] =
            offsets[Clean2048] + sizeof(Game2048Configuration);
        offsets[GameOfLife] =
            offsets[Minesweeper] + sizeof(MinesweeperConfiguration);
        offsets[RandomSeedPicker] =
            offsets[GameOfLife] + sizeof(GameOfLifeConfiguration);
        return offsets;
}
Configuration *assemble_settings_menu_configuration()
{
        Configuration *config = new Configuration();
        config->name = "Set Defaults";

        ConfigurationOption *menu = new ConfigurationOption();
        menu->name = "Menu";
        auto available_games = {"Main", "Sweeper", "2048", "Life"};
        populate_string_option_values(menu, available_games);
        menu->currently_selected = 0;

        int options_num = 1;
        config->options_len = options_num;
        config->options = new ConfigurationOption *[options_num];
        config->options[0] = menu;
        config->curr_selected_option = 0;
        config->confirmation_cell_text = "Next";
        return config;
}

void extract_menu_setting(Game *selected_menu, Configuration *config)
{
        ConfigurationOption menu_option = *config->options[0];

        int curr_idx = menu_option.currently_selected;
        *selected_menu = map_game_from_str(
            static_cast<const char **>(menu_option.available_values)[curr_idx]);
}
