;*      MUSIC.ASM
;*
;* S2 The Party '94 64kb intro
;* -- Music
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



CODESEG


GLOBAL  music : byte

LABEL   music           BYTE
INCLUDE "module.asm"


END
