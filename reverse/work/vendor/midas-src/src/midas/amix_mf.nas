;*      amix_mf.nas
;*
;* Digital Sound Mixer, mono floating point non-interpolating mixing routines
;*
;* $Id: amix_mf.nas,v 1.6.2.1 1997/09/06 17:16:01 pekangas Exp $
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


GLOBALLABEL dsmMixMono8MonoAsmF
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_MONO8MONO_F, [esp+20]

	mov	ecx,[esp+20]
	mov	ebx,[dsmMixStep]

	shl	ecx,2
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

	mov	eax,[dsmByteFloatTable]
	put_eax	Mono8FloatTable1, Mono8Reloc1

	put_eax	Mono8FloatTable2, Mono8Reloc2
	put_eax	Mono8FloatTable3, Mono8Reloc3

	put_eax	Mono8FloatTable4, Mono8Reloc4
	xor	eax,eax

	jmp	short Mono8Loop

Mono8Loop:
	mov	al,[esi]
	add	edx,ebp

	adc	esi,ebx
	; empty v-pipe

	fld_4x_eax_plus Mono8FloatTable1
	fmul	dword [dsmMixLeftVolFloat]	; s0v(0)

	mov	al,[esi]
	add	edx,ebp				; s0v(1)

	adc	esi,ebx				; s0v(2)
	; empty v-pipe

	fadd	dword [edi]			; s0v+(0)
	fld_4x_eax_plus Mono8FloatTable2
	fmul	dword [dsmMixLeftVolFloat]	; s1v(0), s0v+(2)

	mov	al,[esi]
	add	edx,ebp				; s1v(1), s0v+(3)
;10
	adc	esi,ebx				; s1v(2), s0v+
	; empty v-pipe

	fadd	dword [edi+4]			; s1v+(0), s0v+
	fld_4x_eax_plus Mono8FloatTable3
	fmul	dword [dsmMixLeftVolFloat]	; s2v(0), s1v+(2), s0v+

	mov	al,[esi]
	add	edx,ebp				; s2v(1), s1v+(3), s0v+

	adc	esi,ebx				; s2v(2), s1v+, s0v+
	add	edi,16

	fadd	dword [edi-8]			; s2v+(0), s1v+, s0v+
	fld_4x_eax_plus Mono8FloatTable4
	fmul	dword [dsmMixLeftVolFloat]	; s3v(0), s2v+(2), s1v+, s0v+
	fxch	st3
	fstp	dword [edi-16]			; s2v+, s1v+, s3v(2)
	fxch	st2
	fadd	dword [edi-4]			; s3v+(0), s1v+, s2v+
	fxch	st2
	fstp	dword [edi-8]			; s1v+, s3v+(2)
	fstp	dword [edi-12]			; s3v+
	fstp	dword [edi-4]

	cmp	edi,ecx
	jb	Mono8Loop

; 29c/4 -> 7.25 c/smp?

	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_MONO8MONO_F


	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret


GLOBALLABEL dsmMixMono8MonoAsmFReloc
	DD	$-dsmMixMono8MonoAsmF
	DD	Mono8RelocTable
	DD	4

Mono8RelocTable:
	DD	Mono8Reloc1
	DD	Mono8Reloc2
	DD	Mono8Reloc3
	DD	Mono8Reloc4


;/* Mix 8-bit mono samples: */
;static void CALLING mix8Mono(unsigned numSamples)
;{
;    float *dest = (float*) dsmMixDest;
;    uchar *sample = dsmMixSample;
;
;    while ( numSamples )
;    {
;        *(dest++) += dsmMixLeftVolFloat * dsmByteFloatTable[sample[
;            dsmMixSrcPos >> 16]];
;        dsmMixSrcPos += dsmMixStep;
;        numSamples--;
;    }
;
;    dsmMixDest = (uchar*) dest;
;}



	





GLOBALLABEL dsmMixMono16MonoAsmF
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_MONO16MONO_F, [esp+20]

	mov	ecx,[esp+20]
	mov	ebx,[dsmMixStep]

	shl	ecx,2
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

	fild	word [2*esi]
	jmp	.start

.loop:
		; s3v+(2) tms
	fild	word [2*esi]			; s0(0), snv+(3)
	fxch	st1
	fstp	dword [edi-4]			; s0(2)
.start:
	fmul	dword [dsmMixLeftVolFloat]	; s0v(0)

	add	edx,ebp				; s0v(1)
	; empty v-pipe

	adc	esi,ebx				; s0v(2)
	add	edx,ebp

	fadd	dword [edi]			; s0v+(0)
	fild	word [2*esi]			; s1(0), s0v+(1)

	adc	esi,ebx
	add	edx,ebp				; s1(1), s0v+(2)
;9
	; fpu stall cycle

	fmul	dword [dsmMixLeftVolFloat]	; s1v(0), s0v+(3)
	fild	word [2*esi]			; s2(0), s1v(1), s0v+
	fxch	st2
	fstp	dword [edi]			; s1v, s2(2)

	adc	esi,ebx
	add	edx,ebp				; s1v, s2

	fadd	dword [edi+4]			; s1v+(0), s2
	fild	word [2*esi]			; s3(0), s1v+(1), s2
	fxch	st2
	fmul	dword [dsmMixLeftVolFloat]	; s2v(0), s1v+(2), s3(1)

	adc	esi,ebx				; s2v(2), s1v+, s3
	add	edi,16

	; AGI and FPU stall cycle
;19
	fadd	dword [edi-8]			; s2v+(0), s1v+, s3
	fxch	st2
	fmul	dword [dsmMixLeftVolFloat]	; s3v(0), s1v+, s2v+(1)
	fxch	st1
	fstp	dword [edi-12]			; s3v(2), s2v+(3)
	fadd	dword [edi-4]			; s3v+(0), s2v+
	fxch	st1
	fstp	dword [edi-8]			; s3v+(2)

	cmp	edi,ecx
	jb	.loop

;27c/4 -> 6.75c/smp (really 8c/smp)

	fstp	dword [edi-4]

	shr	edx,16
	mov	eax,[dsmMixSample]

	shr	eax,1
	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_MONO16MONO_F

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret




;static void CALLING mix16Mono(unsigned numSamples)
;{
;    float *dest = (float*) dsmMixDest;
;    S16 *sample = (S16*) dsmMixSample;
;
;    while ( numSamples )
;    {
;        *(dest++) += dsmMixLeftVolFloat * ((float) sample[dsmMixSrcPos >> 16]);
;        dsmMixSrcPos += dsmMixStep;
;        numSamples--;
;    }
;
;    dsmMixDest = (uchar*) dest;    
;}



GLOBALLABEL dsmMixMonoUlawMonoAsmF
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_MONOULAWMONO_F, [esp+20]

	mov	ecx,[esp+20]
	mov	ebx,[dsmMixStep]

	shl	ecx,2
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

	mov	eax,[dsmUlawFloatTable]
	put_eax	MonoUlawFloatTable1, MonoUlawReloc1

	put_eax	MonoUlawFloatTable2, MonoUlawReloc2
	put_eax	MonoUlawFloatTable3, MonoUlawReloc3

	put_eax	MonoUlawFloatTable4, MonoUlawReloc4
	xor	eax,eax

	jmp	short MonoUlawLoop

MonoUlawLoop:
	mov	al,[esi]
	add	edx,ebp

	adc	esi,ebx
	; empty v-pipe

	fld_4x_eax_plus MonoUlawFloatTable1
	fmul	dword [dsmMixLeftVolFloat]	; s0v(0)

	mov	al,[esi]
	add	edx,ebp				; s0v(1)

	adc	esi,ebx				; s0v(2)
	; empty v-pipe

	fadd	dword [edi]			; s0v+(0)
	fld_4x_eax_plus MonoUlawFloatTable2
	fmul	dword [dsmMixLeftVolFloat]	; s1v(0), s0v+(2)

	mov	al,[esi]
	add	edx,ebp				; s1v(1), s0v+(3)
;10
	adc	esi,ebx				; s1v(2), s0v+
	; empty v-pipe

	fadd	dword [edi+4]			; s1v+(0), s0v+
	fld_4x_eax_plus MonoUlawFloatTable3
	fmul	dword [dsmMixLeftVolFloat]	; s2v(0), s1v+(2), s0v+

	mov	al,[esi]
	add	edx,ebp				; s2v(1), s1v+(3), s0v+

	adc	esi,ebx				; s2v(2), s1v+, s0v+
	add	edi,16

	fadd	dword [edi-8]			; s2v+(0), s1v+, s0v+
	fld_4x_eax_plus MonoUlawFloatTable4
	fmul	dword [dsmMixLeftVolFloat]	; s3v(0), s2v+(2), s1v+, s0v+
	fxch	st3
	fstp	dword [edi-16]			; s2v+, s1v+, s3v(2)
	fxch	st2
	fadd	dword [edi-4]			; s3v+(0), s1v+, s2v+
	fxch	st2
	fstp	dword [edi-8]			; s1v+, s3v+(2)
	fstp	dword [edi-12]			; s3v+
	fstp	dword [edi-4]

	cmp	edi,ecx
	jb	MonoUlawLoop

; 29c/4 -> 7.25 c/smp?

	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_MONOULAWMONO_F

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret


GLOBALLABEL dsmMixMonoUlawMonoAsmFReloc
	DD	$-dsmMixMonoUlawMonoAsmF
	DD	MonoUlawRelocTable
	DD	4

MonoUlawRelocTable:
	DD	MonoUlawReloc1
	DD	MonoUlawReloc2
	DD	MonoUlawReloc3
	DD	MonoUlawReloc4








;* $Log: amix_mf.nas,v $
;* Revision 1.6.2.1  1997/09/06 17:16:01  pekangas
;* Optimized the floating point mixing routines for Pentium Pro / Pentium II
;*
;* Revision 1.6  1997/07/31 10:56:35  pekangas
;* Renamed from MIDAS Sound System to MIDAS Digital Audio System
;*
;* Revision 1.5  1997/07/30 19:04:01  pekangas
;* Added optimized mono u-law mixing routines and enabled u-law autoconversion
;* by default
;*
;* Revision 1.4  1997/07/23 17:27:01  pekangas
;* Added profiling hooks
;*
;* Revision 1.3  1997/07/17 19:35:12  pekangas
;* The same assembler source should now compile to Win32, DOS and Linux
;* without modifications
;*
;* Revision 1.2  1997/06/04 15:19:52  pekangas
;* Renamed the mixing routines
;*
;* Revision 1.1  1997/06/02 15:22:54  pekangas
;* Initial revision
;*
