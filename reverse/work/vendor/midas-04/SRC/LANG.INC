;*	LANG.INC
;*
;* Language-dependent macros and conditional assembly
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*


IFDEF __BPREAL__
        ; Borland Pascal 7 in Real Mode

	LANG EQU PASCAL

	MODEL	TPASCAL

	MACRO IDATASEG
		CODESEG
	ENDM

        __PASCAL__ = 1
        __REALMODE__ = 1
        __16__ = 1

ELSE
IFDEF __BPPROT__
        ; Borland Pascal 7 in Protected Mode

        LANG EQU PASCAL

        MODEL   TPASCAL

        MACRO IDATASEG
                CODESEG
        ENDM

        __PASCAL__ = 1
        __PROTMODE__ = 1
        __16__ = 1
        __DPMI__ = 1
        __IRQPASSUP__ = 1
        IFNDEF NOEMS
        NOEMS = 1
        ENDIF
ELSE
IFDEF __BC16__
        ; Borland C in 16-bit Real Mode

        ; a hack to force 16-byte segment alignment:

        MACRO MKSEG __name_, __text_
                SEGMENT &__name_&__text_ PARA PUBLIC USE16 "CODE"
                ASSUME  cs:&__name_&__text_, ds:@data
        ENDM
        MKSEG %@FileName, _TEXT
        ENDS

	MODEL	LARGE,C

	LANG EQU C

	MACRO IDATASEG
		DATASEG
	ENDM

        __C__ = 1
        __REALMODE__ = 1
        __16__ = 1
ELSE
IFDEF __WC16__
        ; Watcom C in 16-bit Real Mode

        ; a hack to force 16-byte segment alignment:

        MACRO MKSEG __name_, __text_
                SEGMENT &__name_&__text_ PARA PUBLIC USE16 "CODE"
                ASSUME  cs:&__name_&__text_, ds:@data
        ENDM
        MKSEG %@FileName, _TEXT
        ENDS

	MODEL	LARGE,C

	LANG EQU C

	MACRO IDATASEG
		DATASEG
	ENDM

        __C__ = 1
        __REALMODE__ = 1
        __16__ = 1
ELSE
IFDEF __ASM16__
        ; Pure assembler in 16-bit real mode

        ; Assembler version uses MODEL C, and defines C as the language.

        ; a hack to force 16-byte segment alignment:

        MACRO MKSEG __name_, __text_
                SEGMENT &__name_&__text_ PARA PUBLIC USE16 "CODE"
                ASSUME  cs:&__name_&__text_, ds:@data
        ENDM
        MKSEG %@FileName, _TEXT
        ENDS

	MODEL	LARGE,C

	LANG EQU C

	MACRO IDATASEG
		DATASEG
	ENDM

        __ASM__ = 1
        __REALMODE__ = 1
        __16__ = 1
ELSE
        ERR Destination language not defined
ENDIF
ENDIF
ENDIF
ENDIF
ENDIF
