;*	MPLAYER.INC
;*
;* Module Player definitions
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*


MPCHANNELS = 32 			; maximum number of Module Player
					; channels


;/***************************************************************************\
;*	 enum modIDNumbers
;*	 -----------------
;* Description:  mpModule ID numbers
;\***************************************************************************/

ENUM	modIDNumbers \
	idS3M = 0, \			; Scream Tracker 3 module
        idMOD = 1, \                    ; Protracker module
        idMTM = 2                       ; Multitracker module



;/***************************************************************************\
;*	 struct mpChanInfo
;*	 -----------------
;* Description:  Module Player channel info structure
;\***************************************************************************/

STRUC	mpChanInfo
	flags		DB	?	; bits 0-4 - UNDEFINED
					; bit 5 - new note and/or instrument
					; bit 6 - new volume (S3M)
					; bit 7 - command
	note		DB	?	; note number
	instrument	DB	?	; instrument number
	volume		DB	?	; playing volume
	command 	DB	?	; command number
	infobyte	DB	?	; command infobyte
	volumebar	DB	?	; "fake" volume bar
	commandname	DD	?	; pointer to command name string,
					; ASCIIZ
ENDS



;/***************************************************************************\
;*	 struct mpInformation
;*	 --------------------
;* Description:  Module Player information structure
;\***************************************************************************/

STRUC	mpInformation
        numChannels     DW      ?       ; number of channels
	setFrame	DW	?	; 1 if "set frame", 0 if not
	speed		DW	?	; playing speed
	BPM		DW	?	; playing BPM tempo
	pos		DW	?	; current playing position
	pattern 	DW	?	; current pattern number
	row		DW	?	; current row number
	loopCnt 	DW	?	; song loop counter
        chans           DD      ?       ; pointer to channel data structures.
ENDS



;/****************************************************************************
;*	 struct mpInstrument
;*	 -------------------
;* Description:  General instrument structure in memory
;\****************************************************************************

STRUC	mpInstrument
	fileName	DB	13 dup (?)
	iname		DB	32 dup (?)
	sample		DD	?
	length		DW	?
	loopStart	DW	?
	loopEnd 	DW	?
	looping 	DB	?
	volume		DB	?
	c2Rate		DD	?
	finetune	DB	?
	sdInstHandle	DW	?
ENDS



;/****************************************************************************
;*	 struct mpPattern
;*	 ----------------
;* Description:  General pattern structure
;\****************************************************************************

STRUC	mpPattern
	length		DW	?
	LABEL		data	BYTE
ENDS



;/****************************************************************************
;*	 struct mpModule
;*	 ---------------
;* Description:  General Module structure. See MPLAYER.TXT for documentation
;\****************************************************************************

STRUC	mpModule
	ID		DD	?
	IDnum		DW	?
        MP              DD      ?
        songName        DB      32 dup (?)
	songLength	DW	?
	numInsts	DW	?
	numPatts	DW	?
	numChans	DW	?
	flags		DW	?
	masterVol	DB	?
	speed		DB	?
	tempo		DB	?
	masterMult	DB	?
	stereo		DB	?
	chanSettings	DB	32 dup (?)
	orders		DD	?
	insts		DD	?
	patterns	DD	?
	instsUsed	DD	?
ENDS



;/****************************************************************************
;*	 struct ModulePlayer
;*	 -------------------
;* Description:  Module Player structure
;\****************************************************************************

STRUC	ModulePlayer
	Identify	DD	?
	Init		DD	?
	Close		DD	?

	LoadModule	DD	?
	FreeModule	DD	?

	PlayModule	DD	?
	StopModule	DD	?

        SetUpdRateFunct DD      ?

	Play		DD	?

	SetPosition	DD	?
	GetInformation	DD	?
        SetMasterVolume DD      ?
ENDS
