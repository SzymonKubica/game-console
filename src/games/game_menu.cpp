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

const char *game_to_string(Game game);

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

        auto *game = ConfigurationOption::of_strings(
            "Game",
            {game_to_string(Game::Minesweeper), game_to_string(Game::Clean2048),
             game_to_string(Game::GameOfLife), game_to_string(Game::Settings)},
            game_to_string(initial_config->game));

        auto available_colors = {
            Color::Red,      Color::Green,      Color::Blue,  Color::DarkBlue,
            Color::Magenta,  Color::Cyan,       Color::Gblue, Color::Brown,
            Color::Yellow,   Color::BRRed,      Color::Gray,  Color::LightBlue,
            Color::GrayBlue, Color::LightGreen, Color::LGray, Color::LGrayBlue,
            Color::LBBlue};

        auto *accent_color = ConfigurationOption::of_colors(
            "Color", available_colors, initial_config->accent_color);

        auto available_modes = {
            rendering_mode_to_str(UserInterfaceRenderingMode::Minimalistic),
            rendering_mode_to_str(UserInterfaceRenderingMode::Detailed)};

        auto *rendering_mode = ConfigurationOption::of_strings(
            "UI", available_modes,
            rendering_mode_to_str(initial_config->rendering_mode));

        auto options = {game, accent_color, rendering_mode};

        return new Configuration("Game Console", options, "Select game");
}

void extract_game_config(GameMenuConfiguration *menu_configuration,
                         Configuration *config)
{
        ConfigurationOption game_option = *config->options[0];

        int curr_option_idx = game_option.currently_selected;
        menu_configuration->game = game_from_string(static_cast<const char **>(
            game_option.available_values)[curr_option_idx]);

        ConfigurationOption accent_color = *config->options[1];

        int curr_accent_color_idx = accent_color.currently_selected;
        Color color = static_cast<Color *>(
            accent_color.available_values)[curr_accent_color_idx];
        menu_configuration->accent_color = color;

        ConfigurationOption rendering_mode = *config->options[2];

        int curr_mode_idx = rendering_mode.currently_selected;
        const char *mode_str = static_cast<const char **>(
            rendering_mode.available_values)[curr_mode_idx];
        menu_configuration->rendering_mode = rendering_mode_from_str(mode_str);
}

void select_game(Platform *p)
{
        GameMenuConfiguration config;

        collect_game_menu_config(p, &config);

        UserInterfaceCustomization customization = {
            config.accent_color,
            config.rendering_mode,
        };

        LOG_INFO(TAG, "User selected game: %s.", game_to_string(config.game));

        GameExecutor *executor = [&]() -> GameExecutor * {
                switch (config.game) {
                case Unknown:
                case Clean2048:
                        return new class Clean2048();
                case Minesweeper:
                        return new class Minesweeper();
                case GameOfLife:
                        return new class GameOfLife();
                case Settings:
                        return new class Settings();
                default:
                        return NULL;
                }
        }();

        if (!executor) {
                LOG_DEBUG(TAG, "Selected game: %d. Game not implemented yet.",
                          config.game);
                return;
        }

        executor->game_loop(p, &customization);
}

bool collect_game_menu_config(Platform *p, GameMenuConfiguration *configuration)
{

        GameMenuConfiguration *initial_config =
            load_initial_menu_configuration(p->persistent_storage);

        Configuration *config =
            assemble_menu_selection_configuration(initial_config);

        UserInterfaceCustomization customization = {
            .accent_color = initial_config->accent_color,
            .rendering_mode = initial_config->rendering_mode};

        if (collect_configuration(p, config, &customization, false))
                return false;

        extract_game_config(configuration, config);

        free_configuration(config);
        free(initial_config);
        return true;
}

Game game_from_string(const char *name)
{
        if (strcmp(name, "2048") == 0) {
                return Game::Clean2048;
        } else if (strcmp(name, game_to_string(Minesweeper)) == 0) {
                return Game::Minesweeper;
        } else if (strcmp(name, game_to_string(GameOfLife)) == 0) {
                return Game::GameOfLife;
        } else if (strcmp(name, game_to_string(MainMenu)) == 0) {
                return Game::MainMenu;
        } else if (strcmp(name, game_to_string(Settings)) == 0) {
                return Game::Settings;
        }
        return Game::Unknown;
}

const char *game_to_string(Game game)
{
        switch (game) {
        case MainMenu:
                return "Main Menu";
        case Clean2048:
                return "2048";
        case Minesweeper:
                return "Minesweeper";
        case GameOfLife:
                return "Game Of Life";
        case Settings:
                return "Settings";
        default:
                return "Unknown";
        }
}
