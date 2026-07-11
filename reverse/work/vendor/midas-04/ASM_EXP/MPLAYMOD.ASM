;*      MPLAYMOD.ASM
;*
;* Minimal Protracker module player using MIDAS Sound System.
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

INCLUDE "midas.inc"



DATASEG


moduleName      DB      "..\live.mod", 0
module          DD      ?
playing         DB      "Playing - Press any key to stop$"




CODESEG


PUBLIC NOLANGUAGE main



;/***************************************************************************\
;*
;* Function:    main
;*
;* Description: Main program, called by MIDAS startup code
;*
;\***************************************************************************/

PROC NOLANGUAGE main    FAR

        ; Set MIDAS Sound System defaults:
        call    midasSetDefaults LANG

        ; Initialize MIDAS Sound System:
        call    midasInit LANG

        ; Load module file using Protracker module player:
        call    midasLoadModule LANG, seg moduleName offset moduleName, \
                seg mpMOD offset mpMOD, NULL
        mov     [word module],ax
        mov     [word module+2],dx

        ; Start playing module:
        call    midasPlayModule LANG, [module], 0

        ; Write message and wait for keypress:
        mov     dx,offset playing
        mov     ax,0900h
        int     21h
        mov     ax,0800h
        int     21h

        ; Stop playing module:
        call    midasStopModule LANG, [module]

        ; Deallocate module:
        call    midasFreeModule LANG, [module]

        ; Uninitialize MIDAS Sound System:
        call    midasClose LANG

        ; Exit to DOS with return code 0:
        xor     al,al
        ret
ENDP


END
