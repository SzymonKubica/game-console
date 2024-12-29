#ifndef GAME2048_H
#define GAME2048_H

typedef enum ConfigOption {
        GRID_SIZE = 0,
        TARGET_MAX_TILE = 1,
        READY_TO_GO = 2,
} ConfigOption;

typedef struct GameConfiguration {
        int grid_size;
        int target_max_tile;
        ConfigOption config_option;
} GameConfiguration;

typedef struct GameState {
        int **grid;
        int **old_grid;
        int score;
        int occupied_tiles;
        int grid_size;
        int target_max_tile;
} GameState;

/* Figuring out the abstraction for getting numeric / enumerated input
 * from the user in the UI so that it can be reused across different games.
 *
 * we need:
 * - number of config options
 * - for each config option number of available values
 * - an array of actual values (ints or strings)
 * - a variable corresponding to the current config
 *
 *
 *
 */

template <typename T> struct ConfigurationValue {
        int available_values_len;
        int currently_selected;
        T *available_values;
        char *name;
};

typedef enum ConfigurationOptionType {
        INT,
        STRING,
};

struct Configuration {
        int currently_edited_config_value;
        int configuration_values_len;
        /// Configuration values are represented as a void pointer to an array
        /// of pointers to configuration values of unspecified types.
        void *configuration_values;
        ConfigurationOptionType *type_map;
};

/** Encapsulates the difference in the configuration that has been recorded
 after getting user input. This is to allow for selective redrawing of the
 configuration menu in the UI and avoid redrawing parts that haven't been
 changed. There are two items that can change:
 - the currently edited configuration option (the indicator dot needs to be redrawn)
 - the value of a given configuration option changes and needs to be redrawn

 For the sake of simplicity, this struct stores fields for both options and
 a boolean flag indicating which one has actually changed.
*/
struct ConfigurationDiff {
    /// If true, the diff indicates that the currently edited configuration
    /// option has changed and we need to redraw the indicator dot.
    bool option_switched;
    int previously_edited_option;
    int currently_edited_option;
    /// If `option_switched` is false, this field tells us which configuration
    /// field has been updated and needs to be redrawn.
    int modified_option_index;
};

GameState *
initializeGameState(int gridSize, int target_max_tile);
void initializeRandomnessSeed(int seed);
bool isGameOver(GameState *gs);
bool isGameFinished(GameState *gs);
void takeTurn(GameState *gs, int direction);

#endif
