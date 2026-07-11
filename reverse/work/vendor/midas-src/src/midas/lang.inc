;*	lang.inc
;*
;* Language-dependent macros and conditional assembly
;*
;* $Id: lang.inc,v 1.6 1997/07/31 10:56:42 pekangas Exp $
;*
;* Copyright 1996,1997 Housemarque Inc.
;*
;* This file is part of MIDAS Digital Audio System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Digital Audio System license, "license.txt". By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*


; Actually all target systems look the same at assembly level


; A hack to force 16-byte segment alignment:

SEGMENT _TEXT PARA PUBLIC USE32 "CODE"
ENDS
SEGMENT _DATA PARA PUBLIC USE32 "DATA"
ENDS

MODEL   FLAT,C

LANG EQU C

MACRO IDATASEG
	DATASEG
ENDM

__WC32__ = 1
__C__ = 1
__32__ = 1
__DPMI__ = 1
__PROTMODE__ = 1
__IRQPASSUP__ = 1
NOEMS = 1


; Are we building for lite or normal distribution?

IFNDEF M_LITE
	SUPPORTSTREAMS = 1
ENDIF



;/***************************************************************************\
;*      Memory model dependent macros and equates:
;\***************************************************************************/


IFDEF __32__
        ; 32-bit flat model

        ; Prerred register size equates:
        _ax             EQU     eax
        _bx             EQU     ebx
        _cx             EQU     ecx
        _dx             EQU     edx
        _si             EQU     esi
        _di             EQU     edi
        _bp             EQU     ebp
        _sp             EQU     esp

        ; Default pointer combinations:
        _essi           EQU     esi
        _esdi           EQU     edi
        _esbx           EQU     ebx
        _esbp           EQU     ebp
        _fssi           EQU     esi
        _fsdi           EQU     edi
        _fsbx           EQU     ebx
        _fsbp           EQU     ebp
        _gssi           EQU     esi
        _gsdi           EQU     edi
        _gsbx           EQU     ebx
        _gsbp           EQU     ebp

        ; Data types
        _int            EQU     dword
        INTSIZE = 4
        _long           EQU     dword
        LONGSIZE = 4
        _funct          EQU     near
        _defptr         EQU     near
        _ptr            EQU     dword
        PTRSIZE = 4
        _farptr         EQU     pword
        FARPTRSIZE = 6
        _nearptr        EQU     dword
        NEARPTRSIZE = 4

        MACRO   D_int  name
                &name   DD      ?
        ENDM

        MACRO   D_long  name
                &name   DD      ?
        ENDM

        MACRO   D_ptr   name
                &name   DD      ?
        ENDM

        MACRO   D_farptr name
                &name   DP      ?
        ENDM

        MACRO   D_nearptr name
                &name   DD      ?
        ENDM

        ptr_to          EQU     offset
        farptr_to       EQU     far ptr
        nearptr_to      EQU     offset

        ; LOADPTR - loads a default pointer type
        MACRO   LOADPTR         segreg, indexreg, pointer
        mov     indexreg, pointer
        ENDM

        ; COPYPTR - copies a default pointer type variable (destroys eax)
        MACRO   COPYPTR         destptr, sourceptr
        mov     eax,sourceptr
        mov     destptr,eax
        ENDM

        ; PUSHSEGREG - pushes a segment register in 16-bit modes
        MACRO   PUSHSEGREG      segreg
        ENDM

        ; POPSEGREG - pops a segment register in 16-bit modes
        MACRO   POPSEGREG       segreg
        ENDM
ENDIF

IFDEF __16__
        ; All 16-bit models

        ; Prerred register size equates:
        _ax             EQU     ax
        _bx             EQU     bx
        _cx             EQU     cx
        _dx             EQU     dx
        _si             EQU     si
        _di             EQU     di
        _bp             EQU     bp
        _sp             EQU     sp

        ; Default pointer combinations:
        _essi           EQU     es:si
        _esdi           EQU     es:di
        _esbx           EQU     es:bx
        _esbp           EQU     es:bp
        _fssi           EQU     fs:si
        _fsdi           EQU     fs:di
        _fsbx           EQU     fs:bx
        _fsbp           EQU     fs:bp
        _gssi           EQU     gs:si
        _gsdi           EQU     gs:di
        _gsbx           EQU     gs:bx
        _gsbp           EQU     gs:bp

        ; Data types
        _int            EQU     word
        INTSIZE = 2
        _long           EQU     dword
        LONGSIZE = 4
        _funct          EQU     far
        _defptr         EQU     far
        _ptr            EQU     dword
        PTRSIZE = 4
        _farptr         EQU     dword
        FARPTRSIZE = 4
        _nearptr        EQU     word
        NEARPTRSIZE = 2

        MACRO   D_int  name
                &name   DW      ?
        ENDM

        MACRO   D_long name
                &name   DD      ?
        ENDM

        MACRO   D_ptr   name
                &name   DD      ?
        ENDM

        MACRO   D_farptr name
                &name   DD      ?
        ENDM

        MACRO   D_nearptr name
                &name   DW      ?
        ENDM

        ptr_to          EQU     far ptr
        farptr_to       EQU     far ptr
        nearptr_to      EQU     offset

        ; LOADPTR - loads a default pointer type
        MACRO   LOADPTR         segreg, indexreg, pointer
        IFIDNI  <&segreg>,<ds>
                lds     indexreg,pointer
        ELSE
        IFIDNI  <&segreg>,<es>
                les     indexreg,pointer
        ELSE
        IFIDNI  <&segreg>,<fs>
                lfs     indexreg,pointer
        ELSE
        IFIDNI  <&segreg>,<gs>
                lgs     indexreg,pointer
        ENDIF
        ENDIF
        ENDIF
        ENDIF
        ENDM

        ; COPYPTR - copies a default pointer type variable (destroys eax)
        MACRO   COPYPTR         destptr, sourceptr
        mov     eax,sourceptr
        mov     destptr,eax
        ENDM

        ; PUSHSEGREG - pushes a segment register in 16-bit modes
        MACRO   PUSHSEGREG      segreg
        push    segreg
        ENDM

        ; POPSEGREG - pops a segment register in 16-bit modes
        MACRO   POPSEGREG       segreg
        pop     segreg
        ENDM
ENDIF


;* $Log: lang.inc,v $
;* Revision 1.6  1997/07/31 10:56:42  pekangas
;* Renamed from MIDAS Sound System to MIDAS Digital Audio System
;*
;* Revision 1.5  1997/05/03 15:10:01  pekangas
;* Simplified, added stream support for dos
;*
;* Revision 1.4  1997/01/16 18:41:59  pekangas
;* Changed copyright messages to Housemarque
;*
;* Revision 1.3  1996/07/16 20:20:50  pekangas
;* Added support for Visual C
;*
;* Revision 1.2  1996/05/30 22:38:46  pekangas
;* no changes?
;*
;* Revision 1.1  1996/05/22 20:49:33  pekangas
;* Initial revision
;*