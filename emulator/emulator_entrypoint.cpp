#include "emulator_config.h"

#include "../src/common/platform/sfml/sfml_display.hpp"
#include "../src/common/platform/sfml/emulator_delay.cpp"
#include "../src/common/platform/sfml/sfml_controller.hpp"
#include "../src/common/platform/sfml/sfml_awsd_controller.hpp"

#include "../src/common/logging.hpp"

#include "../src/games/2048.h"
#include <iostream>

// TODO: clean up the mainteinance of the size constants
#define DISPLAY_HEIGHT 240
#define DISPLAY_WIDTH 280

#define TAG "emulator_entrypoint"

SfmlDisplay *display;
EmulatorDelay delay;
SfmlInputController controller;
SfmlAwsdInputController awsd_controller;

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

        Controller *controllers[] = {&controller, &awsd_controller};
        int controllers_num = sizeof(controllers) / sizeof(Controller *);

        while (window.isOpen()) {
                LOG_DEBUG(TAG, "Entering game loop...");
                // We need to loop forever here as the game loop exits when the
                // game is over.
                while (true) {
                        enter_game_loop(display, controllers, controllers_num,
                                        &delay);
                }
        }
}

void print_version(char *argv[])
{
        std::cout << argv[0] << "Version: " << EMULATOR_VERSION_MAJOR << "."
                  << EMULATOR_VERSION_MINOR << std::endl;
}
