;*	MTM.INC
;*
;* Multitracker Module Player, v1.04
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
;*	 struct mtmChannel
;*	 -----------------
;* Description:  Multitracker Module Player internal channel structure
;\***************************************************************************/

STRUC	mtmChannel
	note		DB	?
	inst		DB	?
	cmd		DB	?
	info		DB	?
	comp		DB	?

	sample		DB	?
	volume		DB	?

	period		DW	?
	snote		DW	?
	loff		DB	?
	coff		DB	?
	toperi		DW	?
	notepsp 	DB	?
	retrigc 	DB	?

	status		DB	?

	vibcmd		DB	?
	vibpos		DB	?

	trecmd		DB	?
	trepos		DB	?

	volbar		DB	?
	playoff 	DW	?
ENDS


GLOBAL	LANG mtmLoadModule : far
GLOBAL	LANG mtmFreeModule : far
GLOBAL	LANG mtmIdentify : far
GLOBAL	LANG mtmInit : far
GLOBAL	LANG mtmClose : far
GLOBAL	LANG mtmPlayModule : far
GLOBAL	LANG mtmStopModule : far
GLOBAL  LANG mtmSetUpdRateFunct : far
GLOBAL	LANG mtmPlay : far
GLOBAL	LANG mtmSetPosition : far
GLOBAL	LANG mtmGetInformation : far

GLOBAL	mpMTM : ModulePlayer


;/***************************************************************************\
;*	 enum mtmFunctIDs
;*	 ----------------
;* Description:  ID numbers for Multitracker Module Player functions
;\***************************************************************************/

ENUM	mtmFunctIDs \
	ID_mtmIdentify = ID_mtm, \
	ID_mtmInit, \
	ID_mtmClose, \
	ID_mtmLoadModule, \
	ID_mtmFreeModule, \
	ID_mtmPlayModule, \
	ID_mtmStopModule, \
        ID_mtmSetUpdRateFunct, \
	ID_mtmPlay, \
	ID_mtmSetPosition, \
	ID_mtmGetInformation

