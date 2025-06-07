cmake_minimum_required(VERSION 3.10)

project(GameConsoleEmulator
  VERSION 1.0)

# This configures the language version required by the project.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(game-console-emulator emulator_entrypoint.cpp)

# This provides the dynamic setup where the version from the project() function
# above gets inserted into the emulator_config.h header file.
configure_file(emulator_config.h.in emulator_config.h)
target_include_directories(game-console-emulator PUBLIC
  "${PROJECT_BINARY_DIR}"
)
