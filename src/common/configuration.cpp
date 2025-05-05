#include "configuration.h"
#include <cassert>
#include <string.h>

void shiftEditedConfigOptionBy(Configuration *config, ConfigurationDiff *diff,
                               int steps);

/**
 * Modifies the `Configuration` that is passed in. It shifts the currently
 * edited option one unit up. The modification that is applied to the config is
 * encoded in the `ConfigurationDiff` object so that it can be then rendered
 * on the UI.
 */
void switchEditedConfigOptionUp(Configuration *config, ConfigurationDiff *diff)
{
        shiftEditedConfigOptionBy(config, diff, 1);
}

/**
 * Modifies the `Configuration` that is passed in. It shifts the currently
 * edited option one unit down. The modification that is applied to the config
 * is encoded in the `ConfigurationDiff` object so that it can be then rendered
 * on the UI.
 */
void switchEditedConfigOptionDown(Configuration *config,
                                  ConfigurationDiff *diff)
{
        shiftEditedConfigOptionBy(config, diff, -1);
}

/**
 * Shifts the currently edited config option. We wrap modulo
 * `config->config_values_len`
 * + 1 because the last config bar allows the user to confirm selection.
 */
void shiftEditedConfigOptionBy(Configuration *config, ConfigurationDiff *diff,
                               int steps)
{
        int config_len = config->config_values_len + 1;
        diff->option_switched = true;
        diff->previously_edited_option = config->current_config_value;
        config->current_config_value =
            (config->current_config_value + steps) % config_len;
        diff->currently_edited_option = config->current_config_value;
}

void shiftCurrentConfigOptionBy(Configuration *config, ConfigurationDiff *diff,
                                int steps);

void switchCurrentConfigOptionUp(Configuration *config, ConfigurationDiff *diff)
{
        shiftCurrentConfigOptionBy(config, diff, 1);
}
void switchCurrentConfigOptionDown(Configuration *config,
                                   ConfigurationDiff *diff)
{
        shiftCurrentConfigOptionBy(config, diff, -1);
}

void shiftCurrentConfigOptionBy(Configuration *config, ConfigurationDiff *diff,
                                int steps)
{
        assert(config->current_config_value != config->config_values_len);
        void *currently_edited =
            config->configuration_values[config->current_config_value];

        if (config->type_map[config->current_config_value] ==
            ConfigurationOptionType::INT) {
                ConfigurationValue<int> *current =
                    (ConfigurationValue<int> *)currently_edited;
                current->currently_selected =
                    (current->currently_selected + steps) %
                    current->available_values_len;
        } else {
                ConfigurationValue<char *> *current =
                    (ConfigurationValue<char *> *)currently_edited;
                current->currently_selected =
                    (current->currently_selected + steps) %
                    current->available_values_len;
        }
}

/** For some reason when compiling the `max` function is not available.
 */
int max(int a, int b)
{
        return (a > b) ? a : b;
}

int findMaxConfigOptionNameLength(Configuration *config)
{
        int max_length = 0;
        for (int i = 0; i < config->config_values_len; i++) {
                if (config->type_map[i] == ConfigurationOptionType::INT) {
                        ConfigurationValue<int> *current =
                            (ConfigurationValue<int> *)
                                config->configuration_values[i];
                        max_length =
                            max(max_length, strlen(current->name) +
                                                current->max_config_option_len);
                } else {
                        ConfigurationValue<char *> *current =
                            (ConfigurationValue<char *> *)
                                config->configuration_values[i];
                        max_length =
                            max(max_length, strlen(current->name) +
                                                current->max_config_option_len);
                }
        }
        return max_length;
}
