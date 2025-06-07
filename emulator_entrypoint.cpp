#include <iostream>
#include "emulator_config.h"
#include "src/common/platform/sfml/sfml_display.hpp"
#include "src/common/platform/sfml/emulator_delay.cpp"
#include "src/common/platform/sfml/sfml_controller.hpp"

#include "src/games/2048.h"

// TODO: clean up the mainteinance of the size constants
#define DISPLAY_HEIGHT 240
#define DISPLAY_WIDTH 280

SfmlDisplay *display;
EmulatorDelay delay;
SfmlInputController controller;

void print_version(char *argv[]);
int main(int argc, char *argv[])
{
        print_version(argv);
        std::cout << "Emulator started!" << std::endl;

        sf::RenderWindow window(sf::VideoMode({DISPLAY_WIDTH, DISPLAY_HEIGHT}),
                                "SFML works!");
        std::cout << "Window rendered!" << std::endl;

        std::cout << "Initializing the display..." << std::endl;
        display = new SfmlDisplay(&window);
        display->setup();
        std::cout << "Display initialized!" << std::endl;

        controller = SfmlInputController{};

        while (window.isOpen()) {
                std::cout << "Entering game loop..." << std::endl;
                enter_game_loop(display, &controller, &controller, &delay);
                while (true) {
                }
        }
}

void print_version(char *argv[])
{
        std::cout << argv[0] << "Version: " << EMULATOR_VERSION_MAJOR << "."
                  << EMULATOR_VERSION_MINOR << std::endl;
}
