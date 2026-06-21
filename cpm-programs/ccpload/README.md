# CCPLOAD


## Files
- `ccpload.asm`: Main program
- `demo.asm`: Demo program which uses BDOS function 255 to load `simple.com`
- `simple.asm`: A simple program for testing, just prints a string and waits for any key press before exiting 

## Compile
These programs, especially `ccpload.asm` needs to be compiled with [sjasmplus](https://github.com/z00m128/sjasmplus),
since dedicated features like `RELOCATE_TABLE` are used.
