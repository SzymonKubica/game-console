#pragma once

#include "platform/interface/platform.hpp"
#include "user_interface_customization.hpp"
#include <optional>

typedef enum ConfigurationOptionType {
        INT,
        STRING,
        COLOR,
} ConfigurationOptionType;

enum class UserAction {
        PlayAgain,
        Exit,
        ShowHelp,
};

typedef struct ConfigurationOption {
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
        int max_config_value_len;

        ConfigurationOption()
            : type(INT), available_values(nullptr), available_values_len(0),
              currently_selected(0), name(nullptr), max_config_value_len(0)
        {
        }

      public:
        static ConfigurationOption *of_integers(const char *name,
                                                std::vector<int> values,
                                                int initial_value);
        static ConfigurationOption *of_strings(const char *name,
                                               std::vector<const char *> values,
                                               const char *initial_value);
        static ConfigurationOption *of_colors(const char *name,
                                              std::vector<Color> values,
                                              Color initial_value);

} ConfigurationOption;

/**
 * Given a value that is specifiable by the `ConfigurationOption` it returns the
 * index of this value in the array of available values. This is needed to map
 * from the default game configurations to the default indices when loading the
 * configuration menu.
 */
template <typename T>
int get_config_option_value_index(ConfigurationOption *option, T value)
{
        for (int i = 0; i < option->available_values_len; i++) {
                if (static_cast<T *>(option->available_values)[i] == value) {
                        return i;
                }
        }
        return -1;
}

int get_config_option_string_value_index(ConfigurationOption *option,
                                         const char *value);

struct Configuration;
void populate_options(Configuration *config,
                      std::vector<ConfigurationOption *> options,
                      int currently_selected);

/**
 * A generic container for game configuration values. It allows for storing
 * an arbitrary number of configuration values of type int or string. The idea
 * is that for each configuration value, we provide an array of available
 * values, the currently selected value and the name of configuration option
 * that should be used for rendering in the UI.
 */
struct Configuration {
        /// Name of the configuration group.
        const char *name;
        /**
         * An array of pointers to configuration options available on this
         * configuration object. Each option has a set of values from which the
         * user can select.
         */
        ConfigurationOption **options;
        /// Stores the number of configurable options.
        int options_len;
        /**
         * Represents the configuration value that is currently selected in the
         * UI and is being edited by the user.
         */
        int curr_selected_option;
        /**
         * At the bottom of each config list we render a button that allows the
         * user to confirm their selection of all configs. This field allows us
         * to customize the text that is displayed on the confirmation button.
         */
        const char *confirmation_cell_text;

        Configuration()
            : name(nullptr), options(nullptr), options_len(0),
              curr_selected_option(0), confirmation_cell_text(nullptr)
        {
        }

        Configuration(const char *name,
                      std::vector<ConfigurationOption *> options,
                      const char *confirmation_cell_text)
            : name(name), options(nullptr), options_len(options.size()),
              curr_selected_option(0),
              confirmation_cell_text(confirmation_cell_text)
        {
                this->options = new ConfigurationOption *[this->options_len];
                populate_options(this, options, 0);
        }
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

/**
 * Given a platform providing the display and controllers implementation
 * and a pointer to the configuration object, this function allows users
 * to modify the configuration. The final settings are written into the
 * `config` struct that is passed as a parameter.
 *
 * It returns true if the configuration was successfully collected. If the user
 * requested to go back, it returns false.
 *
 * While the configuration is being collected, the user has ability to abort the
 * process by either requesting exit or asking for help screen (and possibly
 * more actions in the future). This is controlled by the return parameter. If
 * the return is `std::nullopt`, it means that the configuration was collected
 * and no interrupt action was registered. Otherwise, the function returns
 * some `UserInterruptAction` that needs to be handled by the game loop that
 * started collecting the configuration.
 */
std::optional<UserAction>
collect_configuration(Platform *p, Configuration *config,
                      UserInterfaceCustomization *customization,
                      bool allow_exit = true);

void populate_int_option_values(ConfigurationOption *value,
                                std::vector<int> available_values);
void populate_string_option_values(ConfigurationOption *value,
                                   std::vector<const char *> available_values);
void populate_color_option_values(ConfigurationOption *value,
                                  std::vector<Color> available_values);

void populate_options(Configuration *config,
                      std::vector<ConfigurationOption *> options,
                      int currently_selected);

void free_configuration(Configuration *config);
