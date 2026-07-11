;*	MGLOBALS.INC
;*
;* MIDAS Sound System global variables
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*


GLOBAL	LANG useEMS : word		; should EMS be used?
GLOBAL	LANG loadError : word		; module loading error

GLOBAL	LANG ALE : word 		; should Amiga loops be emulated
GLOBAL	LANG ptTempo : word		; should PT modules use tempo
GLOBAL	LANG usePanning : word		; should PT modules use cmd 8 for
					; panning?
GLOBAL	LANG surround : word		; should Surround be enabled?
GLOBAL  LANG extendedOctaves : word     ; enable extended octaves
GLOBAL  LANG useVDS : word              ; use VDS if found?
