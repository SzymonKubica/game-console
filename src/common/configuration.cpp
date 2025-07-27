#include "configuration.hpp"
#include <cassert>
#include <string.h>

void shift_edited_config_option_by(Configuration *config,
                                   ConfigurationDiff *diff, int steps);

/**
 * Modifies the `Configuration` that is passed in. It shifts the currently
 * edited option one unit up. The modification that is applied to the config is
 * encoded in the `ConfigurationDiff` object so that it can be then rendered
 * on the UI.
 */
void switch_edited_config_option_up(Configuration *config,
                                    ConfigurationDiff *diff)
{
        shift_edited_config_option_by(config, diff, 1);
}

/**
 * Modifies the `Configuration` that is passed in. It shifts the currently
 * edited option one unit down. The modification that is applied to the config
 * is encoded in the `ConfigurationDiff` object so that it can be then rendered
 * on the UI.
 */
void switch_edited_config_option_down(Configuration *config,
                                      ConfigurationDiff *diff)
{
        shift_edited_config_option_by(config, diff, -1);
}

/**
 * Shifts the currently edited config option. We wrap modulo
 * `config->config_values_len`
 * + 1 because the last config bar allows the user to confirm selection.
 */
void shift_edited_config_option_by(Configuration *config,
                                   ConfigurationDiff *diff, int steps)
{
        int config_len = config->config_values_len + 1;
        diff->option_switched = true;
        diff->previously_edited_option = config->current_config_value;
        config->current_config_value =
            (config->current_config_value + steps) % config_len;
        diff->currently_edited_option = config->current_config_value;
}

void shift_current_config_option_by(Configuration *config,
                                    ConfigurationDiff *diff, int steps);

void switch_current_config_option_up(Configuration *config,
                                     ConfigurationDiff *diff)
{
        shift_current_config_option_by(config, diff, 1);
}
void switch_current_config_option_down(Configuration *config,
                                       ConfigurationDiff *diff)
{
        shift_current_config_option_by(config, diff, -1);
}

void shift_current_config_option_by(Configuration *config,
                                    ConfigurationDiff *diff, int steps)
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
int max(int a, int b) { return (a > b) ? a : b; }

int find_max_config_option_value_text_length(Configuration *config)
{
        int max_length = 0;
        for (int i = 0; i < config->config_values_len; i++) {
                int current_option_value_length;
                if (config->type_map[i] == ConfigurationOptionType::INT) {
                        ConfigurationValue<int> *current =
                            (ConfigurationValue<int> *)
                                config->configuration_values[i];
                        current_option_value_length =
                            current->max_config_option_len;
                } else {
                        ConfigurationValue<char *> *current =
                            (ConfigurationValue<char *> *)
                                config->configuration_values[i];
                        current_option_value_length =
                            current->max_config_option_len;
                }
                max_length = max(max_length, current_option_value_length);
        }
        return max_length;
}

int find_max_config_option_name_text_length(Configuration *config)
{
        int max_length = 0;
        for (int i = 0; i < config->config_values_len; i++) {
                char *name;
                if (config->type_map[i] == ConfigurationOptionType::INT) {
                        ConfigurationValue<int> *current =
                            (ConfigurationValue<int> *)
                                config->configuration_values[i];
                        name = current->name;
                } else {
                        ConfigurationValue<char *> *current =
                            (ConfigurationValue<char *> *)
                                config->configuration_values[i];
                        name = current->name;
                }
                max_length = max(max_length, strlen(name));
        }
        return max_length;
}
