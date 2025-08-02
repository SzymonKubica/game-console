#include <cassert>
#include <string>
#include <stdlib.h>

#include "configuration.hpp"
#include "constants.hpp"
#include "logging.hpp"
#include "user_interface.h"
#include "platform/interface/controller.hpp"
#include "platform/interface/color.hpp"

#define TAG "configuration"

static int mathematical_modulo(int a, int b);

void shift_edited_config_option(Configuration *config, ConfigurationDiff *diff,
                                int steps);

/**
 * Modifies the `Configuration` that is passed in. It switches the currently
 * edited option to the one above it (or wraps around to the bottom of the
 * configuration menu). Given that the options are rendered top to bottom it
 * decrements the index of the currently edited config option. The modification
 * that is applied to the config is encoded in the `ConfigurationDiff` object
 * which is then used to partially re-render the menu. */
void switch_edited_config_option_up(Configuration *config,
                                    ConfigurationDiff *diff)
{
        shift_edited_config_option(config, diff, -1);
}

/**
 * Modifies the `Configuration` that is passed in. It switches the currently
 * edited option to the one below it (or wraps around to the top of the
 * configuration menu). Given that the options are rendered top to bottom it
 * increments the index of the currently edited config option. The modification
 * that is applied to the config is encoded in the `ConfigurationDiff` object
 * which is then used to partially re-render the menu. */
void switch_edited_config_option_down(Configuration *config,
                                      ConfigurationDiff *diff)
{
        shift_edited_config_option(config, diff, 1);
}

/**
 * Shifts the currently edited config option. We wrap modulo
 * `config->config_values_len`
 * + 1 because the last config bar allows the user to confirm selection.
 */
void shift_edited_config_option(Configuration *config, ConfigurationDiff *diff,
                                int steps)
{
        LOG_DEBUG(TAG, "Config option index before switching: %d",
                  config->curr_selected_option);
        int config_len = config->options_len + 1;
        diff->previously_edited_option = config->curr_selected_option;
        config->curr_selected_option = mathematical_modulo(
            config->curr_selected_option + steps, config_len);
        diff->currently_edited_option = config->curr_selected_option;
        LOG_DEBUG(TAG, "Config option index after switching: %d",
                  config->curr_selected_option);
}

int mathematical_modulo(int a, int b)
{
        // This is a workaround for the fact that the % operator in C++ can
        // return negative values if the first operand is negative.
        return (a % b + b) % b;
}

void shift_current_config_option_value(Configuration *config,
                                       ConfigurationDiff *diff, int steps);

/**
 * Modifies the currently selected configuration bar by incrementing the
 * index of the value of the configuration controlled by this setting.
 */
void increment_current_option_value(Configuration *config,
                                    ConfigurationDiff *diff)
{
        shift_current_config_option_value(config, diff, 1);
}

/**
 * Modifies the currently selected configuration bar by decrementing the
 * index of the value of the configuration controlled by this setting.
 */
void decrement_current_option_value(Configuration *config,
                                    ConfigurationDiff *diff)
{
        shift_current_config_option_value(config, diff, -1);
}

void shift_current_config_option_value(Configuration *config,
                                       ConfigurationDiff *diff, int steps)
{
        assert(config->curr_selected_option != config->options_len);
        ConfigurationOption *current =
            config->options[config->curr_selected_option];

        current->currently_selected = mathematical_modulo(
            current->currently_selected + steps, current->available_values_len);

        diff->modified_option_index = config->curr_selected_option;
}

/** For some reason when compiling the `max` function is not available.
 */
int max(int a, int b) { return (a > b) ? a : b; }

int find_max_config_option_value_text_length(Configuration *config)
{
        int max_length = 0;
        for (int i = 0; i < config->options_len; i++) {
                int current_option_value_length;
                ConfigurationOption *current = config->options[i];
                max_length = max(max_length, current->max_config_value_len);
        }
        return max_length;
}

int find_max_config_option_name_text_length(Configuration *config)
{
        int max_length = 0;
        for (int i = 0; i < config->options_len; i++) {
                ConfigurationOption *current = config->options[i];
                max_length = max(max_length, strlen(current->name));
        }
        return max_length;
}

int find_max_number_length(std::vector<int> numbers);
void populate_int_option_values(ConfigurationOption *value,
                                std::vector<int> available_values)
{
        value->type = ConfigurationOptionType::INT,
        value->available_values_len = available_values.size();
        int *values = new int[value->available_values_len];
        for (int i = 0; i < value->available_values_len; i++) {
                values[i] = available_values[i];
        }
        value->available_values = values;
        value->max_config_value_len = find_max_number_length(available_values);
}

int find_max_string_length(std::vector<const char *> strings);
void populate_string_option_values(ConfigurationOption *value,
                                   std::vector<const char *> available_values)
{
        value->type = ConfigurationOptionType::STRING,
        value->available_values_len = available_values.size();
        const char **values = new const char *[value->available_values_len];
        for (int i = 0; i < value->available_values_len; i++) {
                values[i] = available_values[i];
        }
        value->available_values = values;
        value->max_config_value_len = find_max_string_length(available_values);
}

int find_max_color_str_length(std::vector<Color> available_values);
void populate_color_option_values(ConfigurationOption *value,
                                  std::vector<Color> available_values)
{
        value->type = ConfigurationOptionType::COLOR,
        value->available_values_len = available_values.size();
        Color *values = new Color[value->available_values_len];
        for (int i = 0; i < value->available_values_len; i++) {
                values[i] = available_values[i];
        }
        value->available_values = values;
        value->max_config_value_len =
            find_max_color_str_length(available_values);
}

int find_max_number_length(std::vector<int> numbers)
{
        int max_len = 0;
        for (int value : numbers) {
                max_len = max(max_len, std::to_string(value).size());
        }
        return max_len;
}

int find_max_string_length(std::vector<const char *> strings)
{
        int max_len = 0;
        for (const char *value : strings) {
                max_len = max(max_len, strlen(value));
        }
        return max_len;
}

int find_max_color_str_length(std::vector<Color> colors)
{
        int max_len = 0;
        for (Color value : colors) {
                max_len = max(max_len, strlen(map_color(value)));
        }
        return max_len;
}

void enter_configuration_collection_loop(Platform *p, Configuration *config,
                                         Color accent_color)
{

        ConfigurationDiff *diff = empty_diff();
        render_config_menu(p->display, config, diff, false, accent_color);
        free(diff);
        while (true) {
                Action act;
                Direction dir;
                // We get a fresh, empty diff during each iteration to avoid
                // option value text rerendering when they are not modified.
                ConfigurationDiff *diff = empty_diff();
                bool confirmation_bar_selected =
                    config->curr_selected_option == config->options_len;
                if (action_input_registered(p->action_controllers, &act)) {
                        /* To make the UI more intuitive, we also allow users to
                        cycle configuration options and confirm final selection
                        using the green button. This change was inspired by
                        initial play testing by Tomek. */
                        if (act == Action::GREEN) {
                                if (confirmation_bar_selected) {
                                        p->delay_provider->delay_ms(
                                            MOVE_REGISTERED_DELAY);
                                        break;
                                } else {
                                        increment_current_option_value(config,
                                                                       diff);
                                        render_config_menu(p->display, config,
                                                           diff, true,
                                                           accent_color);
                                        free(diff);
                                        p->delay_provider->delay_ms(
                                            MOVE_REGISTERED_DELAY);
                                        continue;
                                }
                        }
                }
                if (directional_input_registered(p->directional_controllers,
                                                 &dir)) {
                        /* When the user selects the last config bar,
                           i.e. the 'confirmation cell' pressing right
                           on it confirms the selected config and
                           breaks out of the config collection loop. */
                        if (confirmation_bar_selected) {
                                p->delay_provider->delay_ms(
                                    MOVE_REGISTERED_DELAY);
                                if (dir == RIGHT) {
                                        break;
                                }
                                if (dir == LEFT) {
                                        continue;
                                }
                        }

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
                                increment_current_option_value(config, diff);
                                break;
                        }

                        render_config_menu(p->display, config, diff, true,
                                           accent_color);
                        free(diff);

                        p->delay_provider->delay_ms(MOVE_REGISTERED_DELAY);
                }
                p->delay_provider->delay_ms(INPUT_POLLING_DELAY);
        }
}
