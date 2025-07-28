#ifndef CONFIGURATION_H
#define CONFIGURATION_H

typedef enum ConfigurationOptionType {
        INT,
        STRING,
        COLOR,
} ConfigurationOptionType;

typedef struct ConfigurationValue {
        /**
         * The type of the configurable values. Based on this type we know
         * how to cast the `available_values`.
         */
        ConfigurationOptionType type;
        /**
         * Pointer to the head of the array that stores the finite list of
         * available configuration value options.
         */
        void *available_values;
        /**
         * Number of available configuration options for this
         * `ConfigurationValue`.
         */
        int available_values_len;
        // Currently selected option for this configuration value.
        int currently_selected;
        // Name of the configuration value
        const char *name;
        // Max configuration option string length for UI rendering alignment
        int max_config_option_len;

} ConfigurationValue;

/**
 * A generic container for game configuration values. It allows for storing
 * an arbitrary number of configuration values of type int or string. The idea
 * is that for each configuration value, we provide an array of available
 * values, the currently selected value and the name of configuration option
 * that should be used for rendering in the UI.
 */
struct Configuration {
        /**
         * Represents the configuration value that is currently selected in the
         * UI and is being edited by the user.
         */
        int current_config_value;
        /// Stores the number of configurable values.
        int config_values_len;
        /**
         * Configuration values are represented as a void pointer to an array
         * of pointers to configuration values of unspecified types.
         * This is to allow for collecting string / int type inputs.
         *
         * Idea: it might be simpler to always do strings as we'll need those
         * for rendering anyway. In which case for int-type inputs we could
         * simply represent them as strings and then parse them once the
         * configuration is collected
         */
        ConfigurationValue **configuration_values;
        /// Name of the configuration group.
        const char *name;
        /**
         * At the bottom of each config list we render a button that allows the
         * user to confirm their selection of all configs. This field allows us
         * to customize the text that is displayed on the confirmation button.
         */
        const char *confirmation_cell_text;
};

/**
 * Encapsulates the difference in the configuration that has been recorded
 *   after getting user input. This is to allow for selective redrawing of the
 *   configuration menu in the UI and avoid redrawing parts that haven't been
 *   changed. There are two items that can change:
 *   - the currently edited configuration option (the indicator dot needs to be
 *   redrawn)
 *   - the value of a given configuration option changes and needs to be redrawn
 *
 *   For the sake of simplicity, this struct stores fields for both options and
 *   a boolean flag indicating which one has actually changed.
 */
struct ConfigurationDiff {
        /**
         * The two indexes below control tell us what was the previous position
         * of the indicator and the new one. This is used for redrawing the
         * indicator.
         */
        int previously_edited_option;
        int currently_edited_option;
        /**
         * If `option_switched` is false, this field tells us which
         * configuration field has been updated and needs to be redrawn.
         */
        int modified_option_index;
};

void switch_edited_config_option_down(Configuration *config,
                                      ConfigurationDiff *diff);
void switch_edited_config_option_up(Configuration *config,
                                    ConfigurationDiff *diff);

void increment_current_option_value(Configuration *config,
                                     ConfigurationDiff *diff);
void decrement_current_option_value(Configuration *config,
                                       ConfigurationDiff *diff);
int find_max_config_option_name_text_length(Configuration *config);
int find_max_config_option_value_text_length(Configuration *config);

#endif
