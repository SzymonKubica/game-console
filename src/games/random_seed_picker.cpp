#include <cstdint>
#include <cstring>

#include "../common/logging.hpp"
#include "../common/constants.hpp"
#include "../common/maths_utils.hpp"
#include "game_executor.hpp"
#include "random_seed_picker.hpp"
#include "settings.hpp"
#include "game_menu.hpp"

#define TAG "random_seed_picker"
#define GAME_CELL_WIDTH 8

#define GAME_LOOP_DELAY 100

/**
 * Assembles the generic configuration struct that can be used to collect user
 * input specifying the game of life configuration.
 */
Configuration *
assemble_random_seed_picker_configuration(PersistentStorage *storage);

RandomSeedPickerConfiguration *
load_initial_seed_picker_config(PersistentStorage *storage)
{
        int storage_offset = get_settings_storage_offsets()[RandomSeedPicker];

        RandomSeedPickerConfiguration config = {
            .base_seed = 0,
            .reseed_iterations = 1,
        };

        LOG_DEBUG(TAG,
                  "Trying to load initial settings from the persistent storage "
                  "at offset %d",
                  storage_offset);
        storage->get(storage_offset, config);

        RandomSeedPickerConfiguration *output =
            new RandomSeedPickerConfiguration();

        if (config.reseed_iterations == 0) {
                LOG_DEBUG(
                    TAG,
                    "The storage does not contain a valid "
                    "random seed picker configuration, using default values.");
                memcpy(output, &DEFAULT_GAME_OF_LIFE_CONFIG,
                       sizeof(GameOfLifeConfiguration));
                storage->put(storage_offset, DEFAULT_GAME_OF_LIFE_CONFIG);

        } else {
                LOG_DEBUG(TAG, "Using configuration from persistent storage.");
                memcpy(output, &config, sizeof(RandomSeedPickerConfiguration));
        }

        return output;
}

void enter_random_seed_picker_loop(Platform *p,
                                   GameCustomization *customization)
{

        LOG_DEBUG(TAG, "Entering Game of Life game loop");
        GameOfLifeConfiguration config;

        collect_random_seed_picker_configuration(p, &config, customization);

        bool exit_requested = false;
        SimulationMode mode = PAUSED;
        while (!exit_requested) {
                if (mode == RUNNING && iteration == evolution_period - 1) {
                        LOG_DEBUG(TAG, "Taking a simulation step");
                        StateEvolution evolution = take_simulation_step(
                            grid, gd, config.use_toroidal_array);

                        render_state_change(p->display, evolution, gd);
                        save_grid_state_in_rewind_buffer(&rewind_buffer,
                                                         &rewind_buf_idx, grid);
                        grid = evolution.second;
                }
                Direction dir;
                Action act;
                GameOfLifeCell curr =
                    get_cell(caret_pos.x, caret_pos.y, gd->cols, grid);
                if (action_input_registered(p->action_controllers, &act)) {
                        switch (act) {
                        case RED:
                                exit_requested = true;
                                break;
                        default:
                                break;
                        }
                }
                p->delay_provider->delay_ms(GAME_LOOP_DELAY);
        }
}

void collect_random_seed_picker_configuration(
    Platform *p, RandomSeedPickerConfiguration *game_config,
    GameCustomization *customization)
{
        Configuration *config =
            assemble_random_seed_picker_configuration(p->persistent_storage);
        enter_configuration_collection_loop(p, config,
                                            customization->accent_color);
        extract_game_config(game_config, config);
        free_configuration(config);
}

Configuration *
assemble_random_seed_picker_configuration(PersistentStorage *storage)
{
        GameOfLifeConfiguration *initial_config =
            load_initial_game_of_life_config(storage);

        Configuration *config = new Configuration();
        config->name = "Random Seed Picker";

        // Initialize the first config option: base seed
        ConfigurationOption *base_seed = new ConfigurationOption();
        base_seed->name = "Base seed";
        int total_seeds = 100;
        std::vector<int> available_seeds(total_seeds);
        for (int i = 0; i < total_seeds; i++) {
                available_seeds.push_back(i);
        }
        populate_int_option_values(base_seed, available_seeds);
        // We need to use this elaborate mechanism of getting the index of the
        // default value because the config value is also saved in persistent
        // storage so this can change and cannot be hardcoded.
        base_seed->currently_selected = get_config_option_value_index(
            base_seed, map_boolean_to_yes_or_no(initial_config->base_seed));

        // Initialize the first config option: number of re-seed iterations.
        ConfigurationOption *reseed_iterations = new ConfigurationOption();
        reseed_iterations->name = "Base seed";
        int total_available_iterations =
            10 std::vector<int> available_iterations(
                total_available_iterations);
        for (int i = 0; i < total_available_iterations; i++) {
                available_iterations.push_back(i);
        }
        populate_int_option_values(reseed_iterations, available_iterations);
        // We need to use this elaborate mechanism of getting the index of the
        // default value because the config value is also saved in persistent
        // storage so this can change and cannot be hardcoded.
        reseed_iterations->currently_selected = get_config_option_value_index(
            reseed_iterations,
            map_boolean_to_yes_or_no(initial_config->base_seed));

        config->options_len = 2;
        config->options = new ConfigurationOption *[config->options_len];
        config->options[0] = base_seed;
        config->options[1] = reseed_iterations;
        config->curr_selected_option = 0;
        config->confirmation_cell_text = "Generate Seed";

        free(initial_config);
        return config;
}

// TODO
void extract_game_config(GameOfLifeConfiguration *game_config,
                         Configuration *config)
{

        ConfigurationOption prepopulate_grid = *config->options[0];
        int curr_choice_idx = prepopulate_grid.currently_selected;
        const char *choice = static_cast<const char **>(
            prepopulate_grid.available_values)[curr_choice_idx];
        game_config->prepopulate_grid = extract_yes_or_no_option(choice);

        game_config->rewind_buffer_size = REWIND_BUF_SIZE;

        ConfigurationOption simulation_speed = *config->options[1];
        int curr_speed_idx = simulation_speed.currently_selected;
        game_config->simulation_speed = static_cast<int *>(
            simulation_speed.available_values)[curr_speed_idx];

        ConfigurationOption use_toroidal_array = *config->options[2];
        int use_toroidal_array_choice_idx =
            use_toroidal_array.currently_selected;
        const char *toroidal_array_choice = static_cast<const char **>(
            use_toroidal_array.available_values)[use_toroidal_array_choice_idx];
        game_config->use_toroidal_array =
            extract_yes_or_no_option(toroidal_array_choice);
}
