# CP/M TPA

CP/M Transient Program Area - Test Programs

- `bell`:
  Just rings the bell once.
- `keyboard_bell`:
  Ring the bell every time a keyboard button was pressed.
- `ncurses_test`:
  Some tests for ncurses functions (color, cursor movement, deletion, insertion). Display all 8 colors in a grid to show all possible combinations of foreground and background colors.
  Partly copied from [pasta80 examples][pasta80-examples].

## Pascal

Each program is written in Pascal and can be compiled with [pasta80](https://github.com/pleumann/pasta80) to z80 executable.

## Assembly

Following programs are also implemented in plain z80 assembly:

- `bell`
- `keyboard_bell`

These can be compiled with the help of [sjasmplus](https://github.com/z00m128/sjasmplus):\
`sjasmplus --raw=./bell_asm.com bell_asm.z80`

[pasta80-examples]: https://github.com/pleumann/pasta80/tree/master/examples
