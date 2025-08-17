#include "game_menu.hpp"
#include "../common/configuration.hpp"
#include "../common/logging.hpp"
#include "../common/platform/interface/color.hpp"
#include "settings.hpp"
#include "game_of_life.hpp"


Configuration *assemble_settings_menu_configuration();
void extract_menu_setting(Game *selected_game, Configuration *config);

void enter_settings_loop(Platform *platform, GameCustomization *customization)
{
        Configuration *config = assemble_settings_menu_configuration();

                enter_configuration_collection_loop(
                    platform, config, customization->accent_color);

        Game selected_game;
        extract_menu_setting(&selected_game, config);

        switch (selected_game) {
        case Unknown:
        case Clean2048:
                break;
        case Snake:
                break;
        case Minesweeper:
                break;
        case GameOfLife:
                GameOfLifeConfiguration config;
                collect_game_of_life_configuration(platform, &config,
                                                   customization);
                // TODO: change the offset
                platform->persistent_storage->put(0, config);
                break;
        case MainMenu:
                break;
        default:
                break;
        }
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
