        ORG $0100
        ; This is a simple program that demonstrates how to write a relocatable assembly program.
program_start:

        MACRO RUNORIGBDOS
                ld      hl, .macroReturn
                push    hl
                ld      hl, (origBdos)  ; get original bdos function pointer
                jp      (hl)            ; call the original BDOS function to read file
.macroReturn
        ENDM

DEST_END        equ 0fedeh ; end of transient program area
DEST_START      equ DEST_END - RELOCATE_SIZE + 1 ; start of the relocated code in TPA
OFFSET          equ DEST_START - code_to_relocate ; offset to add to addresses in the code that need to be adjusted during relocation

; tnylpo docs: "the default FCB at 0x005c is initialized according to the second and third positional arguments"
; thus the temporary buffer fcbNameType can be initialized by copying from 0x005d
        ld hl, 0x005d           ; copy from
        ld de, fcbNameType      ; copy to
        ld bc, 11               ; copy length; 8 filename + 3 filetype
        ldir

; clear 0x080, tnylpo puts arguments there
        ld      hl, 080h ; FCB address
        ld      (hl), 0  ; clear the first byte of FCB
        ld      de, hl   ; copy target
        inc     de
        ld      bc, 10 ; size to clear, excluding the byte we just cleared
        ldir

; copy BIOS jump table
        ld hl, (0001h)             ; copy from: default BIOS jump table address
        ld de, replicatedBiosTable ; copy to: address of the jump table
        ld bc, 15 * 3              ; copy length: 15 functions, each function pointer is 3 bytes (JP instruction)
        ldir 

; Relocate part of the program to the end of transient program area
; Use reverse copy (LDDR) to have the new location in the register to directly jump to it
        ld      hl, end_of_relocation
        dec     hl
        ld      de, DEST_END
        ld      bc, RELOCATE_SIZE
        lddr
        push    de ; save the new location of the code on the stack

; adjust addresses in the relocated code
        ld      b, relocate_count
        ld      de, DEST_START
        ld      ix, relocation_table
adjust_loop:
        ld      hl, (ix)
        add     hl, de ; get the next address to adjust
        inc     ix
        inc     ix

        ; lower byte adjustment
        ld      a, (hl) ; read the byte at that address
        add     a, low OFFSET
        ld      (hl), a ; write the adjusted byte back
        inc     hl
        ; higher byte adjustment
        ld      a, (hl) ; read the next byte at that address
        adc     a, high OFFSET
        ld      (hl), a ; write the adjusted byte back
        inc     hl
        djnz     adjust_loop

; after relocating and adjusting the code, execute it
        pop     de ; get the new location of the code from the stack
        inc     de
        ex      de, hl
        jp      (hl)

code_to_relocate:
        RELOCATE_START

        ; temporarily clear the TPA to make sure relocation works
        ld      hl, 0100h ; TPA start
        ld      (hl), 76h ; clear the first byte of TPA
        ld      de, hl    ; copy target
        inc     de
        ld      bc, TOTAL_SIZE - 1 ; size to clear, excluding the byte we just cleared
        ldir

        ; store original WBOOT vector and bdos function pointer
        ld hl, (0001h)
        ld (origBios), hl
        ld hl, (0006h)
        ld (origBdos), hl
        ; store (entryWBoot) at 0x0000 to trap SYSTEM RESET
        ld a, 0c3h
        ld (0000h), a
        ld hl, trapBios
        ld (0001h), hl
        ; store (trapBdos) at 0x0005 to trap SYSTEM RESET
        ld a, 0c3h
        ld (0005h), a
        ld hl, trapBdos
        ld (0006h), hl

; copy the saved FCB to the primary FCB, load this file to 0x0100 and execute it
copyFcbNameTypeAndJp:
        ; set all addresses of FCB to zero
        ld hl, 05ch ; FCB address
        ld (hl), 0 ; store 0 in hl
        ld de, hl
        inc de
        ld bc, 36 ; length
        ldir

        ; copy fcbNameType to primary FCB
        ld hl, fcbNameType      ; copy from
        ld de, 0x005d           ; copy to
        ld bc, 11               ; copy length; 8 filename + 3 filetype
        ldir

loadFile:
        ; use a safe stack location
        pop hl ; get the return address from the stack
        ld sp, DEST_START-6
        push hl ; push the return address back to the stack

        ld de, 0005ch   ; primary FCB at 0x005c
        ld c, 0fh       ; open file
        RUNORIGBDOS
        cp 0ffh ; A=0xff -> error
        jp z, loadFileFailed

        ; read file content to 0x0100
        ld hl, 0100h ; TPA start
        ld (dmaAddress), hl
readFileLoop:
        ld de, (dmaAddress)
        ld c, 01ah ; set dma address of de
        RUNORIGBDOS

        ld de, 05ch ; FCB at 0x005c
        ld c, 14h ; read file
        RUNORIGBDOS

        cp 00h ; A=00h -> read 128 bytes successfully
        jr z, readFileOk

        cp 01h ; A=01h -> EOF (end of file)
        jr z, executeFile
        jr loadFileFailed

readFileOk:
        ld hl, (dmaAddress)
        ld de, 128
        add hl, de
        ld (dmaAddress), hl
        jr readFileLoop

executeFile:
        ; close file
        ld      c, 10h          ; close file
        ld      de, 05ch        ; primary FCB at 0x005c
        RUNORIGBDOS

        ; reset disks
        ld      c, 13           ; reset disk system
        RUNORIGBDOS

        ld  de, 0080h
        ld  c, 1Ah              ; Set DMA
        RUNORIGBDOS

        ; clear primary FCB
        ld      hl, 05ch ; FCB address
        ld      (hl), 0  ; clear the first byte of FCB
        ld      de, hl   ; copy target
        inc     de
        ld      bc, 10 ; size to clear, excluding the byte we just cleared
        ldir

        ; clear default DMA area 0x0080
        ld      hl, 080h ; DMA address
        ld      (hl), 0  ; clear the first byte of DMA
        ld      de, hl   ; copy target
        inc     de
        ld      bc, 0fh  ; size to clear, excluding the byte we just cleared
        ldir

        ; push return address on stack, so programs can exit via ret
        ; ld hl, 0000h
        ; push hl
        ld bc, 0000h
        ; jump to 0x100
        ld hl, 0100h
        jp (hl)

loadFileFailed:
        ; ld      de, msgFail
        ; ld      c, 9 ; print string
        ; RUNORIGBDOS
        ret

; BIOS Trap
; replicated and modified BIOS Table
        jp copyFcbNameTypeAndJp ; Function -1: BOOT      System Cold Start Initialization
trapBios:
        jp copyFcbNameTypeAndJp ; Function  0: WBOOT     Warm Start
replicatedBiosTable:
        jp $0000                ; Function  1: CONST     Console Input Status
        jp $0000                ; Function  2: CONIN     Console Character Input
        jp $0000                ; Function  3: CONOUT    Console Character Output
        jp $0000                ; Function  4: LIST      Printer Character Output
        jp $0000                ; Function  5: PUNCH     Paper Tape Punch Output
        jp $0000                ; Function  6: READER    Paper Tape Reader Input
        jp $0000                ; Function  7: HOME      Recalibrate Disk Drive (dummy)
        jp $0000                ; Function  8: SELDSK    Select Disk Drive (dummy)
        jp $0000                ; Function  9: SETTRK    Set Track Number (dummy)
        jp $0000                ; Function 10: SETSEC    Set Sector Number (dummy)
        jp $0000                ; Function 11: SETDMA    Set DMA Address (dummy)
        jp $0000                ; Function 12: READ      Read Selected Sector (dummy)
        jp $0000                ; Function 13: WRITE     Write Selected Sector (dummy)
        jp $0000                ; Function 14: LISTST    Printer Output Status
        jp $0000                ; Function 15: SECTRAN   Sector Number Translation (dummy)

; BDOS Trap
; code to handle bdos function call
trapBdos:
        ; Block function #0 System Reset
        ld a, c
        cp 0
        jr nz, continueBdos ; if not function #0, continue to check for other functions
        ret
continueBdos:
        ; Add function #255 to load file by primary FCB into memory starting at 0x0100
        ld a, c
        cp 255
        jp z, loadFile ; if function #255, jump to loadFile

        RUNORIGBDOS
        ret

origBios:       dw 0 ; store original WBOOT vector
origBdos:       dw 0 ; store original bdos function pointer
dmaAddress:     dw 0 ; store DMA Address for file writing
fcbNameType:    db 0,0,0,0,0,0,0,0,0,0,0 ; store FCB filename and filetype, 11 bytes

; msgFail:        db  "Loading file failed.",13,10,"$"
        RELOCATE_END

relocation_table:
        RELOCATE_TABLE code_to_relocate 
        ; add a list of addresses that need to be adjusted during relocation
        ; these are the addresses of the instructions in the code that reference memory addresses 
        ; (e.g., the addresses of the messages to print)
        ; these addresses are relative to `code_to_relocate`, to make relocation calculations easier

end_of_relocation:
RELOCATE_SIZE   equ $ - code_to_relocate        ; size of code to relocate
TOTAL_SIZE      equ $ - program_start           ; total size of the program

