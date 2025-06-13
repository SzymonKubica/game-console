- [_] fix segmentation faults in the emulator

- [_] finish the generic code for rendering the UI and replace the existing 2048 UI with it
- [_] figure out how to generate sudoku
- [_] design the input model for sudoku

- [_] document the gdb workflow
- [_] figure out how to use the graphical gdb showing code snippets
- [_] maybe using gdb in neovim?
- [_] figure out why the sketch suddenly takes 80% arduino mem (not good - we won't have much space for more games).
- [_] find a good way for logging (preferably not compiled in when running on
      Arduino -> including the entire cstdlib is probably the root cause of the
      build image being so large)
- [_] figure out why manual malloc-ing of structs causes segfaults but doing the
      same thing with classes is somehow fine


- [x] move the the 2028 game rendering code to the 2048 files and make it use the
     display interface
