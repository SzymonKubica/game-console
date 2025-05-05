#include "src/common/input.h"
#include "src/common/configuration.hpp"
#include "src/common/user_interface.h"
#include "src/common/display/display.hpp"
#include "src/common/display/lcd_display.hpp"

#include "src/games/2048.h"

#define INPUT_POLLING_DELAY 50
#define MOVE_REGISTERED_DELAY 150

void setup(void)
{
        // Initialise serial port for debugging
        Serial.begin(115200);

        // Set up button pins as inputs
        pinMode(LEFT_BUTTON_PIN, INPUT);
        pinMode(DOWN_BUTTON_PIN, INPUT);
        pinMode(UP_BUTTON_PIN, INPUT);
        pinMode(RIGHT_BUTTON_PIN, INPUT);
        pinMode(STICK_BUTTON_PIN, INPUT);

        initializeDisplay();

        // Initializes the source of randomness from the
        // noise present on the first digital pin
        initializeRandomnessSeed(analogRead(0));
}

void loop(void)
{
        LcdDisplay display = LcdDisplay{};
        // TODO: step through the code and ensure that the main game loop works
        // as expected.
        GameConfiguration config;
        collectGameConfiguration(&display, &config);


        GameState *state =
            initializeGameState(config.grid_size, config.target_max_tile);

        drawGameCanvas(&display, state);
        update_game_grid(&display, state);

        while (true) {
                Direction dir;
                bool input_registered = false;
                checkJoystickInput(&dir, &input_registered,
                                   (int (*)(unsigned char))&analogRead);
                checkButtonsInput(&dir, &input_registered,
                                  (int (*)(unsigned char))&digitalRead);

                if (input_registered) {
                        takeTurn(state, (int)dir);
                        update_game_grid(&display, state);
                        delay(MOVE_REGISTERED_DELAY);
                }
                delay(INPUT_POLLING_DELAY);

                if (isGameOver(state)) {
                        handleGameOver(&display, state);
                        break;
                }
                if (isGameFinished(state)) {
                        handleGameFinished(&display, state);
                        break;
                }
        }
}

void handleGameOver(Display *display, GameState *state)
{
        drawGameOver(display, state);
        waitForInput();
}

void handleGameFinished(Display *display,GameState *state)
{
        drawGameWon(display, state);
        waitForInput();
}

void waitForInput()
{
        while (true) {
                Direction dir;
                bool input_registered = false;
                checkJoystickInput(&dir, &input_registered,
                                   (int (*)(unsigned char))&analogRead);
                checkButtonsInput(&dir, &input_registered,
                                  (int (*)(unsigned char))&digitalRead);
                if (input_registered) {
                        break;
                }
        }
}

void collectGameConfiguration(Display *display, GameConfiguration *config)
{
        const int AVAILABLE_OPTIONS = 3;
        const int GRID_SIZES_LEN = 3;
        const int TARGET_MAX_TILES_LEN = 6;

        int available_grid_sizes[] = {3, 4, 5};
        int available_target_max_tiles[] = {128, 256, 512, 1024, 2048, 4096};

        ConfigOption curr_opt_idx = GRID_SIZE;
        int grid_size_idx = 1;
        int game_target_idx = 4;

        config->grid_size = available_grid_sizes[grid_size_idx];
        config->target_max_tile = available_target_max_tiles[game_target_idx];
        config->config_option = curr_opt_idx;

        drawConfigurationMenu(display, config, config, false);

        while (true) {
                Direction dir;
                bool input_registered = false;

                checkJoystickInput(&dir, &input_registered,
                                   (int (*)(unsigned char))&analogRead);
                checkButtonsInput(&dir, &input_registered,
                                  (int (*)(unsigned char))&digitalRead);
                bool ready = false;
                if (input_registered) {
                        GameConfiguration old_config;
                        old_config.grid_size = config->grid_size;
                        old_config.target_max_tile = config->target_max_tile;
                        old_config.config_option = config->config_option;
                        switch (dir) {
                        case DOWN:
                                curr_opt_idx = (ConfigOption)((curr_opt_idx + 1) %
                                                          AVAILABLE_OPTIONS);
                                break;
                        case UP:
                                curr_opt_idx = (ConfigOption)((curr_opt_idx - 1) %
                                                          AVAILABLE_OPTIONS);
                                break;
                        case LEFT:
                                if (curr_opt_idx == GRID_SIZE) {
                                        if (grid_size_idx == 0) {
                                                grid_size_idx = GRID_SIZES_LEN - 1;
                                        } else {
                                                grid_size_idx--;
                                        }
                                } else {
                                        if (game_target_idx == 0) {
                                                game_target_idx =
                                                    TARGET_MAX_TILES_LEN - 1;
                                        } else {
                                                game_target_idx--;
                                        }
                                }
                                break;
                        case RIGHT:
                                if (curr_opt_idx == GRID_SIZE) {
                                        grid_size_idx =
                                            (grid_size_idx + 1) % GRID_SIZES_LEN;
                                } else if (curr_opt_idx == TARGET_MAX_TILE) {
                                        game_target_idx = (game_target_idx + 1) %
                                                      TARGET_MAX_TILES_LEN;
                                } else {
                                        ready = true;
                                }
                                break;
                        }

                        config->grid_size = available_grid_sizes[grid_size_idx];
                        config->target_max_tile =
                            available_target_max_tiles[game_target_idx];
                        config->config_option = curr_opt_idx;
                        drawConfigurationMenu(display, config, &old_config, true);
                        delay(MOVE_REGISTERED_DELAY);
                        if (ready) {
                                break;
                        }
                }
                delay(INPUT_POLLING_DELAY);
        }
}

// This is supposed to be the generic function used for collecting the generic
// input. In the final state the `collectGameConfiguration` is to be replaced by this.
void collectGenericConfig(Configuration *config)
{
        LcdDisplay display = LcdDisplay{};
        // We start with an empty diff object
        ConfigurationDiff diff;
        renderGenericConfigMenu(&display, config, &diff, false);

        while (true) {
                Direction dir;
                bool input_registered = false;

                checkJoystickInput(&dir, &input_registered,
                                   (int (*)(unsigned char))&analogRead);
                checkButtonsInput(&dir, &input_registered,
                                  (int (*)(unsigned char))&digitalRead);

                bool ready = false;
                if (input_registered) {
                        Configuration old_config;
                        switch (dir) {
                        case DOWN:
                                switchEditedConfigOptionUp(config, &diff);
                                break;
                        case UP:
                                switchEditedConfigOptionDown(config, &diff);
                                break;
                        case LEFT:
                                switchCurrentConfigOptionDown(config, &diff);
                                break;
                        case RIGHT:
                                if (config->current_config_value ==
                                    config->config_values_len - 1) {
                                        ready = true;
                                } else {
                                        switchCurrentConfigOptionUp(config,
                                                                    &diff);
                                }
                                break;
                        }

                        renderGenericConfigMenu(&display, config, &diff, true);
                        delay(MOVE_REGISTERED_DELAY);
                        if (ready) {
                                break;
                        }
                }
                delay(INPUT_POLLING_DELAY);
        }
}
