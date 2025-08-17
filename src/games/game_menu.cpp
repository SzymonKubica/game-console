#include <stdlib.h>
#include "game_menu.hpp"
#include "../common/configuration.hpp"
#include "../common/logging.hpp"
#include "../common/platform/interface/color.hpp"
#include "2048.hpp"
#include "minesweeper.hpp"
#include "settings.hpp"
#include "game_of_life.hpp"

#define TAG "game_menu"

Configuration *assemble_menu_selection_configuration()
{
        Configuration *config = new Configuration();
        config->name = "Game Console";

        ConfigurationOption *game = new ConfigurationOption();
        game->name = "Game";
        auto available_games = {"Sweeper", "2048", "Life", "Settings"};
        populate_string_option_values(game, available_games);
        game->currently_selected = 2;

        ConfigurationOption *accent_color = new ConfigurationOption();
        accent_color->name = "Accent color";
        auto available_accent_colors = {Color::Red,     Color::Green,
                                        Color::Blue,    Color::DarkBlue,
                                        Color::Magenta, Color::Cyan};
        populate_color_option_values(accent_color, available_accent_colors);
        accent_color->currently_selected = 3;

        int options_num = 2;
        config->options_len = options_num;
        config->options = new ConfigurationOption *[options_num];
        config->options[0] = game;
        config->options[1] = accent_color;
        config->curr_selected_option = 0;
        config->confirmation_cell_text = "Next";
        return config;
}

void extract_game_config(Game *selected_game, GameCustomization *customization,
                         Configuration *config)
{
        ConfigurationOption game_option = *config->options[0];

        int curr_option_idx = game_option.currently_selected;
        *selected_game = map_game_from_str(static_cast<const char **>(
            game_option.available_values)[curr_option_idx]);

        // Game target is the second config option above.
        ConfigurationOption accent_color = *config->options[1];

        int curr_accent_color_idx = accent_color.currently_selected;
        Color color = static_cast<Color *>(
            accent_color.available_values)[curr_accent_color_idx];
        customization->accent_color = color;
}

void select_game(Platform *p)
{
        Configuration *config = assemble_menu_selection_configuration();
        enter_configuration_collection_loop(p, config);

        Game selected_game;
        GameCustomization customization;
        extract_game_config(&selected_game, &customization, config);

        switch (selected_game) {
        case Unknown:
        case Clean2048:
                (new class Clean2048())->enter_game_loop(p, &customization);
                break;
        case Snake:
                LOG_DEBUG(TAG,
                          "Selected game: Snake. Game not implemented yet.");
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
                          selected_game);
                break;
        }
}

Game map_game_from_str(const char *name)
{
        if (strcmp(name, "2048") == 0) {
                return Game::Clean2048;
        } else if (strcmp(name, "Snake") == 0) {
                return Game::Snake;
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
