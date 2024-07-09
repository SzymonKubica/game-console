#include "input.h"
#include "game2048.h"
#include "user_interface.h"

/*
TODO items:
Add detection of when the game is won
*/

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
        GameConfiguration config;
        collectGameConfiguration(&config);

        GameState *state =
            initializeGameState(config.grid_size, config.target_max_tile);

        drawGameCanvas(state);
        updateGameGrid(state);

        while (true) {
                Direction dir;
                bool input_registered = false;
                checkJoystickInput(&dir, &input_registered,
                                   (int (*)(unsigned char)) & analogRead);
                checkButtonsInput(&dir, &input_registered,
                                  (int (*)(unsigned char)) & digitalRead);

                if (input_registered) {
                        takeTurn(state, (int)dir);
                        updateGameGrid(state);
                        delay(MOVE_REGISTERED_DELAY);
                }
                delay(INPUT_POLLING_DELAY);

                if (isGameOver(state)) {
                        handleGameOver(state);
                        break;
                }
                if (isGameFinished(state)) {
                        handleGameFinished(state);
                        break;
                }
        }
}

void handleGameOver(GameState *state)
{
        drawGameOver(state);
        waitForInput();
}

void handleGameFinished(GameState *state)
{
        drawGameWon(state);
        waitForInput();
}

void waitForInput()
{
        while (true) {
                Direction dir;
                bool input_registered = false;
                checkJoystickInput(&dir, &input_registered,
                                   (int (*)(unsigned char)) & analogRead);
                checkButtonsInput(&dir, &input_registered,
                                  (int (*)(unsigned char)) & digitalRead);
                if (input_registered) {
                        break;
                }
        }
}

void collectGameConfiguration(GameConfiguration *config)
{
        const int AVAILABLE_OPTIONS = 2;
        const int GRID_SIZES_LEN = 3;
        const int TARGET_MAX_TILES_LEN = 6;

        int available_grid_sizes[] = {3, 4, 5};
        int available_target_max_tiles[] = {128, 256, 512, 1024, 2048, 4096};

        ConfigOption curr_opt = GRID_SIZE;
        int grid_size = 1;
        int game_target = 4;

        config->grid_size = available_grid_sizes[grid_size];
        config->target_max_tile = available_target_max_tiles[game_target];
        config->config_option = curr_opt;

        drawConfigurationMenu(config, config, false);

        while (true) {
                int stick_registered = digitalRead(STICK_BUTTON_PIN);
                Direction dir;
                bool input_registered = false;

                checkJoystickInput(&dir, &input_registered,
                                   (int (*)(unsigned char)) & analogRead);
                checkButtonsInput(&dir, &input_registered,
                                  (int (*)(unsigned char)) & digitalRead);

                if (input_registered) {
                        GameConfiguration old_config;
                        old_config.grid_size = config->grid_size;
                        old_config.target_max_tile = config->target_max_tile;
                        old_config.config_option = config->config_option;
                        switch (dir) {
                        case UP:
                                curr_opt = (ConfigOption)((curr_opt + 1) %
                                                          AVAILABLE_OPTIONS);
                                break;
                        case DOWN:
                                curr_opt = (ConfigOption)((curr_opt - 1) %
                                                          AVAILABLE_OPTIONS);
                                break;
                        case LEFT:
                                if (curr_opt == GRID_SIZE) {
                                        if (grid_size == 0) {
                                                grid_size = GRID_SIZES_LEN - 1;
                                        } else {
                                                grid_size--;
                                        }
                                } else {
                                        if (game_target == 0) {
                                                game_target =
                                                    TARGET_MAX_TILES_LEN - 1;
                                        } else {
                                                game_target--;
                                        }
                                }
                                break;
                        case RIGHT:
                                if (curr_opt == GRID_SIZE) {
                                        grid_size =
                                            (grid_size + 1) % GRID_SIZES_LEN;
                                } else {
                                        game_target = (game_target + 1) %
                                                      TARGET_MAX_TILES_LEN;
                                }
                                break;
                        }

                        config->grid_size = available_grid_sizes[grid_size];
                        config->target_max_tile =
                            available_target_max_tiles[game_target];
                        config->config_option = curr_opt;
                        drawConfigurationMenu(config, &old_config, true);
                        delay(MOVE_REGISTERED_DELAY);
                }
                delay(INPUT_POLLING_DELAY);
        }
}
