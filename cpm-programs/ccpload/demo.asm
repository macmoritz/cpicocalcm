;; Simple Testprogram which loads simple.com
        org 100h

        ; print msg
        ld      de, msg
        ld      c, 9 ; Function 9: print string
        call    5

        ; Clear primary FCB
        ld hl, 05ch ; FCB address
        ld (hl), 0 ; store 0 in hl
        ld de, hl
        inc de
        ld bc, 36 ; length
        ldir

        ; Set filename in primary FCB
        ld hl, file             ; copy from
        ld de, 0x005d           ; copy to
        ld bc, 11               ; copy length; 8 filename + 3 filetype
        ldir

        ld      c, 255 ; Function 255: Load file from primary FCB and execute
        call    5

msg:    db  "Loading simple.com...",13,10,"$"
file:   db  "SIMPLE  COM"
