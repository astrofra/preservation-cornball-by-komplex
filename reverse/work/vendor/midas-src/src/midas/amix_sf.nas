;*      amix_sf.nas
;*
;* Digital Sound Mixer, stereo floating point non-interpolating mixing routines
;*
;* $Id: amix_sf.nas,v 1.5.2.1 1997/09/06 17:16:01 pekangas Exp $
;*
;* Copyright 1997 Housemarque Inc.
;*
;* This file is part of MIDAS Digital Audio System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Digital Audio System license, "license.txt". By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*

%include "dsmmix.inc"

CODESEG



GLOBALLABEL dsmMixStereo8MonoAsmF
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREO8MONO_F, [esp+20]

	mov	ecx,[esp+20]
	mov	ebx,[dsmMixStep]

	shl	ecx,3
	mov	ebp,ebx

	mov	edi,[dsmMixDest]	; edi = dest
	mov	esi,[dsmMixSample]	; esi = sample

	add	ecx,edi			; ecx = edi+numBytes

	sar	ebx,16			; ebx = step whole
	mov	eax,[dsmMixSrcPos]

	shl	ebp,16			; ebp = step fraction
	mov	edx,eax

	sar	eax,16

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample + position whole

	put_ecx	Mono8LastAddress,Mono8Reloc1
	mov	ecx,[dsmByteFloatTable]

	xor	eax,eax

	mov	al,[esi]
	add	edx,ebp
	adc	esi,ebx
	jmp	Mono8Start


Mono8Loop:
		; s-1r+(2)
	mov	al,[esi]
	add	edx,ebp				; s-1r+(3)

	adc	esi,ebx
	; empty v-pipe

	fstp	dword [edi-4]
Mono8Start:
	fld	dword [4*eax+ecx]		; s0
	fld	st0				; s0, s0

	mov	al,[esi]
	add	edx,ebp				; s0l(1), s0

	fmul	dword [dsmMixLeftVolFloat]	; s0l(0), s0
	fld	dword [4*eax+ecx]		; s1, s0l(2), s0
	fxch	st1
	fadd	dword [edi]			; s0l+(0), s1, s0
	fxch	st2
	fmul	dword [dsmMixRightVolFloat]	; s0r(0), s1, s0l+(1)
	fld	st1
	fmul	dword [dsmMixLeftVolFloat]	; s1l(0),s0r(2),s1,s0l+(3)
	fxch	st3
	fstp	dword [edi]			; s0r, s1, s1l(2)
	fadd	dword [edi+4]			; s0r+(0), s1, s1l
	fxch	st2
	fadd	dword [edi+8]			; s1l+(0), s1, s0r+(1)
	fxch	st1
	fmul	dword [dsmMixRightVolFloat]	; s1r(0), sll+(1), s0r+(2)
	fxch	st2
	; fpu stall cycle
	fstp	dword [edi+4]			; sll+(3), s1r(2)
	fstp	dword [edi+8]			; s1r
	fadd	dword [edi+12]			; s1r+(0)

	adc	esi,ebx
	add	edi,16				; s1r+(1)

	cmp_edi	Mono8LastAddress
	jb	Mono8Loop			; s1r+(2)

; 26c/2 => 13c/smp?

	fstp	dword [edi-4]

	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREO8MONO_F

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

GLOBALLABEL dsmMixStereo8MonoAsmFReloc
	DD	$-dsmMixStereo8MonoAsmF
	DD	Mono8RelocTable
	DD	1

Mono8RelocTable:
	DD	Mono8Reloc1



GLOBALLABEL dsmMixStereo16MonoAsmF
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREO16MONO_F, [esp+20]

	mov	ecx,[esp+20]
	mov	ebx,[dsmMixStep]

	shl	ecx,3
	mov	ebp,ebx

	mov	edi,[dsmMixDest]	; edi = dest
	mov	esi,[dsmMixSample]	; esi = sample

	add	ecx,edi			; ecx = edi+numBytes

	sar	ebx,16			; ebx = step whole
	mov	eax,[dsmMixSrcPos]

	shl	ebp,16			; ebp = step fraction
	mov	edx,eax

	sar	eax,16

	shr	esi,1

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample/2 + position whole

	fild	word [2*esi]			; s0(0)
	add	edx,ebp				; s0(1)
	; empty v-pipe
	adc	esi,ebx				; s0(2)
	add	edx,ebp

	fld	st0				; s0, s0, s-1r+
	
	jmp	.start

.loop:
	; s-1r+(2)
	fild	word [2*esi]			; s0(0), s-1r+

	add	edx,ebp				; s0(1)
	; empty v-pipe

	adc	esi,ebx				; s0(2)
	add	edx,ebp

	fld	st0				; s0, s0, s-1r+
	fxch	st2
	fstp	dword [edi-4]			; s0, s0
.start:
	fmul	dword [dsmMixLeftVolFloat]	; s0l(0), s0
	fild	word [2*esi]			; s1(0), s0l(1), s0
	fxch	st1	; !np!
	fadd	dword [edi]			; s0l+(0), s1(2), s0
	fxch	st2
	fmul	dword [dsmMixRightVolFloat]	; s0r(0), s1, s0l+(1)
	fld	st1				; s1, s0r(0), s1, s0l+(2)
	fmul	dword [dsmMixLeftVolFloat]	; s1l(0),s0r(1),s1,s0l+(3)
	fxch	st3
	fstp	dword [edi]			; s0r, s1, s1l(2)
	fadd	dword [edi+4]			; s0r+(0), s1, s1l
	fxch	st2
	fadd	dword [edi+8]			; s1l+(0), s1, s0r+(1)
	fxch	st1
	fmul	dword [dsmMixRightVolFloat]	; s1r(0), s1l+(1), s0r+(2)
	fxch	st2
	; fpu stall
	fstp	dword [edi+4]			; s1l+(3), s1r(2)
	fstp	dword [edi+8]			; s1r
	fadd	dword [edi+12]			; s1r+(0)

	adc	esi,ebx
	add	edi,16				; s1r+(1)

	cmp	edi,ecx
	jb	.loop				; s1r+(2)

; 25/2 -> 12.5c/smp?

	fstp	dword [edi-4]

	shr	edx,16
	mov	eax,[dsmMixSample]

	shr	eax,1
	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREO16MONO_F

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret



GLOBALLABEL dsmMixStereoUlawMonoAsmF
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREOULAWMONO_F, [esp+20]

	mov	ecx,[esp+20]
	mov	ebx,[dsmMixStep]

	shl	ecx,3
	mov	ebp,ebx

	mov	edi,[dsmMixDest]	; edi = dest
	mov	esi,[dsmMixSample]	; esi = sample

	add	ecx,edi			; ecx = edi+numBytes

	sar	ebx,16			; ebx = step whole
	mov	eax,[dsmMixSrcPos]

	shl	ebp,16			; ebp = step fraction
	mov	edx,eax

	sar	eax,16

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample + position whole

	put_ecx	MonoUlawLastAddress,MonoUlawReloc1
	mov	ecx,[dsmUlawFloatTable]

	xor	eax,eax

	mov	al,[esi]
	add	edx,ebp
	adc	esi,ebx
	jmp	MonoUlawStart


MonoUlawLoop:
		; s-1r+(2)
	mov	al,[esi]
	add	edx,ebp				; s-1r+(3)

	adc	esi,ebx
	; empty v-pipe

	fstp	dword [edi-4]
MonoUlawStart:
	fld	dword [4*eax+ecx]		; s0
	fld	st0				; s0, s0

	mov	al,[esi]
	add	edx,ebp				; s0l(1), s0

	fmul	dword [dsmMixLeftVolFloat]	; s0l(0), s0
	fld	dword [4*eax+ecx]		; s1, s0l(2), s0
	fxch	st1
	fadd	dword [edi]			; s0l+(0), s1, s0
	fxch	st2
	fmul	dword [dsmMixRightVolFloat]	; s0r(0), s1, s0l+(1)
	fld	st1
	fmul	dword [dsmMixLeftVolFloat]	; s1l(0),s0r(2),s1,s0l+(3)
	fxch	st3
	fstp	dword [edi]			; s0r, s1, s1l(2)
	fadd	dword [edi+4]			; s0r+(0), s1, s1l
	fxch	st2
	fadd	dword [edi+8]			; s1l+(0), s1, s0r+(1)
	fxch	st1
	fmul	dword [dsmMixRightVolFloat]	; s1r(0), sll+(1), s0r+(2)
	fxch	st2
	; fpu stall cycle
	fstp	dword [edi+4]			; sll+(3), s1r(2)
	fstp	dword [edi+8]			; s1r
	fadd	dword [edi+12]			; s1r+(0)

	adc	esi,ebx
	add	edi,16				; s1r+(1)

	cmp_edi	MonoUlawLastAddress
	jb	MonoUlawLoop			; s1r+(2)

; 26c/2 => 13c/smp?

	fstp	dword [edi-4]

	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREOULAWMONO_F

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

GLOBALLABEL dsmMixStereoUlawMonoAsmFReloc
	DD	$-dsmMixStereoUlawMonoAsmF
	DD	MonoUlawRelocTable
	DD	1

MonoUlawRelocTable:
	DD	MonoUlawReloc1






;* $Log: amix_sf.nas,v $
;* Revision 1.5.2.1  1997/09/06 17:16:01  pekangas
;* Optimized the floating point mixing routines for Pentium Pro / Pentium II
;*
;* Revision 1.5  1997/07/31 10:56:35  pekangas
;* Renamed from MIDAS Sound System to MIDAS Digital Audio System
;*
;* Revision 1.4  1997/07/30 19:04:02  pekangas
;* Added optimized mono u-law mixing routines and enabled u-law autoconversion
;* by default
;*
;* Revision 1.3  1997/07/23 17:27:01  pekangas
;* Added profiling hooks
;*
;* Revision 1.2  1997/07/17 19:35:12  pekangas
;* The same assembler source should now compile to Win32, DOS and Linux
;* without modifications
;*
;* Revision 1.1  1997/06/04 15:26:20  pekangas
;* Initial revision
;*
;* Revision 1.1  1997/06/02 15:22:54  pekangas
;* Initial revision
;*
