#include <iostream>
#include "emulator_config.h"
#include "src/common/platform/sfml/sfml_example.hpp"

void print_version(char *argv[]);
int main(int argc, char *argv[])
{
        print_version(argv);
        double const inputValue = std::stod(argv[1]);
        std::cout << "Emulator started!" << std::endl;

        // TODO: figure out how to get user input in SFML
        // keypad_controller =
        //     new KeypadController((int (*)(unsigned char))&digitalRead),
        sfml_main();
}

void print_version(char *argv[])
{
        std::cout << argv[0] << "Version: " << EMULATOR_VERSION_MAJOR << "."
                  << EMULATOR_VERSION_MINOR << std::endl;
}
