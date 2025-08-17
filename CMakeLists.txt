cmake_minimum_required(VERSION 3.10)

project(GameConsoleEmulator
  VERSION 1.0)

# This configures the language version required by the project.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# This provides the dynamic setup where the version from the project() function
# above gets inserted into the emulator_config.h header file.
configure_file(emulator_config.h.in emulator_config.h)

# Ensures that sfml-specific code is included
add_compile_definitions(EMULATOR=1)

# This is supposed to all all sources in the project to be built
file(GLOB_RECURSE SFML_PLATFORM_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/common/platform/emulator/*.cpp)
file(GLOB_RECURSE PLATFORM_DEFS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/common/platform/interface/*.cpp)
file(GLOB COMMON_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/common/*.cpp)
file(GLOB GAME_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/games/*.cpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(game-console-emulator
  ${SFML_PLATFORM_SOURCES}
  ${PLATFORM_DEFS}
  ${COMMON_SOURCES}
  ${GAME_SOURCES}
  emulator/emulator_entrypoint.cpp)

target_include_directories(game-console-emulator PUBLIC
  "${PROJECT_BINARY_DIR}"
)

# Set up SFML dependency
include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)
target_link_libraries(game-console-emulator PRIVATE SFML::Graphics)


