# CPicoCalc / M
CP/M for PicoCalc

This projects ports the CP/M emulator [tnylpo](#project-sources) from UNIX to the Raspberry Pi Pico microcontroller-family in the [PicoCalc](picocalc) device. 

## PicoCalc
This software is build for the [PicoCalc](picocalc) from Clockwork Tech LLC.
It is an embedded device powered by the Raspberry Pi Pico family, providing a keyboard, a 320x320 pixel LCD screen, a SD card slot and sound output.

The software runs on Raspberry Pi Pico (RP2040) and Pico 2 (RP2350).

| Microcontroller | Processor Architecture | Clock Speed in MHz | Clock Speed of Emulated Z80 CPU in MHz | Frames per Second | 
|---|---|---|---|---|
| RP2350 | Arm Cortex-M33 | 150 | ~ 6.1 | ~ 41 |
| RP2350 | Hazard RISC-V | 150 | ~ 6.5 | ~ 41  |
| RP2040 | Arm Cortex-M0+ | 125 | ~ 4 | ~ 35 |
| RP2040 | Arm Cortex-M0+ | 200 | ~ 6.1 | ~ 55 |

The tests were conducted using the *all.pas* and *heap.pas* test programs provided by [PASTA/80](pasta80). The specified clock rate of the emulated Z80 CPU is the arithmetic mean of the measured values.

## Project Sources
`tnylpo/`: https://gitlab.com/gbrein/tnypo - Included as a Git submodule, Version 1.2  
`fatfs/`: https://elm-chan.org/fsw/ff/ - Included manually, R0.16, Jul 22, 2025

## Configuration File
The configuration file `.cpicocalcm.conf` is read at start if it exists in the root of the SD-Card.  
The format is inspired by `tnylpo`s config, but simplified:  
Empty lines and lines starting with a hash sign `#` or a semicolon `;` are ignored.  
All other lines have the form `<keyword> = <token>` 
where `<keyword>` is a lowercase string and `<token>` is a string in double quotes.

### Configuration options
- `command = "<path>"` Path to executable file which is run on start. This should be a CCP (Command Console Processor) in most cases.
- `tnylpo config = ".tnylpo.conf"`: Path to `tnylpo` config, defaults to `.tnylpo.conf`.

[pasta80]: https://github.com/pleumann/pasta80/
[picocalc]: https://www.clockworkpi.com/picocalc
