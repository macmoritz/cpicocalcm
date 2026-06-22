# Original CP/M CCP

This is the source code for the original CCP (Console Command Processor) for CP/M.

It was found on ["The Unofficial CP/M Web site"](http://www.cpm.z80.de/source.html) under the name "CP/M 2.2 ORIGINAL SOURCE".

## Building

- `tnylpo asm os2ccp`: Assemble the source code by using the original Digital Research assembler to produce a hex file.
- `sed -i '$ d' os2ccp.hex`: Remove the last line of the hex file, which contains some garbage.
- `hex2bin.py os2ccp.hex os2ccp.com`: Convert the hex file to a binary COM file that can be executed on CP/M. ([hex2bin source](https://github.com/python-intelhex/intelhex/blob/master/intelhex/scripts/hex2bin.py))
- `tnylpo ccpload.com os2ccp.com`: Run the CCP in the tnylpo emulator by using ccpload for handling the file loading.

## Changes

The original CCP source code (`origccp.asm`) was modified to run on the tnylpo emulator (`os2ccp.asm`). The changes include:

- Changing the file `os2ccp.asm` to DOS line endings (CRLF) to be compatible with the assembler.
- Adjusting the base address to 0x0100 to match the default TPA (Transient Program Area) Address: `org 0100h`.
- Replacing `jmp bdos` with `call bdos` to ensure proper return to the CCP after executing a command.
- Reducing the output of the `DIR` command to two columns to fit the reduced screen width of the PicoCalc.
- Loading transient files by using the BDOS (Basic Disk Operating System) function 255 provided by `ccpload`.
