#include <iostream>
#include "emulator_config.h"

void print_version(char *argv[]);
int main(int argc, char* argv[])
{
  print_version(argv);
  double const inputValue = std::stod(argv[1]);
  std::cout << "Emulator started!" << std::endl;
  std::cout << "Testing C++ features " << inputValue << std::endl;
}

void print_version(char *argv[])
{
  std::cout << argv[0] << "Version: "<< EMULATOR_VERSION_MAJOR << "."
            << EMULATOR_VERSION_MINOR << std::endl;
}
