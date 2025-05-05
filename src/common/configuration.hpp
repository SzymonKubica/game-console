#ifndef CONFIGURATION_H
#define CONFIGURATION_H

typedef enum ConfigurationOptionType {
        INT,
        STRING,
} ConfigurationOptionType;

template <typename T> struct ConfigurationValue {
        /// Pointer to the head of the array that stores the finite list of
        /// available configuration value options.
        T *available_values;
        /// Numer of available configuration options for this
        /// `ConfigurationValue`.
        int available_values_len;
        /// Currently selected option for this configuration value.
        int currently_selected;
        /// Name of the configuration value
        char *name;
        /// Max config option string length for UI rendering purposes
        int max_config_option_len;
} ;

/**
 * A generic container for game configuration values. It allows for storing
 * an arbitrary number of configuration values of type int or string. The idea
 * is that for each configuration value, we provide an array of available
 * values, the currently selected value and the name of configuration option
 * that should be used for rendering in the UI.
 */
struct Configuration {
        /// Represents the configuration value that is currently selecte in the
        /// UI and is being edited by the user.
        int current_config_value;
        /// Should store the number of configurable values.
        int config_values_len;
        /// Configuration values are represented as a void pointer to an array
        /// of pointers to configuration values of unspecified types.
        void **configuration_values;
        /// Maintains information about the types of the configurable values
        /// the idea is that a single configuration should allow for setting
        /// both integer and string valuea out of finite lists of options.
        ConfigurationOptionType *type_map;
        /// Name of the configuration group.
        char *name;
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
        /// If true, the diff indicates that the currently edited configuration
        /// option has changed and we need to redraw the indicator dot.
        bool option_switched;
        int previously_edited_option;
        int currently_edited_option;
        /// If `option_switched` is false, this field tells us which
        /// configuration field has been updated and needs to be redrawn.
        int modified_option_index;
};

void switch_edited_config_option_down(Configuration *config, ConfigurationDiff *diff);
void switch_edited_config_option_up(Configuration *config, ConfigurationDiff *diff);

void switch_current_config_option_up(Configuration *config, ConfigurationDiff *diff);
void switch_current_config_option_down(Configuration *config, ConfigurationDiff *diff);
int find_max_config_option_name_length(Configuration *config);

#endif
