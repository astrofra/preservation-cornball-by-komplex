;*      amix_mi.c
;*
;* Digital Sound Mixer, mono integer non-interpolating mixing routines
;*
;* $Id: amix_mi.nas,v 1.8 1997/07/31 16:24:28 pekangas Exp $
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




GLOBALLABEL dsmMixMono8MonoAsmI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_MONO8MONO_I, [esp+20]

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
	; mov lastAddress,ecx
	DB	0x89, 0x0D
Mono8Reloc1:
	DD	Mono8LastAddress

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample + position whole

	mov	eax,[dsmMixLeftVolTable]
	shr	eax,2

	; mov stepWhole1,ebx
	DB	0x89, 0x1d
Mono8Reloc2:
	DD	Mono8StepWhole1
	; mov stepWhole2,ebx
	DB	0x89, 0x1d
Mono8Reloc3:
	DD	Mono8StepWhole2

	jmp	short Mono8Loop


Mono8Loop:
	mov	al,[esi]
	add	edx,ebp

	; adc esi,stepWhole
	DB	0x81, 0xd6
Mono8StepWhole1:
	DD	0x1742ABBA
	mov	ecx,[edi]

	mov	ebx,[4*eax]
	add	edi,8

	mov	al,[esi]
	add	ebx,ecx

	mov	[edi-8],ebx
	add	edx,ebp

	mov	ecx,[4*eax]
	mov	ebx,[edi-4]

	; adc esi,stepWhole
	DB	0x81, 0xd6
Mono8StepWhole2:
	DD	0x1742ABBA
	add	ebx,ecx

	mov	[edi-4],ebx

	; cmp edi,lastAddress
	DB	0x81, 0xff
Mono8LastAddress:
	DD	0

	jne	Mono8Loop

; 9c/2 -> 4.5 c/smp?


	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_MONO8MONO_I

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret


GLOBALLABEL dsmMixMono8MonoAsmIReloc
	DD	$-dsmMixMono8MonoAsmI
	DD	Mono8RelocTable
	DD	3

Mono8RelocTable:
	DD	Mono8Reloc1
	DD	Mono8Reloc2
	DD	Mono8Reloc3



GLOBALLABEL dsmMixMono16MonoAsmI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_MONO16MONO_I, [esp+20]

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
	;mov [stepFrac1],ebp
	DB	0x89, 0x2d
Mono16Reloc1:
	DD	Mono16StepFrac1

	shr	esi,1
	;mov [stepFrac2],ebp
	DB	0x89, 0x2d
Mono16Reloc2:
	DD	Mono16StepFrac2

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample/2 + position whole

	mov	eax,[dsmMixLeftVolTable]
	shr	eax,2

	;mov	[stepWhole1],ebx
	DB	0x89, 0x1d
Mono16Reloc3:
	DD	Mono16StepWhole1
	;mov	[stepWhole2],ebx
	DB	0x89, 0x1d
Mono16Reloc4:
	DD	Mono16StepWhole2

	mov	ebx,eax
	;mov	[lastAddress],ecx
	DB	0x89, 0x0d
Mono16Reloc5:
	DD	Mono16LastAddress


Mono16Loop:
	mov	al,[2*esi]		; al = s0l
	mov	ecx,[edi]		; ecx = dest

	add	al,0x80
	mov	bl,[2*esi+1]		; bl = s0h

	adc	bl,0x80
	;add edx,stepFrac
	DB	0x81, 0xc2
Mono16StepFrac1:
	DD	0xBABEFACE

	; adc esi,stepWhole
	DB	0x81, 0xd6
Mono16StepWhole1:
	DD	0x1742ABBA
	mov	ebp,[4*eax]		; ebp = vol*s0l

	sar	ebp,8
	add	edi,8

	mov	al,[2*esi]
	add	ecx,ebp

	mov	ebp,[4*ebx]
	mov	bl,[2*esi+1]

	add	ecx,ebp
	add	al,0x80

	adc	bl,0x80
	mov	[edi-8],ecx

	mov	ebp,[4*eax]
	mov	ecx,[edi-4]

	sar	ebp,8
	; vim

	add	ecx,ebp
	mov	ebp,[4*ebx]

	add	ecx,ebp
	;add edx,stepFrac
	DB	0x81, 0xc2
Mono16StepFrac2:
	DD	0xBABEFACE

	; adc esi,stepWhole
	DB	0x81, 0xd6
Mono16StepWhole2:
	DD	0x1742ABBA
	mov	[edi-4],ecx

	; cmp edi,lastAddress
	DB	0x81, 0xff
Mono16LastAddress:
	DD	0xFEEDBEEF
	jne	Mono16Loop

; 15c/2smp -> 7.5c/smp?

;---
	shr	edx,16
	mov	eax,[dsmMixSample]

	shr	eax,1
	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_MONO16MONO_I

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

GLOBALLABEL dsmMixMono16MonoAsmIReloc
	DD	$-dsmMixMono16MonoAsmI
	DD	Mono16RelocTable
	DD	5

Mono16RelocTable:
	DD	Mono16Reloc1
	DD	Mono16Reloc2
	DD	Mono16Reloc3
	DD	Mono16Reloc4
	DD	Mono16Reloc5



GLOBALLABEL dsmMixMonoUlawMonoAsmI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_MONOULAWMONO_I, [esp+20]

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
	put_ecx	MonoUlawLastAddress, MonoUlawReloc1

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample + position whole

	mov	eax,[dsmMixLeftUlawVolTable]
	shr	eax,2

	put_ebx	MonoUlawStepWhole1, MonoUlawReloc2
	put_ebx	MonoUlawStepWhole2, MonoUlawReloc3

	jmp	short MonoUlawLoop


MonoUlawLoop:
	mov	al,[esi]
	add	edx,ebp

	adc_esi	MonoUlawStepWhole1
	mov	ecx,[edi]

	mov	ebx,[4*eax]
	add	edi,8

	mov	al,[esi]
	add	ebx,ecx

	mov	[edi-8],ebx
	add	edx,ebp

	mov	ecx,[4*eax]
	mov	ebx,[edi-4]

	adc_esi	MonoUlawStepWhole2
	add	ebx,ecx

	mov	[edi-4],ebx
	cmp_edi	MonoUlawLastAddress

	jne	MonoUlawLoop

; 9c/2 -> 4.5 c/smp?


	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_MONOULAWMONO_I

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret


GLOBALLABEL dsmMixMonoUlawMonoAsmIReloc
	DD	$-dsmMixMonoUlawMonoAsmI
	DD	MonoUlawRelocTable
	DD	3

MonoUlawRelocTable:
	DD	MonoUlawReloc1
	DD	MonoUlawReloc2
	DD	MonoUlawReloc3








;* $Log: amix_mi.nas,v $
;* Revision 1.8  1997/07/31 16:24:28  pekangas
;* Fixed a sound quality problem not caught by the tests
;*
;* Revision 1.7  1997/07/31 10:56:35  pekangas
;* Renamed from MIDAS Sound System to MIDAS Digital Audio System
;*
;* Revision 1.6  1997/07/30 19:04:01  pekangas
;* Added optimized mono u-law mixing routines and enabled u-law autoconversion
;* by default
;*
;* Revision 1.5  1997/07/29 11:44:00  pekangas
;* Cleanup made possible by NASM 0.95 release
;*
;* Revision 1.4  1997/07/23 17:27:03  pekangas
;* Added profiling hooks
;*
;* Revision 1.3  1997/07/17 19:35:12  pekangas
;* The same assembler source should now compile to Win32, DOS and Linux
;* without modifications
;*
;* Revision 1.2  1997/07/17 13:31:27  pekangas
;* Added optimized 16-bit mono sample mixing
;* Optimized 8-bit mono sample mixing a bit more
;*
;* Revision 1.1  1997/07/17 09:55:08  pekangas
;* Initial revision
;*

