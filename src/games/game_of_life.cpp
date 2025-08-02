#include "../common/logging.hpp"
#include "../common/configuration.hpp"
#include "game_executor.hpp"
#include <cstring>

#define TAG "game_of_life"

typedef struct GameOfLifeConfiguration {
        bool prepopulate_grid;
        int simulation_speed;
} GameOfLifeConfiguration;

static void collect_game_configuration(Platform *p,
                                       GameOfLifeConfiguration *game_config,
                                       GameCustomization *customization);
void extract_game_config(GameOfLifeConfiguration *game_config,
                         Configuration *config);
Configuration *assemble_game_of_life_configuration();
void enter_game_of_life_loop(Platform *platform,
                             GameCustomization *customization)
{

        LOG_DEBUG(TAG, "Entering Minesweeper game loop");
        GameOfLifeConfiguration config;

        collect_game_configuration(platform, &config, customization);
}

void collect_game_configuration(Platform *p,
                                GameOfLifeConfiguration *game_config,
                                GameCustomization *customization)
{
        Configuration *config = assemble_game_of_life_configuration();
        enter_configuration_collection_loop(p, config,
                                            customization->accent_color);
        extract_game_config(game_config, config);
        free_configuration(config);
}

Configuration *assemble_game_of_life_configuration()
{
        Configuration *config = new Configuration();
        config->name = "Game of Life";

        // Initialize the first config option: game gridsize
        ConfigurationOption *choices = new ConfigurationOption();
        choices->name = "Spawn randomly";
        std::vector<const char *> available_values = {"Yes", "No"};
        populate_string_option_values(choices, available_values);
        choices->currently_selected = 1;

        ConfigurationOption *simulation_speed = new ConfigurationOption();
        simulation_speed->name = "Speed";
        std::vector<int> available_speeds = {1, 2, 3};
        populate_int_option_values(simulation_speed, available_speeds);
        simulation_speed->currently_selected = 1;

        config->options_len = 2;
        config->options = new ConfigurationOption *[config->options_len];
        config->options[0] = choices;
        config->options[1] = simulation_speed;
        config->curr_selected_option = 0;
        config->confirmation_cell_text = "Start Game";
        return config;
}

void extract_game_config(GameOfLifeConfiguration *game_config,
                         Configuration *config)
{

        ConfigurationOption prepopulate_grid = *config->options[0];
        int curr_choice_idx = prepopulate_grid.currently_selected;

        const char *choice = static_cast<const char **>(
            prepopulate_grid.available_values)[curr_choice_idx];
        if (strlen(choice) == 3 && strncmp(choice, "Yes", 3) == 0) {
                game_config->prepopulate_grid = true;
        }
        if (strlen(choice) == 2 && strncmp(choice, "No", 2) == 0) {
                game_config->prepopulate_grid = false;
        }
        // Grid size is the first config option in the game struct
        // above.
        ConfigurationOption simulation_speed = *config->options[1];

        int curr_speed_idx = simulation_speed.currently_selected;
        game_config->simulation_speed = static_cast<int *>(
            simulation_speed.available_values)[curr_speed_idx];
}
