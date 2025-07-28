#include <stdlib.h>
#include "game_menu.hpp"
#include "../common/configuration.hpp"
#include "../common/logging.hpp"
#include "../common/platform/interface/color.hpp"
#include "2048.hpp"

#define TAG "game_menu"

Configuration *assemble_game_selection_configuration()
{
        Configuration *config =
            static_cast<Configuration *>(malloc(sizeof(Configuration)));

        config->name = "Game Console";

        // Initialize the first config option: game gridsize
        ConfigurationValue *game = static_cast<ConfigurationValue *>(
            malloc(2 * sizeof(ConfigurationValue)));
        game->type = ConfigurationOptionType::STRING;
        game->name = "Game";
        game->available_values_len = 2;
        const char **available_games =
            new const char *[game->available_values_len];
        available_games[0] = "2048";
        available_games[1] = "Snake";
        game->available_values = available_games;
        game->currently_selected = 1;
        game->max_config_option_len = 1;

        ConfigurationValue *accent_color = static_cast<ConfigurationValue *>(
            malloc(sizeof(ConfigurationValue)));
        accent_color->name = "Accent color";
        accent_color->type = ConfigurationOptionType::COLOR;
        accent_color->available_values_len = 4;
        Color *available_accent_colors =
            new Color[accent_color->available_values_len];
        available_accent_colors[0] = Color::Red;
        available_accent_colors[1] = Color::Green;
        available_accent_colors[2] = Color::Blue;
        available_accent_colors[3] = Color::DarkBlue;
        accent_color->available_values = available_accent_colors;
        accent_color->currently_selected = 3;
        accent_color->max_config_option_len =
            strlen(map_color(Color::DarkBlue));

        config->config_values_len = 2;
        config->current_config_value = 0;
        config->configuration_values = static_cast<ConfigurationValue **>(
            malloc(config->config_values_len * sizeof(ConfigurationValue *)));
        config->configuration_values[0] = game;
        config->configuration_values[1] = accent_color;
        config->confirmation_cell_text = "Next";
        return config;
}

void extract_game_config(Game *selected_game, GameCustomization *customization,
                         Configuration *config)
{
        // Grid size is the first config option in the game struct above.
        ConfigurationValue grid_size = *config->configuration_values[0];

        int curr_grid_size_idx = grid_size.currently_selected;
        *selected_game = map_game_from_str(static_cast<const char **>(
            grid_size.available_values)[curr_grid_size_idx]);

        // Game target is the second config option above.
        ConfigurationValue accent_color = *config->configuration_values[1];

        int curr_accent_color_idx = accent_color.currently_selected;
        Color color = static_cast<Color *>(
            accent_color.available_values)[curr_accent_color_idx];
        customization->border_color = color;
}

void select_game(Platform *p)
{
        Configuration *config = assemble_game_selection_configuration();
        enter_configuration_collection_loop(p, config);

        Game selected_game;
        GameCustomization customization;
        extract_game_config(&selected_game, &customization, config);

        switch (selected_game) {
        case Unknown:
        case Clean2048:
                enter_game_loop(p);
                break;
        case Snake:
                LOG_DEBUG(TAG,
                          "Selected game: Snake. Game not implemented yet.");
                break;
        }
}

Game map_game_from_str(const char *name)
{
        if (strcmp(name, "2048") == 0) {
                return Game::Clean2048;
        } else if (strcmp(name, "Snake") == 0) {
                return Game::Snake;
        }
        return Game::Unknown;
}
