#include "configuration.hpp"
#include "logging.hpp"
#include "platform/interface/color.hpp"
#include <cassert>
#include <string.h>

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
                  config->current_config_value);
        int config_len = config->config_values_len + 1;
        diff->previously_edited_option = config->current_config_value;
        config->current_config_value = mathematical_modulo(
            config->current_config_value + steps, config_len);
        diff->currently_edited_option = config->current_config_value;
        LOG_DEBUG(TAG, "Config option index after switching: %d",
                  config->current_config_value);
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
        assert(config->current_config_value != config->config_values_len);
        ConfigurationValue *current =
            config->configuration_values[config->current_config_value];

        current->currently_selected = mathematical_modulo(
            current->currently_selected + steps, current->available_values_len);

        diff->modified_option_index = config->current_config_value;
}

/** For some reason when compiling the `max` function is not available.
 */
int max(int a, int b) { return (a > b) ? a : b; }

int find_max_config_option_value_text_length(Configuration *config)
{
        int max_length = 0;
        for (int i = 0; i < config->config_values_len; i++) {
                int current_option_value_length;
                ConfigurationValue *current = config->configuration_values[i];
                max_length = max(max_length, current->max_config_option_len);
        }
        return max_length;
}

int find_max_config_option_name_text_length(Configuration *config)
{
        int max_length = 0;
        for (int i = 0; i < config->config_values_len; i++) {
                ConfigurationValue *current = config->configuration_values[i];
                max_length = max(max_length, strlen(current->name));
        }
        return max_length;
}
