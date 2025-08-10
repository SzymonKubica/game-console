# Ideas
- games that can be implemented with the limited display:
  - snake
  - sudoku (assuming we can fit 9x9 grid)
  - some fun animations
- we can add compatibility layer for raspberry pi and make a controller that will be embedded
  into my desk at home / control the game via ssh.

- game of life based puzzle game:
  - you have a limited number of seeds
  - you need to clear a given pattern on the grid
  - the pattern is constructed of cells that become cleared whenever at least
    one game of life cell lives there for at least one generation
  - you can pause the evolution and plant a seed whenever you want
  - the fewer evolution generations and seeds you use the better

# TODO

- [_] figure out how to generate sudoku
- [_] design the input model for sudoku
- [_] think about the sudoku game logic
- [_] add proper customization passing (not just extra parameter for accent color)

- [_] add rewind functionality for game of life
- [_] save high score / default configs in some arduino persistent memory (EEPROM is for that and 8kB
  should be plenty.) Ensure that the config / scores are only written if the user
  explicitly says that the write has to happen. EEPROM has a finite number of write
  cycles so we should not try to write to it in some tight loop.

# In Progress

# Done
- [x] create macropad setting for neovim gdb plugin
- [x] figure out how to use the graphical gdb showing code snippets
- [x] maybe using gdb in neovim?
- [x] document the gdb workflow
- [x] add the font to the project resources to ensure portability
- [x] add proper non-deterministic randomness for the emulated minesweeper
- [x] fix memory leaks in the game menu
- [x] fix memory leaks in the game of life loop
- [x] add proper deallocation function for the config objects
- [x] fix minesweeper rendering
- [x] implement minesweeper
- [x] design the input model for minesweeper
- [x] refactor 2048 game logic
- [x] finish the generic code for rendering the UI and replace the existing 2048 UI with it
- [x] add platform context object to reduce the number of params for functions
      that have need to interact with the controllers and the display.
      (this was partially done on 2025-07-23 by allowing for an arbitrary length
      list of game controllers).
- [x] find out why the game doesn't restart on the emulator
- [x] move the the 2028 game rendering code to the 2048 files and make it use the
      display interface

- [x] figure out why manual malloc-ing of structs causes segfaults but doing the
      same thing with classes is somehow fine.

      This turned out to be a misdiagnosis. The problem was rooted in the incorrect
      initialization of the game grid array and not in the allocation of the internal state structs

- [x] fix segmentation faults in the emulator

- [x] figure out why the sketch suddenly takes 80% arduino mem (not good - we won't have much space for more games).

      This program size issue was caused by the C++ `<iostream>` header being included for in the arduino build.
      This header was added for debugging in the emulator and it turns out that this library is huge.
      Note that, after removing this header, the memory requirement dropped from 82% to 29% which is
      more than enough what we need.

- [x] find a good way for logging (preferably not compiled in when running on
      Arduino -> including the entire cstdlib is probably the root cause of the
      build image being so large)

      The solution here was to borrow the simple logger from the old weather station
      code. Turns out `printf` doesn't require nearly as much space as `<iostream>` and
      so we can use the logging library without conditionally compiling it only for the
      emulated version.
