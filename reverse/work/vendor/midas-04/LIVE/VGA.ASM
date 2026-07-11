;*      VGA.ASM
;*
;* S2 The Party '94 64kb intro
;* -- VGA hardware manipulation routines
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*


IDEAL
P386
JUMPS

INCLUDE "lang.inc"
INCLUDE "intro.inc"


DATASEG




CODESEG




GLOBAL  vgaSetNC4 : far
GLOBAL  vgaSetC4 : far



;/***************************************************************************\
;*
;* Function:     void vgaSetNC4(void);
;*
;* Description:  Sets up 320x200x256 Non Chain Four mode
;*
;\***************************************************************************/

PROC    vgaSetNC4       FAR

        mov     ax,0013h
        int     10h

        mov     dx,03C8h
        xor     al,al
        out     dx,al                   ; clear the palette to all black
        inc     dx
        mov     cx,768
@@cpal:
        out     dx,al
        dec     cx
        jnz     @@cpal

        mov     dx,03C4h                ; Sequencer
        mov     ax,0604h                ; Memory Mode Register: no chain 4,
        out     dx,ax                   ; no odd/even, ext. mem, graphics

        mov     dx,03D4h                ; CRT Controller
        mov     al,14h                  ; Underline Location Register
	out     dx,al
	inc     dx
        in      al,dx                   ; Disable Double Word Mode
	and     al,0BFh
	out     dx,al

        dec     dx                      ; CRT Controller
        mov     al,17h                  ; Mode Control Register
	out     dx,al
	inc     dx
	in      al,dx
        or      al,40h                  ; Enable Byte Mode
	out     dx,al

	ret
ENDP



;/***************************************************************************\
;*
;* Function:     void vgaSetC4(void);
;*
;* Description:  Sets up 320x200x256 Chain Four mode
;*
;\***************************************************************************/

PROC    vgaSetC4        FAR

        mov     ax,0013h
        int     10h

        mov     dx,03C8h
        xor     al,al
        out     dx,al                   ; clear the palette to all black
        inc     dx
        mov     cx,768
@@cpal:
        out     dx,al
        dec     cx
        jnz     @@cpal

        ret
ENDP



END
