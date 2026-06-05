# CPicoCalc / M
CP/M for PicoCalc


# Project Sources
`tnylpo/`: https://gitlab.com/gbrein/tnypo  - Included as a Git submodule  
`fatfs/`: https://elm-chan.org/fsw/ff/  - Included manually, R0.16, Jul 22, 2025

# Configuration File
The configuration file `.cpicocalcm.conf` is read at start if it exists in the root of the SD-Card.  
The format is inspired by `tnylpo`s config, but simplified:  
Empty lines and lines starting with a hash sign `#` or a semicolon `;` are ignored.  
All other lines have the form `<keyword> = <token>` 
where `<keyword>` is a lowercase string and `<token>` is a string in double quotes.

## Configuration options
- `command = "<path>"` Path to executable file which is run on start. This should be a CCP (Command Console Processor) in most cases.
- `tnylpo config = ".tnylpo.conf"`: Path to `tnylpo` config, defaults to `.tnylpo.conf`.