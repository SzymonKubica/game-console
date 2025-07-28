#include <stdlib.h>
#include "../common/user_interface.h"
#include "../common/configuration.hpp"
#include "../common/platform/interface/color.hpp"

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

        config->config_values_len = 3;
        config->current_config_value = 0;
        config->configuration_values = static_cast<ConfigurationValue **>(
            malloc(config->config_values_len * sizeof(ConfigurationValue *)));
        config->configuration_values[0] = game;
        config->configuration_values[1] = accent_color;
        config->confirmation_cell_text = "Next";
        return config;
}

void placeholder (){
        Configuration *config = assemble_game_selection_configuration();
        ConfigurationDiff *diff = empty_diff();
        render_config_menu(p->display, config, diff, false);
        free(diff);

        while (true) {
                Direction dir;
                bool ready = false;
                // We get a fresh, empty diff during each iteration to avoid
                // option value text rerendering when they are not modified.
                ConfigurationDiff *diff = empty_diff();
                if (input_registered(p->controllers, &dir)) {
                        switch (dir) {
                        case DOWN:
                                switch_edited_config_option_down(config, diff);
                                break;
                        case UP:
                                switch_edited_config_option_up(config, diff);
                                break;
                        case LEFT:
                                decrement_current_option_value(config, diff);
                                break;
                        case RIGHT:
                                /* When the user selects the last config bar,
                                   i.e. the 'confirmation cell' pressing right
                                   on it will confirm the selected config and
                                   break out of the config collection loop. */
                                if (config->current_config_value ==
                                    config->config_values_len) {
                                        ready = true;
                                } else {
                                        increment_current_option_value(config,
                                                                       diff);
                                }
                                break;
                        }

                        render_config_menu(p->display, config, diff, true);
                        free(diff);

                        if (ready) {
                                extract_game_config(game_config, customization,
                                                    config);
                                break;
                        }
                        p->delay_provider->delay_ms(MOVE_REGISTERED_DELAY);
                }
                p->delay_provider->delay_ms(INPUT_POLLING_DELAY);
        }

}
