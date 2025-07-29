#include "emulator_config.h"

#include "../src/common/platform/interface/platform.hpp"
#include "../src/common/platform/sfml/sfml_display.hpp"
#include "../src/common/platform/sfml/emulator_delay.cpp"
#include "../src/common/platform/sfml/sfml_controller.hpp"
#include "../src/common/platform/sfml/sfml_awsd_controller.hpp"
#include "../src/common/platform/sfml/sfml_action_controller.hpp"

#include "../src/common/logging.hpp"

#include "../src/games/game_menu.hpp"
#include <iostream>

// TODO: clean up the mainteinance of the size constants
#define DISPLAY_HEIGHT 240
#define DISPLAY_WIDTH 280

#define TAG "emulator_entrypoint"

SfmlDisplay *display;
EmulatorDelay delay;
SfmlInputController controller;
SfmlAwsdInputController awsd_controller;
SfmlActionInputController action_controller;

void print_version(char *argv[]);
int main(int argc, char *argv[])
{
        print_version(argv);

        LOG_DEBUG(TAG, "Emulator enabled!");

        sf::RenderWindow window(sf::VideoMode({DISPLAY_WIDTH, DISPLAY_HEIGHT}),
                                "game-console-emulator");

        // The problem with simply rendering to the window is that we would need
        // to redraw everything every frame. This is not the behaviour we want
        // as the arduino display doesn't work this way. In Arduino lcd, once
        // we draw something, it stays there until something is drawn on top of
        // it. We achieve this behaviour by using a RenderTexture. This texture
        // is then written into by the game engine and stores the drawn shapes
        // until something is drawn on top of it.
        sf::RenderTexture texture({DISPLAY_WIDTH, DISPLAY_HEIGHT});

        LOG_DEBUG(TAG, "Window rendered!");

        LOG_DEBUG(TAG, "Initializing the display...");
        display = new SfmlDisplay(&window, &texture);
        display->setup();
        LOG_DEBUG(TAG, "Display initialized!");

        controller = SfmlInputController{};
        awsd_controller = SfmlAwsdInputController{};
        action_controller = SfmlActionInputController{};

        std::vector<DirectionalController *> controllers(2);
        controllers[0] = &controller;
        controllers[1] = &awsd_controller;

        std::vector<ActionController *> action_controllers(1);
        action_controllers[0] = &action_controller;

        Platform platform = {.display = display,
                             .directional_controllers = &controllers,
                             .action_controllers = &action_controllers,
                             .delay_provider = &delay};

        while (window.isOpen()) {
                LOG_DEBUG(TAG, "Entering game loop...");
                // We need to loop forever here as the game loop exits when the
                // game is over.
                while (true) {
                        try {
                                select_game(&platform);
                        } catch (std::runtime_error &e) {
                                LOG_DEBUG(TAG, "Game loop exited: %s",
                                          e.what());
                                break;
                        }
                }
        }
}

void print_version(char *argv[])
{
        std::cout << argv[0] << "Version: " << EMULATOR_VERSION_MAJOR << "."
                  << EMULATOR_VERSION_MINOR << std::endl;
}
