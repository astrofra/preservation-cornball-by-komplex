;*      DIVERR.ASM
;*
;* MIDAS Sound System DEBUG version divide error handler
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*


P386
IDEAL
JUMPS


INCLUDE "lang.inc"
INCLUDE "errors.inc"
INCLUDE "diverr.inc"
INCLUDE "vgatext.inc"
INCLUDE "timer.inc"



DATASEG

oldInt8         DD      ?               ; old timer interrupt handler
oldInt0         DD      ?               ; old divide error handler



IDATASEG

deMessage       DB      "Divide error (divide by zero or divide overflow) "
                DB      "detected",0
deTimer         DB      "in timer interrupt handler",0
deMessage2      DB      "Please report this bug, including ALL information "
                DB      "below.",0
deAddress       DB      "Address: 0000:0000",0
deSegAddr       DB      "DIVERR_TEXT = 0000",0
deVersion       DB      "Version: ", ??date, ", ", ??time, 0
deHalted        DB      "DOS timer handler restored and system halted - "
                DB      "flush disk caches and reset",0


CODESEG



;/***************************************************************************\
;*
;* Function:    void deInit(void);
;*
;* Description: Initializes divide error handler. MUST be called before
;*              MIDAS Sound System is initialized.
;*
;\***************************************************************************/

PROC    deInit          FAR

        ; Get old divide error exception handler address and store it in
        ; oldInt0:
        mov     ax,3500h
        int     21h
        mov     [word oldInt0],bx
        mov     [word oldInt0+2],es

        ; Get old timer interrupt handler address and store it in oldInt8:
        mov     ax,3508h
        int     21h
        mov     [word oldInt8],bx
        mov     [word oldInt8+2],es

        ; Set new divide error exception handler:
        push    ds
        mov     ax,seg deHandler
        mov     ds,ax
        mov     dx,offset deHandler
        mov     ax,2500h
        int     21h
        pop     ds

        ret
ENDP




;/***************************************************************************\
;*
;* Function:    void deClose(void);
;*
;* Description: Removes divide error handler. MUST be called after MIDAS
;*              Sound System is uninitialized.
;*
;\***************************************************************************/

PROC    deClose         FAR

        ; Return old divide error exception handler:
        push    ds
        mov     ax,2500h
        lds     dx,[oldInt0]
        int     21h
        pop     ds

        ret
ENDP




;/***************************************************************************\
;*
;* Function:    deHandler
;*
;* Description: Divide error exception handler
;*
;\***************************************************************************/

PROC NOLANGUAGE deHandler       FAR

        cli

        mov     ax,@data                ; point ds to correct data segment
        mov     ds,ax

        ; Restore original timer interrupt handler:
        push    ds
        mov     ax,2508h
        lds     dx,[oldInt8]
        int     21h
        pop     ds

        sti                             ; enable interrupts

        ; Set standard text mode:
        call    vgaSetMode LANG, 03h

        ; Display top message:
        call    vgaWriteStr LANG, 12, 1, seg deMessage offset deMessage, \
                4Ah, 57

        cmp     [tmrActive],1           ; running in timer interrupt?
        jne     @@notmr

        ; In timer interrupt, display "in timer interrupt handler":
        call    vgaWriteStr LANG, 27, 2, seg deTimer offset deTimer, 4Ah, 26

        mov     al,20h                  ; acknowledge interrupt to get timer
        out     20h,al                  ; running again

        mov     al,36h                  ; DOS default timer mode
	out	43h,al
	xor	al,al			; set timer count to 65536 - 18.2Hz
	out	40h,al			; (DOS default)
	out	40h,al

        mov     al,20h
        out     20h,al

        mov     al,36h                  ; DOS default timer mode
	out	43h,al
	xor	al,al			; set timer count to 65536 - 18.2Hz
	out	40h,al			; (DOS default)
	out	40h,al

@@notmr:
        ; Display "Report this bug...":
        call    vgaWriteStr LANG, 12, 4, seg deMessage2 offset deMessage2, \
                0Fh, 56

        ; Display "Address:":
        call    vgaWriteStr LANG, 26, 6, seg deAddress offset deAddress, \
                0Fh, 18

        ; Display "DIVERR_TXT = ":
        call    vgaWriteStr LANG, 26, 7, seg deSegAddr offset deSegAddr, \
                0Fh, 18

        ; Display version information:
        call    vgaWriteStr LANG, 26, 8, seg deVersion offset deVersion, \
                0Fh, 27

        ; Display system halted message:
        call    vgaWriteStr LANG, 3, 11, seg deHalted offset deHalted, 0Fh, 75

        mov     bp,sp                   ; point bp to stack

        ; Get exception segment and display it:
        call    vgaWriteByte LANG, 35, 6, [word ss:bp+3], 0Fh
        call    vgaWriteByte LANG, 37, 6, [word ss:bp+2], 0Fh

        ; Get exception offset and display it:
        call    vgaWriteByte LANG, 40, 6, [word ss:bp+1], 0Fh
        call    vgaWriteByte LANG, 42, 6, [word ss:bp], 0Fh

        ; Display current code segment:
        mov     ax,@code
        shr     ax,8
        call    vgaWriteByte LANG, 40, 7, ax, 0Fh
        mov     ax,@code
        and     ax,0Fh
        call    vgaWriteByte LANG, 42, 7, ax, 0Fh

        ; Halt:
        jmp     short $

        ret
ENDP



END
