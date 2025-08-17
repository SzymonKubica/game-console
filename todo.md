# Ideas
- games that can be implemented with the limited display:
  - snake
  - sudoku (assuming we can fit 9x9 grid)
  - some fun animations
  into my desk at home / control the game via ssh.
- [_] figure out how to generate sudoku
- [_] design the input model for sudoku
- [_] think about the sudoku game logic
- we can add compatibility layer for raspberry pi and make a controller that will be embedded

- game of life based puzzle game:
  - you have a limited number of seeds
  - you need to clear a given pattern on the grid
  - the pattern is constructed of cells that become cleared whenever at least
    one game of life cell lives there for at least one generation
  - you can pause the evolution and plant a seed whenever you want
  - the fewer evolution generations and seeds you use the better

- test whether drawing a single large rectangle is faster than drawing multiple
  small squares. If that is the case, we can optimize game of life rendering by
  looking at contiguous regions of blocks that need to be painted black / white
  and do those using a small number of draw_rectangle commands instead of drawing
  each square separately.

# TODO
- [_] add ability to scroll through the config menu for games that require more
      than 3 config options
- [_] add proper customization passing (not just extra parameter for accent color)
- [_] save high score / default configs in some arduino persistent memory (EEPROM is for that and 8kB
  should be plenty.) Ensure that the config / scores are only written if the user
  explicitly says that the write has to happen. EEPROM has a finite number of write
  cycles so we should not try to write to it in some tight loop.
- [_] define persistent memory interface (first step with templates and then try to add the concept from cpp 20)
- [_] implement emulated persistent memory based on local files
- [_] make the game of life random grid population truly random (currently it looks
      like the same pattern every time) (The idea is to mess with the seed on input and save
      it in persitent memory)
- [_] add high score saving to 2048.


# In Progress

# Done
- [x] further optimise game of life implementation to remove memory issues no matter
      how complicated the diff
- [x] fix / find memory leaks in the game of life code / optimize internal representation
      to avoid memory issues. Current hypothesis is that there are no memory leaks.
      The reason is that if we decrease the rewind ring buffer size to 0, the
      game on target device no longer freezes because of memory issues. Also, when
      running the emulator the memory footprint of the console is 148Mb and stays
      like that even if I use the rewind feature.
      Update: there is no memory leaks for sure, the game can handle a rewind buffer
      of 3 evolutions just fine and does not freeze.
- [x] add rewind functionality for game of life
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
