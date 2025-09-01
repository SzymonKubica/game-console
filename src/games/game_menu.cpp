#include <stdlib.h>
#include "game_menu.hpp"
#include "../common/configuration.hpp"
#include "../common/logging.hpp"
#include "../common/platform/interface/color.hpp"
#include "2048.hpp"
#include "game_executor.hpp"
#include "minesweeper.hpp"
#include "settings.hpp"
#include "game_of_life.hpp"

#define TAG "game_menu"

GameMenuConfiguration DEFAULT_MENU_CONFIGURATION = {.game = GameOfLife,
                                                    .accent_color = DarkBlue};

const char *map_game_to_str(Game game);

GameMenuConfiguration *
load_initial_menu_configuration(PersistentStorage *storage)
{

        int storage_offset = get_settings_storage_offsets()[MainMenu];

        GameMenuConfiguration configuration = {.game = Unknown,
                                               .accent_color = DarkBlue};

        LOG_DEBUG(TAG,
                  "Trying to load initial settings from the persistent storage "
                  "at offset %d",
                  storage_offset);
        storage->get(storage_offset, configuration);

        GameMenuConfiguration *output = new GameMenuConfiguration();

        if (configuration.game == Unknown) {
                LOG_DEBUG(TAG,
                          "The storage does not contain a valid "
                          "game menu configuration, using default values.");
                memcpy(output, &DEFAULT_MENU_CONFIGURATION,
                       sizeof(GameMenuConfiguration));
                storage->put(storage_offset, DEFAULT_MENU_CONFIGURATION);

        } else {
                LOG_DEBUG(TAG, "Using configuration from persistent storage.");
                memcpy(output, &configuration, sizeof(GameMenuConfiguration));
        }

        LOG_DEBUG(TAG,
                  "Loaded menu configuration: game=%d, "
                  "accent_color=%d",
                  output->game, output->accent_color);

        return output;
}

Configuration *
assemble_menu_selection_configuration(GameMenuConfiguration *initial_config)
{

        Configuration *config = new Configuration();
        config->name = "Game Console";

        ConfigurationOption *game = new ConfigurationOption();
        game->name = "Game";
        auto available_games = {"Sweeper", "2048", "Life", "Settings"};
        populate_string_option_values(game, available_games);
        game->currently_selected = get_config_option_string_value_index(
            game, map_game_to_str(initial_config->game));

        ConfigurationOption *accent_color = new ConfigurationOption();
        accent_color->name = "Accent color";
        auto available_accent_colors = {Color::Red,     Color::Green,
                                        Color::Blue,    Color::DarkBlue,
                                        Color::Magenta, Color::Cyan};
        populate_color_option_values(accent_color, available_accent_colors);
        accent_color->currently_selected = get_config_option_value_index(
            accent_color, initial_config->accent_color);

        int options_num = 2;
        config->options_len = options_num;
        config->options = new ConfigurationOption *[options_num];
        config->options[0] = game;
        config->options[1] = accent_color;
        config->curr_selected_option = 0;
        config->confirmation_cell_text = "Next";
        return config;
}

void extract_game_config(GameMenuConfiguration *menu_configuration,
                         Configuration *config)
{
        ConfigurationOption game_option = *config->options[0];

        int curr_option_idx = game_option.currently_selected;
        menu_configuration->game = map_game_from_str(static_cast<const char **>(
            game_option.available_values)[curr_option_idx]);

        // Game target is the second config option above.
        ConfigurationOption accent_color = *config->options[1];

        int curr_accent_color_idx = accent_color.currently_selected;
        Color color = static_cast<Color *>(
            accent_color.available_values)[curr_accent_color_idx];
        menu_configuration->accent_color = color;
}

void select_game(Platform *p)
{
        GameMenuConfiguration configuration;

        collect_game_configuration(p, &configuration);

        GameCustomization customization = {.accent_color =
                                               configuration.accent_color};

        switch (configuration.game) {
        case Unknown:
        case Clean2048:
                (new class Clean2048())->enter_game_loop(p, &customization);
                break;
        case Minesweeper:
                (new class Minesweeper())->enter_game_loop(p, &customization);
                break;
        case GameOfLife:
                (new class GameOfLife())->enter_game_loop(p, &customization);
                break;
        case Settings:
                (new class Settings())->enter_game_loop(p, &customization);
                break;
        default:
                LOG_DEBUG(TAG, "Selected game: %d. Game not implemented yet.",
                          configuration.game);
                break;
        }
}

bool collect_game_configuration(Platform *p,
                                GameMenuConfiguration *configuration)
{

        GameMenuConfiguration *initial_config =
            load_initial_menu_configuration(p->persistent_storage);

        Configuration *config =
            assemble_menu_selection_configuration(initial_config);

        if (!collect_configuration(p, config, initial_config->accent_color,
                                   false))
                return false;
        extract_game_config(configuration, config);

        free_configuration(config);
        free(initial_config);
        return true;
}

Game map_game_from_str(const char *name)
{
        if (strcmp(name, "2048") == 0) {
                return Game::Clean2048;
        } else if (strcmp(name, "Sweeper") == 0) {
                // We need to use a shorter name here because of rendering
                // constraints (arduino font is wider and doesn't fit nicely)
                return Game::Minesweeper;
        } else if (strcmp(name, "Life") == 0) {
                // We need to use a shorter name here because of rendering
                // constraints (arduino font is wider and doesn't fit nicely)
                return Game::GameOfLife;
        } else if (strcmp(name, "Main") == 0) {
                return Game::MainMenu;
        } else if (strcmp(name, "Settings") == 0) {
                return Game::Settings;
        }
        return Game::Unknown;
}

const char *map_game_to_str(Game game)
{
        switch (game) {
        case MainMenu:
                return "Menu";
        case Clean2048:
                return "2048";
        case Minesweeper:
                return "Sweeper";
        case GameOfLife:
                return "Life";
        case Settings:
                return "Settings";
        default:
                return "Unknown";
        }
}
