;*      mix_mfi.c
;*
;* Digital Sound Mixer, mono floating point interpolating mixing routines
;*
;* $Id: amix_mfi.nas,v 1.5.2.1 1997/09/06 17:16:01 pekangas Exp $
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


GLOBALLABEL dsmMixMono8MonoAsmFI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_MONO8MONO_FI, [esp+20]

	; volMulL = dsmMixLeftVolFloat * 1.0 / 2^16
	fld	dword [dsmMixLeftVolFloat]
	fmul	dword [one_per_2_16]
	fstp	dword [volMulL]	

	mov	edi,[dsmMixDest]	; edi = dest
	mov	esi,[dsmMixSample]	; esi = sample

	mov	eax,[esp+20]
	mov	ecx,edi

	shl	eax,2
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

	put_ecx	Mono8LastAddress, Mono8Reloc1
	mov	ecx,[dsmByteFloatTable]

	xor	eax,eax

	mov	al,[esi]
	shr	edx,16			; sv-1+(2)

	mov	[tmp1],edx		; tmp1 = posFrac
	mov	ebp,0x10000		; sv-1+(3)

	jmp	short Mono8Start

	
Mono8Loop:
	; sv-1+(1)

	mov	al,[esi]
	shr	edx,16			; sv-1+(2)

	mov	[tmp1],edx		; tmp1 = posFrac
	mov	ebp,0x10000		; sv-1+(3)

	fstp	dword [edi-4]	

Mono8Start:
	fld	dword [4*eax+ecx]	; s0
	
	mov	al,[esi+1]
	sub	ebp,edx

	mov	[tmp2],ebp		; tmp2 = 0x10000 - posFrac
	mov	edx,[posFrac]
	
	fld	dword [4*eax+ecx]	; s1, s0
	fild	dword [tmp2]		; i0(0), s1, s0
	fild	dword [tmp1]		; i1(0), i0(1), s1, s0
;10
	mov	ebp,[stepFrac]
	; empty v-pipe
	
	add	edx,ebp
	; empty v-pipe

	fmulp	st2,st0			; i0, s1i1(0), s0
	
	adc	esi,ebx
	mov	[posFrac],edx
	
	fmulp	st2,st0			; s1i1(2), s0i0(0)

	mov	al,[esi]		; s1i1, s0i0(1)
	shr	edx,16

	mov	[tmp1],edx		; tmp1 = posFrac
	mov	ebp,0x10000		; s1i1, s0i0(2)

	faddp	st1,st0			; s+(0)
	
	fld	dword [4*eax+ecx]	; s01, s+(1)
	
	mov	al,[esi+1]
	sub	ebp,edx			; s01, s+(2)
;20
	mov	[tmp2],ebp		; tmp2 = 0x10000 - posFrac
	mov	edx,[posFrac]		; s01, s+
	
	fld	dword [4*eax+ecx]	; s11, s01, s+
	fxch	st2
	fmul	dword [volMulL]		; sv(0), s01, s11
	
	fild	dword [tmp2]		; i01(0), sv(1), s01, s11
	fild	dword [tmp1]		; i11(0), i01(1), sv(2), s01, s11
	fld	dword [edi]		; d, i11(1), i01(2), sv, s01, s11
	faddp	st3,st0			; i11(2), i01, sv+, s01, s11
	fmulp	st4,st0			; i01, sv+, s01, s1i1(0)
	fmulp	st2,st0			; sv+, s0i1(0), s1i1(1)
	fstp	dword [edi]		; s0i1(2), s1i1
	faddp	st1,st0			; s1+(0)
;32
	mov	ebp,[stepFrac]
	; empty v-pipe
	
	add	edx,ebp
	; empty v-pipe

	fmul	dword [volMulL]		; sv1(0)

	adc	esi,ebx
	mov	[posFrac],edx	

	add	edi,8			; sv1(2)

	fadd	dword [edi-4]		; sv1+(0)

	cmp_edi	Mono8LastAddress
	jb	near Mono8Loop
	
;39c / 2 smp => 19.5c/smp?

	; 2c stall (iik)
	fstp	dword [edi-4]
	
	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_MONO8MONO_FI

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp

	ret

GLOBALLABEL dsmMixMono8MonoAsmFIReloc
	DD	$-dsmMixMono8MonoAsmFI
	DD	Mono8RelocTable
	DD	1

Mono8RelocTable:
	DD	Mono8Reloc1



GLOBALLABEL dsmMixMono16MonoAsmFI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_MONO16MONO_FI, [esp+20]

	; volMulL = dsmMixLeftVolFloat * 1.0 / 2^16
	fld	dword [dsmMixLeftVolFloat]
	fmul	dword [one_per_2_16]
	fstp	dword [volMulL]	

	mov	edi,[dsmMixDest]	; edi = dest
	mov	esi,[dsmMixSample]	; esi = sample

	mov	eax,[esp+20]
	mov	ecx,edi

	shl	eax,2
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

	mov	eax,edx
	mov	ecx,0x10000
	shr	eax,16
	jmp	.loopstart

.loop:
	; sv-1(1), s+d-2(2)

	mov	eax,edx
	mov	ecx,0x10000		; sv-1(2), s+d-2(3)

	fadd	dword [edi-4]		; s+d-1(0), s+d-2
	fxch	st1,st0
	fstp	dword [edi-8]		; s+d-1(2)
	
	shr	eax,16			; s+d-1(3)
	; empty v

	fstp	dword [edi-4]

.loopstart:
	fild	word [2*esi]		; s0(0)
	fild	word [2*esi+2]		; s1(0), s0(1)

	mov	[tmp1],eax
	sub	ecx,eax
;10
	fild	dword [tmp1]		; i1(0), s1(2), s0

	mov	[tmp2],ecx
	add	edx,ebp

	adc	esi,ebx			; i1(2), s1, s0
	mov	eax,edx

	fmulp	st1,st0			; s1i1(0), s0
	fild	dword [tmp2]		; i0(0), s1i1(1), s0
	
	shr	eax,16
	mov	ecx,0x10000

	mov	[tmp1],eax
	sub	ecx,eax			; i0(2), s1i1, s0

	fild	dword [tmp1]		; i11(0), i0, s1i1, s0
	fild	word [2*esi+2]		; s11(0), i11(1), i0, s1i1, s0
	fxch	st2,st0			; i0, i11(2), s11(1), s1i1, s0
	fmulp	st4,st0			; i11, s11(2), s1i1, s0i0(0)
;21
	mov	[tmp2],ecx
	add	edx,ebp			; i11, s11, s1i1, s0i0(1)

	fmulp	st1,st0			; i1s11(0), s1i1, s0i0(2)
	fxch	st1,st0
	faddp	st2,st0			; i1s11(1), s+(0)
	fild	word [2*esi]		; s01(0), i1s11(2), s+(1)
	fild	dword [tmp2]		; i01(0), s01(1), i1s11, s+(2)

	adc	esi,ebx
	mov	ecx,[lastAddress]	; i01(1), s01(2), i1s11, s+(1)

	add	edi,8			; i01(2), s01, i1s11, s+(2)
	; empty v
;28
	fmulp	st1,st0			; i0s01(0), i1s11, s+
	fld	dword [volMulL]		; v, i0s01(1), i1s11, s+
	fmul	st3,st0			; v, i0s01(2), i1s11, sv(0)
	fxch	st2,st0
	faddp	st1,st0			; s+1(0), v, sv(1)
	fld	dword [edi-8]		; d, s+1(1), v, sv(2)
	faddp	st3,st0			; s+1(2), v, s+d(0)
	fmulp	st1,st0			; sv1(0), s+d(1)
;35
	cmp	edi,ecx
	jb	near .loop
	
;36c / 2 smp => 18c/smp?

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

	M_PROF_END DSMMIX_PROF_MONO16MONO_FI

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret
	


GLOBALLABEL dsmMixMonoUlawMonoAsmFI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_MONOULAWMONO_FI, [esp+20]

	; volMulL = dsmMixLeftVolFloat * 1.0 / 2^16
	fld	dword [dsmMixLeftVolFloat]
	fmul	dword [one_per_2_16]
	fstp	dword [volMulL]	

	mov	edi,[dsmMixDest]	; edi = dest
	mov	esi,[dsmMixSample]	; esi = sample

	mov	eax,[esp+20]
	mov	ecx,edi

	shl	eax,2
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

	put_ecx	MonoUlawLastAddress, MonoUlawReloc1
	mov	ecx,[dsmUlawFloatTable]

	xor	eax,eax

	mov	al,[esi]
	shr	edx,16			; sv-1+(2)

	mov	[tmp1],edx		; tmp1 = posFrac
	mov	ebp,0x10000		; sv-1+(3)

	jmp	short MonoUlawStart

	
MonoUlawLoop:
	; sv-1+(1)

	mov	al,[esi]
	shr	edx,16			; sv-1+(2)

	mov	[tmp1],edx		; tmp1 = posFrac
	mov	ebp,0x10000		; sv-1+(3)

	fstp	dword [edi-4]	

MonoUlawStart:
	fld	dword [4*eax+ecx]	; s0
	
	mov	al,[esi+1]
	sub	ebp,edx

	mov	[tmp2],ebp		; tmp2 = 0x10000 - posFrac
	mov	edx,[posFrac]
	
	fld	dword [4*eax+ecx]	; s1, s0
	fild	dword [tmp2]		; i0(0), s1, s0
	fild	dword [tmp1]		; i1(0), i0(1), s1, s0
;10
	mov	ebp,[stepFrac]
	; empty v-pipe
	
	add	edx,ebp
	; empty v-pipe

	fmulp	st2,st0			; i0, s1i1(0), s0
	
	adc	esi,ebx
	mov	[posFrac],edx
	
	fmulp	st2,st0			; s1i1(2), s0i0(0)

	mov	al,[esi]		; s1i1, s0i0(1)
	shr	edx,16

	mov	[tmp1],edx		; tmp1 = posFrac
	mov	ebp,0x10000		; s1i1, s0i0(2)

	faddp	st1,st0			; s+(0)
	
	fld	dword [4*eax+ecx]	; s01, s+(1)
	
	mov	al,[esi+1]
	sub	ebp,edx			; s01, s+(2)
;20
	mov	[tmp2],ebp		; tmp2 = 0x10000 - posFrac
	mov	edx,[posFrac]		; s01, s+
	
	fld	dword [4*eax+ecx]	; s11, s01, s+
	fxch	st2
	fmul	dword [volMulL]		; sv(0), s01, s11
	
	fild	dword [tmp2]		; i01(0), sv(1), s01, s11
	fild	dword [tmp1]		; i11(0), i01(1), sv(2), s01, s11
	fld	dword [edi]		; d, i11(1), i01(2), sv, s01, s11
	faddp	st3,st0			; i11(2), i01, sv+, s01, s11
	fmulp	st4,st0			; i01, sv+, s01, s1i1(0)
	fmulp	st2,st0			; sv+, s0i1(0), s1i1(1)
	fstp	dword [edi]		; s0i1(2), s1i1
	faddp	st1,st0			; s1+(0)
;32
	mov	ebp,[stepFrac]
	; empty v-pipe
	
	add	edx,ebp
	; empty v-pipe

	fmul	dword [volMulL]		; sv1(0)

	adc	esi,ebx
	mov	[posFrac],edx	

	add	edi,8			; sv1(2)

	fadd	dword [edi-4]		; sv1+(0)

	cmp_edi	MonoUlawLastAddress
	jb	near MonoUlawLoop
	
;39c / 2 smp => 19.5c/smp?

	; 2c stall (iik)
	fstp	dword [edi-4]
	
	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_MONOULAWMONO_FI

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp

	ret

GLOBALLABEL dsmMixMonoUlawMonoAsmFIReloc
	DD	$-dsmMixMonoUlawMonoAsmFI
	DD	MonoUlawRelocTable
	DD	1

MonoUlawRelocTable:
	DD	MonoUlawReloc1







;* $Log: amix_mfi.nas,v $
;* Revision 1.5.2.1  1997/09/06 17:16:01  pekangas
;* Optimized the floating point mixing routines for Pentium Pro / Pentium II
;*
;* Revision 1.5  1997/07/31 10:56:35  pekangas
;* Renamed from MIDAS Sound System to MIDAS Digital Audio System
;*
;* Revision 1.4  1997/07/30 19:04:01  pekangas
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
;* Revision 1.1  1997/06/26 14:32:39  pekangas
;* Initial revision
;*
