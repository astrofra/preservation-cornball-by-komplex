;*      MSTARTUP.ASM
;*
;* MIDAS Sound System assembler version startup code. MUST be the first
;* object module linked. Shrinks program memory block to appropriate
;* size to allow DOS memory allocation and calls global function main (far).
;* main should return an error code in al for DOS (0 = success).
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


STACKSIZE = 4096                        ; Modify if larger stack is needed!

; Segment and Group declarations: (Work at least with Borland C 3.1)

SEGMENT mstartup_TEXT   PARA PUBLIC 'CODE'
ENDS
SEGMENT _FARDATA        PARA PUBLIC 'FAR_DATA'
ENDS
SEGMENT _FARBSS         PARA PUBLIC 'FAR_BSS'
ENDS
SEGMENT _OVERLAY_       PARA PUBLIC 'OVRINFO'
ENDS
SEGMENT _1STUB_         PARA PUBLIC 'STUBSEG'
ENDS
SEGMENT _DATA           PARA PUBLIC 'DATA'
ENDS
SEGMENT _INIT_          PARA PUBLIC 'INITDATA'
LABEL   InitStart byte
ENDS
SEGMENT _INITEND_       PARA PUBLIC 'INITDATA'
LABEL   InitEnd byte
ENDS
SEGMENT _EXIT_          PARA PUBLIC 'EXITDATA'
LABEL   ExitStart byte
ENDS
SEGMENT _EXITEND_       PARA PUBLIC 'EXITDATA'
LABEL   ExitEnd byte
ENDS
SEGMENT _CVTSEG         PARA PUBLIC 'DATA'
ENDS
SEGMENT _SCNSEG         PARA PUBLIC 'DATA'
ENDS
SEGMENT _BSS            PARA PUBLIC 'BSS'
ENDS
SEGMENT _BSSEND         PARA PUBLIC 'BSSEND'
ENDS
SEGMENT _STACK          STACK 'STACK'
ENDS



INCLUDE "lang.inc"
INCLUDE "errors.inc"


STACK STACKSIZE




DATASEG


psp             DW      ?               ; Program Segment Prefix
startFail       DB      "MIDAS Sound System startup failure - "
                DB      "heap corrupted?",0
outOfMem        DB      "Startup failure: Not enough memory for program "
                DB      "(impossible!)",0



CODESEG


EXTRN NOLANGUAGE main : far             ; main program start



;/***************************************************************************\
;*
;* Function:    start
;*
;* Description: MIDAS startup code. Must be the entry point of the program.
;*              exits directly to DOS.
;*
;\***************************************************************************/

PROC NOLANGUAGE start

        ; Register contents at the entry to an .EXE program:
        ;
        ; ds & es       PSP
        ; cs:ip         Here
        ; ss:sp         Top of stack

        mov     ax,@data                ; point ds to data segment
        mov     ds,ax
        mov     [psp],es                ; save PSP segment address

        ; Calculate number of paragraphs needed for program:
        mov     ax,ss
        sub     ax,[psp]                ; ax = stack - PSP
        mov     bx,sp
        shr     bx,4                    ; bx = size of stack in paragraphs +
        add     bx,4                    ; 4 (safety)
        add     bx,ax                   ; bx = number of paragraphs needed

        ; Reallocate program memory block:
        mov     ax,4A00h
        int     21h
        jnc     @@ok1

        cmp     ax,8                    ; error occurred - out of memory?
        je      @@outofmem

        ; Not out of memory - probably heap corrupted:
        call    errErrorExit LANG, seg startFail offset startFail

@@outofmem:
        ; Out of memory: (should NEVER happen)
        call    errErrorExit LANG, seg outOfMem offset outOfMem

@@ok1:
        ; Call main program:
        call    far main

        mov     ah,4Ch                  ; exit to DOS
        int     21h
ENDP



END     start
