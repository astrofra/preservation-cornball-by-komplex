;*      mprofasm.nas
;*
;* MIDAS internal profiling support
;*
;* $Id: mprofasm.nas,v 1.2.2.1 1997/08/21 18:00:53 pekangas Exp $
;*
;* Copyright 1997 Housemarque Inc.
;*
;* This file is part of MIDAS Digital Audio System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Digital Audio System license, "license.txt". By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*

%include "mnasm.inc"
%define MPROF_NOT_EXTERNAL
%include "mprof.inc"


CODESEG


;/***************************************************************************\
;*
;* Function:    void mProfAsmStartSection(void)
;*
;* Description: Starts a profiling section. WARNING! This function does NOT
;*              C calling convention, and should not be called directly!
;*
;* Input:       eax    section number
;*              ecx    number of rounds that will be executed
;*
;* Returns:     ecx    pointer to the address where the current cycle count
;*                     should be stored, low dword first
;*
;\***************************************************************************/

GLOBALLABEL mProfAsmStartSection

	; Slow as hell, but so what?

	push	ebx

	shl	eax,6
	mov	ebx,[mProfSections]
	add	ebx,eax

	mov	[ebx+mProfSection_curRounds],ecx

	mov	ecx,ebx
	add	ecx,mProfSection_startCyclesLow
	mov	eax,[ecx]	; get to cache

	pop	ebx
	ret



;/***************************************************************************\
;*
;* Function:    void mProfAsmEndSection(void);
;*
;* Description: Ends a profiling section. WARNING! This function does NOT
;*              C calling convention, and should not be called directly!
;*
;* Input:       eax:edx current cycle count
;*              ecx     section number
;*
;\***************************************************************************/

GLOBALLABEL mProfAsmEndSection

	; Slow as hell, but so what?

	push	ebx

	shl	ecx,6
	mov	ebx,[mProfSections]
	add	ebx,ecx

	sub	eax,[ebx+mProfSection_startCyclesLow]
	sbb	edx,[ebx+mProfSection_startCyclesHigh]
	add	[ebx+mProfSection_totalCyclesLow],eax
	adc	[ebx+mProfSection_totalCyclesHigh],edx

	mov	ecx,[ebx+mProfSection_curRounds]
	add	[ebx+mProfSection_totalRounds],ecx
	inc	dword [ebx+mProfSection_numCalls]

	cmp	ecx,[ebx+mProfSection_minRounds]
	jae	.notminrounds
	mov	[ebx+mProfSection_minRounds],ecx

.notminrounds:
	cmp	ecx,[ebx+mProfSection_maxRounds]
	jbe	.notmaxrounds
	mov	[ebx+mProfSection_maxRounds],ecx

.notmaxrounds:
	div	ecx

	cmp	eax,[ebx+mProfSection_minCyclesPerRound]
	jae	.notmincycles
	mov	[ebx+mProfSection_minCyclesPerRound],eax

.notmincycles:
	cmp	eax,[ebx+mProfSection_maxCyclesPerRound]
	jbe	.notmaxcycles
	mov	[ebx+mProfSection_maxCyclesPerRound],eax

.notmaxcycles:

	pop	ebx
	ret



;/***************************************************************************\
;*
;* Function:    void mProfAsmEndSectionRounds(void);
;*
;* Description: Ends a profiling section with a new round count
;*
;* Input:       eax:edx current cycle count
;*              ecx     section number
;*              ebx     new number of rounds
;*
;\***************************************************************************/

GLOBALLABEL mProfAsmEndSectionRounds

	push	esi

	mov	esi,ecx
	shl	esi,6
	add	esi,[mProfSections]

	mov	[esi+mProfSection_curRounds],ebx

	call	mProfAsmEndSection

	pop	esi
	ret




;* $Log: mprofasm.nas,v $
;* Revision 1.2.2.1  1997/08/21 18:00:53  pekangas
;* Added functions for setting the number of rounds at the end of the section
;*
;* Revision 1.2  1997/07/31 10:56:53  pekangas
;* Renamed from MIDAS Sound System to MIDAS Digital Audio System
;*
;* Revision 1.1  1997/07/23 17:26:50  pekangas
;* Initial revision
;*






