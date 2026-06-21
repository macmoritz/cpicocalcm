;; Simple Testprogram for testing the functionality
        org 100h

        ; print msg
        ld      de, msg
        ld      c, 9 ; Function 9: print string
        call    5

        ; Read input blocking and echo
        ld      c, 1 ; Function 1: CONIN
        call    5

        pop     hl ; pop return address from stack to prevent corruption   
        ; exit program
        jp      0h ; address 0x0000: WBOOT

msg:    db  "Hello from Testprogram.",13,10,"$"
