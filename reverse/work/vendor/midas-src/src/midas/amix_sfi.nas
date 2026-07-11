;*      mix_sfi.c
;*
;* Digital Sound Mixer, stereo floating point interpolating mixing routines
;*
;* $Id: amix_sfi.nas,v 1.5.2.1 1997/09/06 17:16:01 pekangas Exp $
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

DATASEG

one_per_2_16	DD	0.0000152587890625	; 1 / 2^16
volMulL		DD	0
volMulR		DD	0
posFrac		DD	0			; position fractional part
stepFrac	DD	0			; step fractional part
tmp1		DD	0
tmp2		DD	0
lastAddress	DD	0




CODESEG


GLOBALLABEL dsmMixStereo8MonoAsmFI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREO8MONO_FI, [esp+20]

	; volMulL = dsmMixLeftVolFloat * 1.0 / 2^16
	fld	dword [dsmMixLeftVolFloat]
	fmul	dword [one_per_2_16]
	fstp	dword [volMulL]	
	fld	dword [dsmMixRightVolFloat]
	fmul	dword [one_per_2_16]
	fstp	dword [volMulR]	

	mov	edi,[dsmMixDest]	; edi = dest
	mov	esi,[dsmMixSample]	; esi = sample

	mov	eax,[esp+20]
	mov	ecx,edi

	shl	eax,3
	mov	ebx,[dsmMixStep]

	add	ecx,eax			; ecx = edi+numBytes
	mov	ebp,ebx

	sar	ebx,16			; ebx = step whole
	mov	eax,[dsmMixSrcPos]

	shl	ebp,16			; ebp = step fraction
	mov	edx,eax

	sar	eax,16
	mov	[stepFrac],ebp

	shl	edx,16			; edx = position fraction
	mov	[posFrac],edx
	add	esi,eax			; esi = sample + position whole

	mov	eax,[dsmByteFloatTable]

	put_eax	Mono8FloatTable1, Mono8Reloc1
	put_eax	Mono8FloatTable2, Mono8Reloc2

	put_eax	Mono8FloatTable3, Mono8Reloc3
	put_eax	Mono8FloatTable4, Mono8Reloc4

	xor	eax,eax
	mov	[lastAddress],ecx

	jmp	short Mono8Loop

Mono8Loop:
	shr	edx,16
	mov	al,[esi+1]

	mov	[tmp1],edx
	mov	ecx,0x10000

	fild	dword [tmp1]		; i1(0)
	fld_4x_eax_plus Mono8FloatTable1
;	fld	dword [4*eax]		; s1, i1(1)

	mov	al,[esi]
	sub	ecx,edx			; s1, i1(2)

	fmulp	st1,st0			; s1i1(0)
	
	mov	[tmp2],ecx
	mov	edx,[posFrac]		; s1i1(1)

	fild	dword [tmp2]		; i0(0), s1i1(2)
	fld_4x_eax_plus Mono8FloatTable2
;	fld	dword [4*eax]		; s0, i0(1), s1i1

	add	edx,ebp
	; empty v-pipe
;10
	adc	esi,ebx
	mov	[posFrac],edx

	shr	edx,16
	mov	al,[esi+1]

	mov	[tmp1],edx
	mov	ecx,0x10000		; s0, i0, s1i1

	fild	dword [tmp1]		; i11(0), s0, i0, s1i1
	fld_4x_eax_plus Mono8FloatTable3
;	fld	dword [4*eax]		; s11, i11(1), s0, i0, s1i1
	fxch	st2,st0
	fmulp	st3,st0			; i11(2), s11, s0i0(0), s1i1

	mov	al,[esi]
	sub	ecx,edx
	
	mov	[tmp2],ecx
	mov	edx,[posFrac]		; i11, s11, s0i0(2), s1i1

	fmulp	st1,st0			; i1s11(0), s0i0, s1i1
	fild	dword [tmp2]		; i01(0), i1s11(1), s0i0, s1i1
	fld_4x_eax_plus Mono8FloatTable4
;	fld	dword [4*eax]		; s01, i01(1), i1s11(2), s0i0, s1i1
;21
	add	edx,ebp
	mov	ecx,[lastAddress]	; s01, i01(2), i1s11, s0i0, s1i1

	fmulp	st1,st0			; i0s01(0), i1s11, s0i0, s1i1
	fxch	st2,st0
	faddp	st3,st0			; i1s11, i0s01(1), s+(0)

	adc	esi,ebx
	mov	[posFrac],edx		; i1s11, i0s01(2), s+(1)
;25
	faddp	st1,st0			; s+1(0), s+(2)
	fld	st1			; s+, s+l(1), s+
	fmul	dword [volMulL]		; svl(0), s+1(2), s+
	fld	st1			; s+1, svl(1), s+1, s+
	fmul	dword [volMulL]		; svl1(0), svl(2), s+1, s+
	fxch	st1,st0
	fadd	dword [edi]		; sl+d(0), svl1(1), s+1, s+
	fxch	st3,st0
	fmul	dword [volMulR]		; svr(0), svl1(2), s+1, sl+d(1)
	fxch	st1,st0
	fadd	dword [edi+8]		; sl+d1(0), svr(1), s+1, sl+d(2)
	fxch	st2,st0
	fmul	dword [volMulR]		; svr1(0), svr(2), sl+d1(1), sl+d(3)
	fxch	st3,st0
	fstp	dword [edi]		; svr, sl+d1(3), svr1(2)
	fadd	dword [edi+4]		; sr+d(0), sl+d1, svr1
	fxch	st2,st0
	fadd	dword [edi+12]		; sr+d1(0), s1+d1, sr+d(1)
	fxch	st1,st0
	fstp	dword [edi+8]		; sr+d1(2), sr+d
;40
	add	edi,16
	; empty v-pipe

	fstp	dword [edi-4]		; sr+d
	fstp	dword [edi-12]

	cmp	edi,ecx
	jb	near Mono8Loop
;46c / 2 smp => 23c/smp?

	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREO8MONO_FI

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

GLOBALLABEL dsmMixStereo8MonoAsmFIReloc
	DD	$-dsmMixStereo8MonoAsmFI
	DD	Mono8RelocTable
	DD	4

Mono8RelocTable:
	DD	Mono8Reloc1
	DD	Mono8Reloc2
	DD	Mono8Reloc3
	DD	Mono8Reloc4


GLOBALLABEL dsmMixStereo16MonoAsmFI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREO16MONO_FI, [esp+20]

	; volMulL = dsmMixLeftVolFloat * 1.0 / 2^16
	fld	dword [dsmMixLeftVolFloat]
	fmul	dword [one_per_2_16]
	fstp	dword [volMulL]	
	fld	dword [dsmMixRightVolFloat]
	fmul	dword [one_per_2_16]
	fstp	dword [volMulR]	

	mov	edi,[dsmMixDest]	; edi = dest
	mov	esi,[dsmMixSample]	; esi = sample

	mov	eax,[esp+20]
	mov	ecx,edi

	shl	eax,3
	mov	ebx,[dsmMixStep]

	add	ecx,eax			; ecx = edi+numBytes
	mov	ebp,ebx

	sar	ebx,16			; ebx = step whole
	mov	eax,[dsmMixSrcPos]

	shl	ebp,16			; ebp = step fraction
	mov	edx,eax

	sar	eax,16
	mov	[stepFrac],ebp

	shr	esi,1
	mov	[lastAddress],ecx

	shl	edx,16			; edx = position fraction

	mov	[posFrac],edx
	add	esi,eax			; esi = sample/2 + position whole


	; loop first round start:
	mov	eax,edx
	mov	ecx,0x10000

	shr	eax,16
	;empty v

	mov	[tmp1],eax
	sub	ecx,eax

	fild	word [2*esi]		; s0(0)
	fild	word [2*esi+2]		; s1(0), s0(1)
	fild	dword [tmp1]		; i1(0), s1(1), s0(2)

	mov	[tmp2],ecx
	add	edx,ebp

	adc	esi,ebx
	mov	eax,edx			; i1(2), s1, s0

	fmulp	st1,st0			; i1s1(0), s0
	fxch	st1,st0			; s0, i1s1(1)

	jmp	.loopstart


.loop:
	; sl+d-11(1), svr-1(2)

	mov	eax,edx
	mov	ecx,0x10000

	shr	eax,16
	;empty v

	fstp	dword [edi-8]		; svr-1
	fadd	dword [edi-4]		; sr+d-1(0)

	mov	[tmp1],eax
	sub	ecx,eax			; sr+d-1(1)

	fild	word [2*esi]		; s0(0), sr+d-1(2)
	fild	word [2*esi+2]		; s1(0), s0(1), sr+d-1(3)
	fild	dword [tmp1]		; i1(0), s1(1), s0(2), sr+d-1

	mov	[tmp2],ecx
	add	edx,ebp
;10
	adc	esi,ebx
	mov	eax,edx			; i1(2), s1, s0, sr+d-1

	fmulp	st1,st0			; i1s1(0), s0, sr+d-1
	fxch	st2,st0
	fstp	dword [edi-4]		; s0, i1s1(2)
.loopstart:
	fild	dword [tmp2]		; i0(0), s0, i1s1

	shr	eax,16
	mov	ecx,0x10000		; i0(1), s0, i1s1

	mov	[tmp1],eax
	sub	ecx,eax			; i0(2), s0, i1s1

	fmulp	st1,st0			; i0s0(0), i1s1
	fild	word [2*esi+2]		; s11(0), i0s0(1), i1s1
	fild	dword [tmp1]		; i11(0), s11(1), i0s0(2), i1s1
	fxch	st2,st0			; i0s0, s11(2), i11(1), i1s1
	faddp	st3,st0			; s11, i11(2), s+(0)
	fmulp	st1,st0			; s1i11(0), s+(1)
;23
	mov	[tmp2],ecx
	add	edx,ebp			; s1i11(1), s+(2)

	fld	st1			; s+, s1i11(2), s+	
	fmul	dword [volMulL]		; svl(0), s1i11, s+
	fild	word [2*esi]		; s01(0), svl(1), s1i11(2), s+
	fild	dword [tmp2]		; i01(0), s01(1), svl(2), s1i11, s+

	adc	esi,ebx
	mov	ecx,[lastAddress]	; i01(1), s01(2), svl, s1i11, s+

	add	edi,16			; i01(2), s01, svl, s1i11, s+
	; empty v
;30
	fmulp	st1,st0			; s0i01(0), svl, s1i11, s+
	fxch	st1,st0
	fadd	dword [edi-16]		; sl+d(0), s0i01(1), s1i11, s+
	fxch	st3,st0	
	fmul	dword [volMulR]		; svr(0), s0i01(2), s1i11, sl+d(1)
	fxch	st1,st0
	faddp	st2,st0			; svr(1), s+1(0), s1+d(2)

	; stall cycle (doh)		; svr(2), s+1(1), sl+d(3)

	fadd	dword [edi-12]		; sr+d(0), s+1(2), sl+d
	fld	st1			; s+1, sr+d(1), s+1, sl+d
	fmul	dword [volMulL]		; svl1(0), sr+d(2), s+1, sl+d
	fxch	st3,st0
	fstp	dword [edi-16]		; sr+d, s+1, svl1(2)
	fstp	dword [edi-12]		; s+1, svl1
	fmul	dword [volMulR]		; svr1(0), svl1
	fxch	st1,st0
	fadd	dword [edi-8]		; sl+d1(0), svr1(1)
;44
	cmp	edi,ecx
	jb	near .loop

	; sl+d1(1), svr1(2)

; 45c / 2smp => 22.5c/smp?

	fxch	st1,st0
	fadd	dword [edi-4]
	fxch	st1,st0
	fstp	dword [edi-8]
	fstp	dword [edi-4]
	
	shr	edx,16
	mov	eax,[dsmMixSample]

	shr	eax,1
	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREO16MONO_FI

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

	

GLOBALLABEL dsmMixStereoUlawMonoAsmFI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREOULAWMONO_FI, [esp+20]

	; volMulL = dsmMixLeftVolFloat * 1.0 / 2^16
	fld	dword [dsmMixLeftVolFloat]
	fmul	dword [one_per_2_16]
	fstp	dword [volMulL]	
	fld	dword [dsmMixRightVolFloat]
	fmul	dword [one_per_2_16]
	fstp	dword [volMulR]	

	mov	edi,[dsmMixDest]	; edi = dest
	mov	esi,[dsmMixSample]	; esi = sample

	mov	eax,[esp+20]
	mov	ecx,edi

	shl	eax,3
	mov	ebx,[dsmMixStep]

	add	ecx,eax			; ecx = edi+numBytes
	mov	ebp,ebx

	sar	ebx,16			; ebx = step whole
	mov	eax,[dsmMixSrcPos]

	shl	ebp,16			; ebp = step fraction
	mov	edx,eax

	sar	eax,16
	mov	[stepFrac],ebp

	shl	edx,16			; edx = position fraction
	mov	[posFrac],edx
	add	esi,eax			; esi = sample + position whole

	mov	eax,[dsmUlawFloatTable]

	put_eax	MonoUlawFloatTable1, MonoUlawReloc1
	put_eax	MonoUlawFloatTable2, MonoUlawReloc2

	put_eax	MonoUlawFloatTable3, MonoUlawReloc3
	put_eax	MonoUlawFloatTable4, MonoUlawReloc4

	xor	eax,eax
	mov	[lastAddress],ecx

	jmp	short MonoUlawLoop

MonoUlawLoop:
	shr	edx,16
	mov	al,[esi+1]

	mov	[tmp1],edx
	mov	ecx,0x10000

	fild	dword [tmp1]		; i1(0)
	fld_4x_eax_plus MonoUlawFloatTable1
;	fld	dword [4*eax]		; s1, i1(1)

	mov	al,[esi]
	sub	ecx,edx			; s1, i1(2)

	fmulp	st1,st0			; s1i1(0)
	
	mov	[tmp2],ecx
	mov	edx,[posFrac]		; s1i1(1)

	fild	dword [tmp2]		; i0(0), s1i1(2)
	fld_4x_eax_plus MonoUlawFloatTable2
;	fld	dword [4*eax]		; s0, i0(1), s1i1

	add	edx,ebp
	; empty v-pipe
;10
	adc	esi,ebx
	mov	[posFrac],edx

	shr	edx,16
	mov	al,[esi+1]

	mov	[tmp1],edx
	mov	ecx,0x10000		; s0, i0, s1i1

	fild	dword [tmp1]		; i11(0), s0, i0, s1i1
	fld_4x_eax_plus MonoUlawFloatTable3
;	fld	dword [4*eax]		; s11, i11(1), s0, i0, s1i1
	fxch	st2,st0
	fmulp	st3,st0			; i11(2), s11, s0i0(0), s1i1

	mov	al,[esi]
	sub	ecx,edx
	
	mov	[tmp2],ecx
	mov	edx,[posFrac]		; i11, s11, s0i0(2), s1i1

	fmulp	st1,st0			; i1s11(0), s0i0, s1i1
	fild	dword [tmp2]		; i01(0), i1s11(1), s0i0, s1i1
	fld_4x_eax_plus MonoUlawFloatTable4
;	fld	dword [4*eax]		; s01, i01(1), i1s11(2), s0i0, s1i1
;21
	add	edx,ebp
	mov	ecx,[lastAddress]	; s01, i01(2), i1s11, s0i0, s1i1

	fmulp	st1,st0			; i0s01(0), i1s11, s0i0, s1i1
	fxch	st2,st0
	faddp	st3,st0			; i1s11, i0s01(1), s+(0)

	adc	esi,ebx
	mov	[posFrac],edx		; i1s11, i0s01(2), s+(1)
;25
	faddp	st1,st0			; s+1(0), s+(2)
	fld	st1			; s+, s+l(1), s+
	fmul	dword [volMulL]		; svl(0), s+1(2), s+
	fld	st1			; s+1, svl(1), s+1, s+
	fmul	dword [volMulL]		; svl1(0), svl(2), s+1, s+
	fxch	st1,st0
	fadd	dword [edi]		; sl+d(0), svl1(1), s+1, s+
	fxch	st3,st0
	fmul	dword [volMulR]		; svr(0), svl1(2), s+1, sl+d(1)
	fxch	st1,st0
	fadd	dword [edi+8]		; sl+d1(0), svr(1), s+1, sl+d(2)
	fxch	st2,st0
	fmul	dword [volMulR]		; svr1(0), svr(2), sl+d1(1), sl+d(3)
	fxch	st3,st0
	fstp	dword [edi]		; svr, sl+d1(3), svr1(2)
	fadd	dword [edi+4]		; sr+d(0), sl+d1, svr1
	fxch	st2,st0
	fadd	dword [edi+12]		; sr+d1(0), s1+d1, sr+d(1)
	fxch	st1,st0
	fstp	dword [edi+8]		; sr+d1(2), sr+d
;40
	add	edi,16
	; empty v-pipe

	fstp	dword [edi-4]		; sr+d
	fstp	dword [edi-12]

	cmp	edi,ecx
	jb	near MonoUlawLoop
;46c / 2 smp => 23c/smp?

	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREOULAWMONO_FI

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

GLOBALLABEL dsmMixStereoUlawMonoAsmFIReloc
	DD	$-dsmMixStereoUlawMonoAsmFI
	DD	MonoUlawRelocTable
	DD	4

MonoUlawRelocTable:
	DD	MonoUlawReloc1
	DD	MonoUlawReloc2
	DD	MonoUlawReloc3
	DD	MonoUlawReloc4


;* $Log: amix_sfi.nas,v $
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
;* Revision 1.3  1997/07/23 17:27:02  pekangas
;* Added profiling hooks
;*
;* Revision 1.2  1997/07/17 19:35:12  pekangas
;* The same assembler source should now compile to Win32, DOS and Linux
;* without modifications
;*
;* Revision 1.1  1997/06/26 19:13:36  pekangas
;* Initial revision
;*
;* Revision 1.1  1997/06/26 14:32:39  pekangas
;* Initial revision
;*
