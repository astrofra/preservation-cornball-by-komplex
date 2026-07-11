;*	MUSIC.ASM
;*
;* Music-2-OBJ file for some of the MM-module examples
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

include "lang.inc"

CODESEG

GLOBAL	music : FAR

LABEL	music	FAR
INCLUDE "module.asm"

ENDS
END

