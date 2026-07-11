;*	SDEVICE.INC
;*
;* Sound Device definitions
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*

SMPMAX = 65519				; max sample length (65536-16 - 1)
MAXINSTS = 256                          ; maximum number of instruments

DMABUFLEN = 25                          ; mixing buffer length is a 1/25th of
                                        ; a second


ENUM	smpTypes \
	smpNone = 0, \
	smp8bit


ENUM	sdPanning \
	panLeft = -64, \
	panMiddle = 0, \
	panMono = 0, \
	panRight = 64,\
	panSurround = 80h


ENUM	sdSmpPos \
	sdSmpNone = 0, \
	sdSmpConv, \
	sdSmpEMS


;/***************************************************************************\
;*	 enum sdMode
;*	 -----------
;* Description:  Possible SoundDevice output modes
;\***************************************************************************/

ENUM	sdMode \
	sdMono = 1, \
	sdStereo = 2, \
	sd8bit = 4, \
        sd16bit = 8



;/****************************************************************************
;*	 enum sdStatus
;*	 -------------
;* Description:  SoundDevice status
;\****************************************************************************

ENUM	sdStatus \
	sdUnInitialized = 0, \
	sdOK



;/***************************************************************************\
;*       enum sdConfigBits
;*       -----------------
;* Description:  Sound Device configuration information bits
;\***************************************************************************/

ENUM    sdConfigBits \
        sdUsePort = 1, \                ; SD uses an I/O port
        sdUseIRQ = 2, \                 ; SD uses an IRQ
        sdUseDMA = 4, \                 ; SD uses a DMA channel
        sdUseMixRate = 8, \             ; SD uses the set mixing rate
        sdUseOutputMode = 16, \         ; SD uses the set output mode
        sdUseDSM = 32                   ; SD uses software mixing (DSM)



;/****************************************************************************
;*	 struct SoundDevice
;*	 ------------------
;* Description:  SoundDevice structure
;\****************************************************************************

STRUC	SoundDevice

	tempoPoll	DW	?	; 1 if Play() should be called
					; according to the tempo, 0 if
					; not (mixing SoundDevice)
        configBits      DW      ?       ; Configuration info bits. See enum
                                        ; sdConfigBits.
        port            DW      ?       ; Sound Device I/O port address
        IRQ             DB      ?       ; Sound Device IRQ number
        DMA             DB      ?       ; Sound Device DMA channel number
        cardType        DW      ?       ; Sound Device sound card type
        numCardTypes    DW      ?       ; number of different sound card types
                                        ; for this Sound Device
        status          DW      ?       ; Sound Device status
        modes           DW      ?       ; Possible modes for this SD,
                                        ; see enum sdMode

        ID              DD      ?       ; pointer Sound Device ID string
        cardNames       DD      ?       ; pointer to an array of pointers to
                                        ; sound card name strings
        numPortAddresses  DW    ?       ; number of possible port addresses
                                        ; in table
        portAddresses   DD      ?       ; pointer to an array of possible
                                        ; I/O port addresses

	Detect		DD	?
	Init		DD	?
	Close		DD	?

	GetMixRate	DD	?
	GetMode 	DD	?

	OpenChannels	DD	?
	CloseChannels	DD	?
	ClearChannels	DD	?

	Mute		DD	?
	Pause		DD	?

	SetMasterVolume DD	?
        GetMasterVolume DD      ?
        SetAmplification  DD    ?
        GetAmplification  DD    ?

	PlaySound	DD	?
	StopSound	DD	?
	SetRate 	DD	?
	GetRate 	DD	?
	SetVolume	DD	?
        GetVolume       DD      ?
	SetInstrument	DD	?
        GetInstrument   DD      ?
	SetPosition	DD	?
	GetPosition	DD	?

	SetPanning	DD	?
	GetPanning	DD	?
	MuteChannel	DD	?

	AddInstrument	DD	?
	RemInstrument	DD	?

	SetUpdRate	DD	?

        StartPlay       DD      ?
	Play		DD	?
ENDS
