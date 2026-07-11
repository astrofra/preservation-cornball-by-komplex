;*	MOD.INC
;*
;* ProTracker Module Player, v1.36
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*


;/***************************************************************************\
;*	 struct modInstHdr
;*	 -----------------
;* Description:  Protracker module instrument header. Note that all 16-bit
;*		 fields are big-endian.
;\***************************************************************************/

STRUC	modInstHdr
	iname		DB	22 DUP (?)  ; instrument name
	slength 	DW	?	; sample length
	finetune	DB	?	; sample finetune value
	volume		DB	?	; sample default volume
	loopStart	DW	?	; sample loop start, in words
	loopLength	DW	?	; sample loop length, in words
ENDS



;/***************************************************************************\
;*	 struct modHeader
;*	 ----------------
;* Description:  Protracker module file header
;\***************************************************************************/

STRUC	modHeader
	songName	DB	20 DUP (?)	; song name
	instruments	modInstHdr 31 DUP (?)	; instrument headers */
	songLength	DB	?		; song length
	unused		DB	?		; unused by Protracker, used
						; to be song restart position
	orders		DB	128 DUP (?)	; pattern playing orders
	sign		DB	4 DUP (?)	; module signature
ENDS




;/***************************************************************************\
;*	 struct modChannel
;*	 -----------------
;* Description:  Protracker Module Player internal channel structure
;\***************************************************************************/

STRUC	modChannel
	note		DB	?
	inst		DB	?
	cmd		DB	?
	info		DB	?
        flags           DB      ?

	sample		DB	?
	volume		DB	?

	period		DW	?
	snote		DW	?
	loff		DB	?
	coff		DB	?
        toPeri          DW      ?
        notePSp         DB      ?
        retrigC         DB      ?

	status		DB	?

        vibCmd          DB      ?
        vibPos          DB      ?

        treCmd          DB      ?
        trePos          DB      ?

        volBar          DB      ?

        loopPos         DB      ?
        loopCnt         DB      ?

        playOff         DW      ?
ENDS



GLOBAL	LANG modLoadModule : far
GLOBAL	LANG modFreeModule : far
GLOBAL	LANG modConvertSample : far
GLOBAL  LANG modConvertPattern : far
GLOBAL	LANG modIdentify : far
GLOBAL	LANG modInit : far
GLOBAL	LANG modClose : far
GLOBAL	LANG modPlayModule : far
GLOBAL  LANG modStopModule : far
GLOBAL  LANG modSetUpdRateFunct : far
GLOBAL	LANG modPlay : far
GLOBAL	LANG modSetPosition : far
GLOBAL	LANG modGetInformation : far


GLOBAL	mpMOD : ModulePlayer



;/***************************************************************************\
;*	 enum modFunctIDs
;*	 ----------------
;* Description:  ID numbers for Protracker Module Player functions
;\***************************************************************************/

ENUM	modFunctIDs \
	ID_modIdentify = ID_mod, \
	ID_modInit, \
	ID_modClose, \
	ID_modLoadModule, \
	ID_modFreeModule, \
	ID_modPlayModule, \
	ID_modStopModule, \
        ID_modSetUpdRateFunct, \
        ID_modSetInterrupt, \
	ID_modRemoveInterrupt, \
	ID_modPlay, \
	ID_modSetPosition, \
	ID_modGetInformation, \
	ID_modConvertSample, \
        ID_modConvertPattern
