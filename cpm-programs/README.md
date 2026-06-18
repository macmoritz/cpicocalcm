# CP/M Programs

- `bell`:
  Just rings the bell once.
- `keyboard_bell`:
  Ring the bell every time a keyboard button was pressed.
- `ncurses`:
  Some tests for ncurses functions (color, cursor movement, deletion, insertion). Display all 8 colors in a grid to show all possible combinations of foreground and background colors.
  Partly copied from [pasta80 examples][pasta80-examples].
- `chrtabl`:
  Displays a character table of visible characters (0x32 - 0x128) for the normal and alternative charset

## Pascal

All programs are written in Pascal and can be compiled with [pasta80](https://github.com/pleumann/pasta80) to a z80 executable.

## Assembly

Following programs are also implemented in plain z80 assembly:

- `bell`
- `keyboard_bell`

These can be compiled with the help of [sjasmplus](https://github.com/z00m128/sjasmplus):\
`sjasmplus --raw=./bell_asm.com bell_asm.z80`

## Header File

To create a header file for including an executable in the compiled project, [xxd](https://man.archlinux.org/man/xxd.1) can be used:\
`xxd -i bell_asm.com > bell_asm.h` 

This was used while developing, especially before the SD card functionality was implemented.

[pasta80-examples]: https://github.com/pleumann/pasta80/tree/master/examples
