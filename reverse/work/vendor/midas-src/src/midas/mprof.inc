;*      mprof.inc
;*
;* MIDAS internal profiling support
;*
;* $Id: mprof.inc,v 1.3.2.1 1997/08/21 18:00:53 pekangas Exp $
;*
;* Copyright 1997 Housemarque Inc.
;*
;* This file is part of MIDAS Digital Audio System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Digital Audio System license, "license.txt". By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*


%ifndef __mprof_inc
%define __mprof_inc



; Total number of sections:
M_PROF_NUM_SECTIONS EQU 0x200

; Section number starts for different system areas:
M_PROF_NONE EQU 0
M_PROF_COMMON EQU 0x10
M_PROF_GMPLAYER EQU 0x20
M_PROF_DSM EQU 0x100


;/***************************************************************************\
;*      struct mProfSection
;*      -------------------
;* Description: A profiling code section
;\***************************************************************************/

	mProfSection_numCalls		EQU	0x00
	mProfSection_totalCyclesLow	EQU	0x04
	mProfSection_totalCyclesHigh	EQU	0x08
	mProfSection_totalRounds	EQU	0x0C
	mProfSection_minRounds		EQU	0x10
	mProfSection_maxRounds		EQU	0x14
	mProfSection_minCyclesPerRound	EQU	0x18
	mProfSection_maxCyclesPerRound	EQU	0x1C
	mProfSection_curRounds		EQU	0x20
	mProfSection_startCyclesLow	EQU	0x24
	mProfSection_startCyclesHigh	EQU	0x28
;	mProfSection_filler



%ifdef M_PROF

  EXTERNSYMBOL mProfSections
  %ifndef M_TARGET_LINUX
    %define mProfSections _mProfSections
  %endif

  %ifndef MPROF_NOT_EXTERNAL
    EXTERNSYMBOL mProfAsmStartSection
    EXTERNSYMBOL mProfAsmEndSection
    EXTERNSYMBOL mProfAsmEndSectionRounds
    DATASEG
      ; We'll use indirect calls to make sure the code can safely be relocated
      ; in data area
      __mProfAsmStartSection DD mProfAsmStartSection
      __mProfAsmEndSection DD mProfAsmEndSection
  %endif

  ; Start a profiling section:
  %macro M_PROF_START 2
	mov	eax,%1
	mov	ecx,%2
	call	[__mProfAsmStartSection]
	rdtsc
	mov	[ecx],eax
	mov	[ecx+4],edx
  %endmacro

  ; End a profiling section:
  %macro M_PROF_END 1
	rdtsc
	mov	ecx,%1
	call	[__mProfAsmEndSection]
  %endmacro

  ; End a profiling section with a new round count:
  %macro M_PROF_END_ROUNDS 1
	rdtsc
	mov	ecx,%1
	mov	ebx,%2
	call	[__mProfAsmEndSectionRounds]
  %endmacro

%else

  %macro M_PROF_START 2
  %endm

  %macro M_PROF_END 1
  %endm

%endif


%endif

;* $Log: mprof.inc,v $
;* Revision 1.3.2.1  1997/08/21 18:00:53  pekangas
;* Added functions for setting the number of rounds at the end of the section
;*
;* Revision 1.3  1997/07/31 10:56:53  pekangas
;* Renamed from MIDAS Sound System to MIDAS Digital Audio System
;*
;* Revision 1.2  1997/07/29 11:44:00  pekangas
;* Cleanup made possible by NASM 0.95 release
;*
;* Revision 1.1  1997/07/23 17:26:49  pekangas
;* Initial revision
;*
