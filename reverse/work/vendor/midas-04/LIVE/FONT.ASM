;*      FONT.ASM
;*
;* S2 The Party '94 64kb intro
;* -- Font
;*
;* Copyright 1994 Petteri Kangaslampi
;*


IDEAL
P386
JUMPS

INCLUDE "lang.inc"
INCLUDE "intro.inc"



DATASEG


GLOBAL  font : dword


LABEL   font    dword
                DD      far ptr _space                  ; space
                DD      far ptr _excl                   ; !
                DD      far ptr _space                  ; "
                DD      far ptr _space                  ; #
                DD      far ptr _space                  ; $
                DD      far ptr _space                  ; %
                DD      far ptr _space                  ; &
                DD      far ptr _space                  ; '
                DD      far ptr _space                  ; (
                DD      far ptr _space                  ; )
                DD      far ptr _space                  ; *
                DD      far ptr _plus                   ; +
                DD      far ptr _comma                  ; ,
                DD      far ptr _minus                  ; -
                DD      far ptr _period                 ; .
                DD      far ptr _slash                  ; /
                DD      far ptr _0                      ; 0
                DD      far ptr _1                      ; 1
                DD      far ptr _2                      ; 2
                DD      far ptr _3                      ; 3
                DD      far ptr _4                      ; 4
                DD      far ptr _5                      ; 5
                DD      far ptr _6                      ; 6
                DD      far ptr _7                      ; 7
                DD      far ptr _8                      ; 8
                DD      far ptr _9                      ; 9
                DD      far ptr _space                  ; :
                DD      far ptr _space                  ; ;
                DD      far ptr _space                  ; <
                DD      far ptr _space                  ; =
                DD      far ptr _space                  ; >
                DD      far ptr _quest                  ; ?
                DD      far ptr _space                  ; @
                DD      far ptr _a
                DD      far ptr _b
                DD      far ptr _c
                DD      far ptr _d
                DD      far ptr _e
                DD      far ptr _f
                DD      far ptr _g
                DD      far ptr _h
                DD      far ptr _i
                DD      far ptr _j
                DD      far ptr _k
                DD      far ptr _l
                DD      far ptr _m
                DD      far ptr _n
                DD      far ptr _o
                DD      far ptr _p
                DD      far ptr _q
                DD      far ptr _r
                DD      far ptr _s
                DD      far ptr _t
                DD      far ptr _u
                DD      far ptr _v
                DD      far ptr _w
                DD      far ptr _x
                DD      far ptr _y
                DD      far ptr _z


; Letter format:
;               DB      Number of polygons
; For each polygon
;               DB      Number of vertices
; For each vertex
;               DB      coord
;       where coord = 16*x + y
;
; Each letter is 6 units wide and 10 high, the center is at (3,5)


_a              DB      4
                DB      6,      10h, 50h, 61h, 42h, 22h, 01h
                DB      4,      01h, 22h, 2Ah, 0Ah
                DB      4,      24h, 44h, 46h, 26h
                DB      4,      42h, 61h, 6Ah, 4Ah

_b              DB      6
                DB      4,      00h, 40h, 42h, 02h
                DB      6,      40h, 50h, 61h, 64h, 55h, 44h
                DB      5,      24h, 44h, 55h, 46h, 26h
                DB      6,      55h, 66h, 69h, 5Ah, 4Ah, 46h
                DB      4,      08h, 48h, 4Ah, 0Ah
                DB      4,      02h, 22h, 28h, 08h

_c              DB      3
                DB      5,      10h, 60h, 62h, 02h, 01h
                DB      4,      02h, 22h, 28h, 08h
                DB      5,      08h, 68h, 6Ah, 1Ah, 09h

_d              DB      6
                DB      4,      00h, 40h, 32h, 02h
                DB      4,      40h, 62h, 43h, 32h
                DB      4,      43h, 62h, 68h, 47h
                DB      4,      47h, 68h, 4Ah, 38h
                DB      4,      02h, 22h, 28h, 08h
                DB      4,      08h, 38h, 4Ah, 0Ah

_e              DB      5
                DB      4,      00h, 60h, 62h, 02h
                DB      4,      02h, 22h, 24h, 04h
                DB      4,      04h, 44h, 46h, 06h
                DB      4,      06h, 26h, 28h, 08h
                DB      4,      08h, 68h, 6Ah, 0Ah

_f              DB      4
                DB      4,      00h, 60h, 62h, 02h
                DB      4,      02h, 22h, 24h, 04h
                DB      4,      04h, 44h, 46h, 06h
                DB      4,      06h, 26h, 2Ah, 0Ah

_g              DB      5
                DB      5,      10h, 60h, 62h, 02h, 01h
                DB      4,      02h, 22h, 28h, 08h
                DB      5,      08h, 68h, 6Ah, 1Ah, 09h
                DB      4,      44h, 64h, 68h, 48h
                DB      4,      34h, 44h, 46h, 36h

_h              DB      3
                DB      4,      00h, 20h, 2Ah, 0Ah
                DB      4,      24h, 44h, 46h, 26h
                DB      4,      40h, 60h, 6Ah, 4Ah

_i              DB      3
                DB      4,      00h, 60h, 62h, 02h
                DB      4,      22h, 42h, 48h, 28h
                DB      4,      08h, 68h, 6Ah, 0Ah

_j              DB      3
                DB      5,      40h, 60h, 69h, 5Ah, 48h
                DB      4,      28h, 48h, 5Ah, 1Ah
                DB      5,      06h, 26h, 28h, 1Ah, 09h

_k              DB      4
                DB      4,      00h, 20h, 2Ah, 0Ah
                DB      4,      40h, 60h, 44h, 24h
                DB      4,      24h, 44h, 46h, 26h
                DB      4,      26h, 46h, 6Ah, 4Ah

_l              DB      2
                DB      4,      00h, 20h, 2Ah, 0Ah
                DB      4,      28h, 68h, 6Ah, 2Ah

_m              DB      4
                DB      4,      00h, 20h, 2Ah, 0Ah
                DB      4,      20h, 32h, 35h, 23h
                DB      4,      32h, 40h, 43h, 35h
                DB      4,      40h, 60h, 6Ah, 4Ah

_n              DB      3
                DB      4,      00h, 20h, 2Ah, 0Ah
                DB      4,      20h, 44h, 47h, 23h
                DB      4,      40h, 60h, 6Ah, 4Ah

_o              DB      4
                DB      6,      10h, 50h, 61h, 62h, 02h, 01h
                DB      4,      02h, 22h, 28h, 08h
                DB      4,      42h, 62h, 68h, 48h
                DB      6,      08h, 68h, 69h, 5Ah, 1Ah, 09h

_p              DB      4
                DB      5,      00h, 50h, 61h, 62h, 02h
                DB      4,      02h, 22h, 2Ah, 0Ah
                DB      4,      42h, 62h, 64h, 44h
                DB      5,      24h, 64h, 65h, 56h, 26h

_q              DB      5
                DB      6,      10h, 50h, 61h, 62h, 02h, 01h
                DB      4,      02h, 22h, 28h, 08h
                DB      4,      42h, 62h, 68h, 48h
                DB      4,      36h, 46h, 48h, 38h
                DB      5,      08h, 68h, 6Ah, 1Ah, 09h

_r              DB      5
                DB      5,      00h, 50h, 61h, 62h, 02h
                DB      4,      02h, 22h, 2Ah, 0Ah
                DB      4,      42h, 62h, 64h, 44h
                DB      5,      24h, 64h, 65h, 56h, 26h
                DB      4,      26h, 46h, 6Ah, 4Ah

_s              DB      5
                DB      5,      10h, 60h, 62h, 02h, 01h
                DB      4,      02h, 22h, 24h, 04h
                DB      6,      04h, 54h, 65h, 66h, 16h, 05h
                DB      4,      46h, 66h, 68h, 48h
                DB      5,      08h, 68h, 69h, 5Ah, 0Ah

_t              DB      2
                DB      4,      00h, 60h, 62h, 02h
                DB      4,      22h, 42h, 4Ah, 2Ah

_u              DB      3
                DB      4,      00h, 20h, 28h, 08h
                DB      4,      40h, 60h, 68h, 48h
                DB      6,      08h, 68h, 69h, 5Ah, 1Ah, 09h

_v              DB      4
                DB      4,      00h, 20h, 26h, 06h
                DB      4,      40h, 60h, 66h, 46h
                DB      5,      06h, 26h, 38h, 3Ah, 2Ah
                DB      5,      46h, 66h, 4Ah, 3Ah, 38h

_w              DB      4
                DB      4,      00h, 20h, 2Ah, 0Ah
                DB      4,      40h, 60h, 6Ah, 4Ah
                DB      4,      28h, 36h, 38h, 2Ah
                DB      4,      36h, 48h, 4Ah, 38h

_x              DB      6
                DB      5,      00h, 20h, 23h, 15h, 03h
                DB      5,      40h, 60h, 63h, 55h, 43h
                DB      4,      23h, 35h, 27h, 15h
                DB      4,      43h, 55h, 47h, 35h
                DB      5,      15h, 27h, 2Ah, 0Ah, 07h
                DB      5,      55h, 67h, 6Ah, 4Ah, 47h

_y              DB      4
                DB      4,      00h, 20h, 26h, 02h
                DB      4,      40h, 60h, 62h, 46h
                DB      4,      22h, 46h, 4Ah, 2Ah
                DB      3,      42h, 46h, 34h

_z              DB      3
                DB      4,      00h, 60h, 62h, 02h
                DB      4,      42h, 62h, 28h, 08h
                DB      4,      08h, 68h, 6Ah, 0AH

_1              DB      3
                DB      4,      20h, 40h, 42h, 02h
                DB      4,      22h, 42h, 48h, 28h
                DB      4,      08h, 68h, 6Ah, 0Ah

_2              DB      5
                DB      5,      00h, 50h, 61h, 62h, 02h
                DB      4,      42h, 62h, 64h, 44h
                DB      6,      14h, 64h, 65h, 56h, 06h, 05h
                DB      4,      06h, 26h, 28h, 08h
                DB      5,      08h, 68h, 6Ah, 1Ah, 09h

_3              DB      5
                DB      5,      00h, 50h, 61h, 62h, 02h
                DB      4,      42h, 62h, 64h, 44h
                DB      4,      24h, 64h, 46h, 26h
                DB      5,      55h, 66h, 68h, 48h, 46h
                DB      5,      08h, 68h, 69h, 5Ah, 0Ah

_4              DB      3
                DB      4,      00h, 20h, 26h, 06h
                DB      4,      24h, 44h, 46h, 26h
                DB      4,      40h, 60h, 6Ah, 4Ah

_5              DB      5
                DB      4,      00h, 60h, 62h, 02h
                DB      4,      02h, 22h, 24h, 04h
                DB      5,      04h, 54h, 65h, 66h, 06h
                DB      4,      46h, 66h, 68h, 48h
                DB      5,      08h, 68h, 69h, 5Ah, 0Ah

_6              DB      5
                DB      5,      10h, 60h, 62h, 02h, 01h
                DB      4,      02h, 22h, 28h, 08h
                DB      6,      08h, 68h, 69h, 5Ah, 1Ah, 09h
                DB      5,      44h, 54h, 65h, 68h, 48h
                DB      4,      24h, 44h, 46h, 26h

_7              DB      2
                DB      4,      00h, 60h, 62h, 02h
                DB      4,      42h, 62h, 4Ah, 2Ah

_8              DB      7
                DB      6,      10h, 50h, 61h, 62h, 02h, 01h
                DB      5,      02h, 22h, 24h, 15h, 04h
                DB      5,      42h, 62h, 64h, 55h, 44h
                DB      4,      24h, 44h, 66h, 06h
                DB      4,      06h, 26h, 28h, 08h
                DB      4,      46h, 66h, 68h, 48h
                DB      6,      08h, 68h, 69h, 5Ah, 1Ah, 09h

_9              DB      5
                DB      6,      10h, 50h, 61h, 62h, 02h, 01h
                DB      5,      02h, 22h, 26h, 16h, 05h
                DB      4,      24h, 44h, 46h, 26h
                DB      4,      42h, 62h, 68h, 48h
                DB      5,      08h, 68h, 69h, 5Ah, 0Ah

_0              DB      5
                DB      6,      10h, 50h, 61h, 62h, 02h, 01h
                DB      4,      02h, 22h, 28h, 08h
                DB      4,      42h, 62h, 68h, 48h
                DB      6,      08h, 68h, 69h, 5Ah, 1Ah, 09h
                DB      6,      32h, 42h, 44h, 38h, 28h, 26h

_period         DB      1
                DB      4,      28h, 48h, 4Ah, 2Ah

_comma          DB      2
                DB      4,      27h, 47h, 49h, 29h
                DB      4,      29h, 49h, 3Ah, 1Ah

_excl           DB      2
                DB      4,      20h, 40h, 46h, 26h
                DB      4,      28h, 48h, 4Ah, 2Ah

_quest          DB      5
                DB      6,      10h, 50h, 61h, 62h, 02h, 01h
                DB      4,      02h, 22h, 23h, 03h
                DB      4,      42h, 62h, 64h, 44h
                DB      4,      44h, 64h, 46h, 26h
                DB      4,      28h, 48h, 4Ah, 2Ah

_minus          DB      1
                DB      4,      04h, 64h, 66h, 06h

_slash          DB      1
                DB      4,      40h, 60h, 2Ah, 0Ah

_plus           DB      3
                DB      4,      22h, 42h, 44h, 24h
                DB      4,      04h, 64h, 66h, 06h
                DB      4,      26h, 46h, 48h, 28h

_space          DB      0





END
