;*      amix_si.c
;*
;* Digital Sound Mixer, stereo integer non-interpolating mixing routines
;*
;* $Id: amix_si.nas,v 1.7.2.2 1997/08/20 17:49:25 pekangas Exp $
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

posFrac	DD	0


CODESEG




GLOBALLABEL dsmMixStereo8MonoAsmI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREO8MONO_I, [esp+20]

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
	put_ebp	Mono8StepFrac1, Mono8Reloc1

	put_ebp Mono8StepFrac2, Mono8Reloc2
	put_ebx Mono8StepWhole1, Mono8Reloc3

	put_ebp Mono8StepFrac3, Mono8Reloc4
	put_ebx Mono8StepWhole2, Mono8Reloc5

	put_ebp Mono8StepFrac4, Mono8Reloc6
	put_ebx Mono8StepWhole3, Mono8Reloc7

	put_ebx	Mono8StepWhole4, Mono8Reloc8
	put_ecx	Mono8LastAddress, Mono8Reloc9

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample + position whole

	mov	eax,[dsmMixLeftVolTable]
	mov	ebx,[dsmMixRightVolTable]

	shr	eax,2
	shr	ebx,2

	mov	al,[esi]
	mov	bl,al

	jmp short Mono8Loop

Mono8Loop:
	mov	ebp,[4*eax]
	add_edx Mono8StepFrac1

	adc_esi Mono8StepWhole1
	mov	ecx,[edi]

	add	ecx,ebp
	mov	ebp,[4*ebx]

	mov	[edi],ecx
	mov	al,[esi]
	
	mov	ecx,[edi+4]
	mov	bl,al

	add	ecx,ebp
	mov	ebp,[4*eax]

	mov	[edi+4],ecx
	add_edx	Mono8StepFrac2

	adc_esi	Mono8StepWhole2
	mov	ecx,[edi+8]
	
	add	ecx,ebp
	mov	ebp,[4*ebx]

	mov	[edi+8],ecx
	mov	al,[esi]

	mov	ecx,[edi+12]
	mov	bl,al

	add	ecx,ebp
	add_edx Mono8StepFrac3

	adc_esi Mono8StepWhole3
	mov	[edi+12],ecx

	mov	ebp,[4*eax]
	mov	ecx,[edi+16]

	add	ecx,ebp
	mov	al,[esi]

	mov	ebp,[4*ebx]
	mov	[edi+16],ecx

	mov	ecx,[edi+20]
	; empty v

	add	ecx,ebp
	mov	bl,al

	mov	ebp,[4*eax]
	mov	[edi+20],ecx

	mov	ecx,[edi+24]
	add_edx	Mono8StepFrac4

	adc_esi	Mono8StepWhole4
	add	ecx,ebp

	mov	[edi+24],ecx
	mov	ebp,[4*ebx]

	mov	ecx,[edi+28]
	mov	al,[esi]

	add	ecx,ebp
	mov	bl,al

	mov	[edi+28],ecx
	add	edi,32

	cmp_edi	Mono8LastAddress
	jne	near Mono8Loop	

	; 26c/4s -> 6.5c/smp?	


	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREO8MONO_I

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret


GLOBALLABEL dsmMixStereo8MonoAsmIReloc
	DD	$-dsmMixStereo8MonoAsmI
	DD	Mono8RelocTable
	DD	9

Mono8RelocTable:
	DD	Mono8Reloc1
	DD	Mono8Reloc2
	DD	Mono8Reloc3
	DD	Mono8Reloc4
	DD	Mono8Reloc5
	DD	Mono8Reloc6
	DD	Mono8Reloc7
	DD	Mono8Reloc8
	DD	Mono8Reloc9



; Middle:
GLOBALLABEL dsmMixStereo8MonoAsmMI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREO8MONO_MI, [esp+20]

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
	put_ebp	Mono8MStepFrac1, Mono8MReloc1

	put_ebp Mono8MStepFrac2, Mono8MReloc2
	put_ebx Mono8MStepWhole1, Mono8MReloc3

	put_ebp Mono8MStepFrac3, Mono8MReloc4
	put_ebx Mono8MStepWhole2, Mono8MReloc5

	put_ebp Mono8MStepFrac4, Mono8MReloc6
	put_ebx Mono8MStepWhole3, Mono8MReloc7

	put_ebx	Mono8MStepWhole4, Mono8MReloc8
	put_ecx	Mono8MLastAddress, Mono8MReloc9

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample + position whole

	mov	eax,[dsmMixLeftVolTable]
	mov	ebx,[dsmMixRightVolTable]

	shr	eax,2
	shr	ebx,2

	mov	al,[esi]
	mov	bl,al

	jmp short Mono8MLoop

Mono8MLoop:
	mov	al,[esi]
	add_edx	Mono8MStepFrac1

	adc_esi	Mono8MStepWhole1
	mov	ecx,[edi]

	mov	ebp,[4*eax]
	mov	ebx,[edi+4]

	add	ecx,ebp
	add	ebx,ebp

	mov	[edi],ecx
	mov	al,[esi]

	mov	[edi+4],ebx
	add_edx	Mono8MStepFrac2

	adc_esi	Mono8MStepWhole2
	mov	ecx,[edi+8]

	mov	ebp,[4*eax]
	mov	ebx,[edi+12]

	add	ecx,ebp
	add	ebx,ebp

	mov	[edi+8],ecx
	mov	al,[esi]

	mov	[edi+12],ebx
	add_edx	Mono8MStepFrac3

	adc_esi	Mono8MStepWhole3
	mov	ecx,[edi+16]

	mov	ebp,[4*eax]
	mov	ebx,[edi+20]

	add	ecx,ebp
	add	ebx,ebp

	mov	[edi+16],ecx
	mov	al,[esi]

	mov	[edi+20],ebx
	add_edx	Mono8MStepFrac4

	adc_esi	Mono8MStepWhole4
	mov	ecx,[edi+24]

	mov	ebp,[4*eax]
	mov	ebx,[edi+28]

	add	ecx,ebp
	add	ebx,ebp

	mov	[edi+24],ecx
	mov	al,[esi]

	mov	[edi+28],ebx

	add	edi,32
	; bii

	cmp_edi	Mono8MLastAddress
	jb	near Mono8MLoop

	; 23c/4s -> 5.75c/smp?	


	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREO8MONO_MI

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret


GLOBALLABEL dsmMixStereo8MonoAsmMIReloc
	DD	$-dsmMixStereo8MonoAsmMI
	DD	Mono8MRelocTable
	DD	9

Mono8MRelocTable:
	DD	Mono8MReloc1
	DD	Mono8MReloc2
	DD	Mono8MReloc3
	DD	Mono8MReloc4
	DD	Mono8MReloc5
	DD	Mono8MReloc6
	DD	Mono8MReloc7
	DD	Mono8MReloc8
	DD	Mono8MReloc9





; Surround:
GLOBALLABEL dsmMixStereo8MonoAsmSI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREO8MONO_SI, [esp+20]

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
	put_ebp	Mono8SStepFrac1, Mono8SReloc1

	put_ebp Mono8SStepFrac2, Mono8SReloc2
	put_ebx Mono8SStepWhole1, Mono8SReloc3

	put_ebp Mono8SStepFrac3, Mono8SReloc4
	put_ebx Mono8SStepWhole2, Mono8SReloc5

	put_ebp Mono8SStepFrac4, Mono8SReloc6
	put_ebx Mono8SStepWhole3, Mono8SReloc7

	put_ebx	Mono8SStepWhole4, Mono8SReloc8
	put_ecx	Mono8SLastAddress, Mono8SReloc9

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample + position whole

	mov	eax,[dsmMixLeftVolTable]
	mov	ebx,[dsmMixRightVolTable]

	shr	eax,2
	shr	ebx,2

	mov	al,[esi]
	mov	bl,al

	jmp short Mono8SLoop

Mono8SLoop:
	mov	al,[esi]
	add_edx	Mono8SStepFrac1

	adc_esi	Mono8SStepWhole1
	mov	ecx,[edi]

	mov	ebp,[4*eax]
	mov	ebx,[edi+4]

	add	ecx,ebp
	sub	ebx,ebp

	mov	[edi],ecx
	mov	al,[esi]

	mov	[edi+4],ebx
	add_edx	Mono8SStepFrac2

	adc_esi	Mono8SStepWhole2
	mov	ecx,[edi+8]

	mov	ebp,[4*eax]
	mov	ebx,[edi+12]

	add	ecx,ebp
	sub	ebx,ebp

	mov	[edi+8],ecx
	mov	al,[esi]

	mov	[edi+12],ebx
	add_edx	Mono8SStepFrac3

	adc_esi	Mono8SStepWhole3
	mov	ecx,[edi+16]

	mov	ebp,[4*eax]
	mov	ebx,[edi+20]

	add	ecx,ebp
	sub	ebx,ebp

	mov	[edi+16],ecx
	mov	al,[esi]

	mov	[edi+20],ebx
	add_edx	Mono8SStepFrac4

	adc_esi	Mono8SStepWhole4
	mov	ecx,[edi+24]

	mov	ebp,[4*eax]
	mov	ebx,[edi+28]

	add	ecx,ebp
	sub	ebx,ebp

	mov	[edi+24],ecx
	mov	al,[esi]

	mov	[edi+28],ebx

	add	edi,32
	; bii

	cmp_edi	Mono8SLastAddress
	jb	near Mono8SLoop

	; 23c/4s -> 5.75c/smp?	


	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREO8MONO_SI

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret


GLOBALLABEL dsmMixStereo8MonoAsmSIReloc
	DD	$-dsmMixStereo8MonoAsmSI
	DD	Mono8SRelocTable
	DD	9

Mono8SRelocTable:
	DD	Mono8SReloc1
	DD	Mono8SReloc2
	DD	Mono8SReloc3
	DD	Mono8SReloc4
	DD	Mono8SReloc5
	DD	Mono8SReloc6
	DD	Mono8SReloc7
	DD	Mono8SReloc8
	DD	Mono8SReloc9






GLOBALLABEL dsmMixStereo16MonoAsmI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREO16MONO_I, [esp+20]

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
	put_ebx	Mono16StepWhole1, Mono16Reloc1

	shr	esi,1
	put_ebp	Mono16StepFrac1, Mono16Reloc2

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample/2 + position whole

	mov	eax,[dsmMixLeftVolTable]
	mov	ebx,[dsmMixRightVolTable]

	shr	eax,2
	shr	ebx,2

	mov	[posFrac],edx
	put_ecx	Mono16LastAddress, Mono16Reloc3

	jmp short Mono16Loop

Mono16Loop:
	mov	al,[2*esi]
	mov	ecx,[edi]

	mov	edx,eax
	add	al,0x80

	mov	bl,al
	mov	dl,[2*esi+1]

	adc	dl,0x80
	mov	ebp,[4*eax]

	sar	ebp,8
	add	edi,8

	add	ecx,ebp
	mov	ebp,[4*edx]

	add	ecx,ebp
	mov	ebp,[4*ebx]

	mov	[edi-8],ecx
	mov	bl,dl	

	sar	ebp,8
	mov	ecx,[edi-4]

	add	ecx,ebp
	mov	ebp,[4*ebx]

	add	ecx,ebp
	mov	edx,[posFrac]

	mov	[edi-4],ecx
	add_edx	Mono16StepFrac1

	adc_esi	Mono16StepWhole1
	mov	[posFrac],edx

	cmp_edi	Mono16LastAddress
	jne	Mono16Loop

	; 14c/smp?

	shr	edx,16
	mov	eax,[dsmMixSample]

	shr	eax,1
	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREO16MONO_I

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

GLOBALLABEL dsmMixStereo16MonoAsmIReloc
	DD	$-dsmMixStereo16MonoAsmI
	DD	Mono16RelocTable
	DD	3

Mono16RelocTable:
	DD	Mono16Reloc1
	DD	Mono16Reloc2
	DD	Mono16Reloc3



; Middle:
GLOBALLABEL dsmMixStereo16MonoAsmMI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREO16MONO_MI, [esp+20]

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
	put_ebx	Mono16MStepWhole1, Mono16MReloc1

	shr	esi,1
	put_ebp	Mono16MStepFrac1, Mono16MReloc2

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample/2 + position whole

	mov	eax,[dsmMixLeftVolTable]

	shr	eax,2

	mov	ebx,eax
	put_ecx	Mono16MLastAddress, Mono16MReloc3

	jmp short Mono16MLoop

Mono16MLoop:
	mov	ebx,eax
	mov	al,[2*esi]

	add	al,0x80
	mov	bl,[2*esi+1]

	adc	bl,0x80
	add_edx	Mono16MStepFrac1

	adc_esi	Mono16MStepWhole1
	mov	ebp,[4*eax]

	sar	ebp,8
	mov	ebx,[4*ebx]

	add	ebp,ebx
	mov	ecx,[edi]

	add	ecx,ebp
	mov	ebx,[edi+4]

	mov	[edi],ecx
	add	ebx,ebp

	mov	[edi+4],ebx
	add	edi,8

	cmp_edi	Mono16MLastAddress
	jne	Mono16MLoop

	; 10c/smp?

	shr	edx,16
	mov	eax,[dsmMixSample]

	shr	eax,1
	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREO16MONO_MI

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

GLOBALLABEL dsmMixStereo16MonoAsmMIReloc
	DD	$-dsmMixStereo16MonoAsmMI
	DD	Mono16MRelocTable
	DD	3

Mono16MRelocTable:
	DD	Mono16MReloc1
	DD	Mono16MReloc2
	DD	Mono16MReloc3


; Surround:
GLOBALLABEL dsmMixStereo16MonoAsmSI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREO16MONO_SI, [esp+20]

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
	put_ebx	Mono16SStepWhole1, Mono16SReloc1

	shr	esi,1
	put_ebp	Mono16SStepFrac1, Mono16SReloc2

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample/2 + position whole

	mov	eax,[dsmMixLeftVolTable]

	shr	eax,2

	mov	ebx,eax
	put_ecx	Mono16SLastAddress, Mono16SReloc3

	jmp short Mono16SLoop

Mono16SLoop:
	mov	ebx,eax
	mov	al,[2*esi]

	add	al,0x80
	mov	bl,[2*esi+1]

	adc	bl,0x80
	add_edx	Mono16SStepFrac1

	adc_esi	Mono16SStepWhole1
	mov	ebp,[4*eax]

	sar	ebp,8
	mov	ebx,[4*ebx]

	add	ebp,ebx
	mov	ecx,[edi]

	add	ecx,ebp
	mov	ebx,[edi+4]

	mov	[edi],ecx
	sub	ebx,ebp

	mov	[edi+4],ebx
	add	edi,8

	cmp_edi	Mono16SLastAddress
	jne	Mono16SLoop

	; 10c/smp?

	shr	edx,16
	mov	eax,[dsmMixSample]

	shr	eax,1
	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREO16MONO_SI

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

GLOBALLABEL dsmMixStereo16MonoAsmSIReloc
	DD	$-dsmMixStereo16MonoAsmSI
	DD	Mono16SRelocTable
	DD	3

Mono16SRelocTable:
	DD	Mono16SReloc1
	DD	Mono16SReloc2
	DD	Mono16SReloc3




GLOBALLABEL dsmMixStereoUlawMonoAsmI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREOULAWMONO_I, [esp+20]

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
	put_ebp	MonoUlawStepFrac1, MonoUlawReloc1

	put_ebp MonoUlawStepFrac2, MonoUlawReloc2
	put_ebx MonoUlawStepWhole1, MonoUlawReloc3

	put_ebp MonoUlawStepFrac3, MonoUlawReloc4
	put_ebx MonoUlawStepWhole2, MonoUlawReloc5

	put_ebp MonoUlawStepFrac4, MonoUlawReloc6
	put_ebx MonoUlawStepWhole3, MonoUlawReloc7

	put_ebx	MonoUlawStepWhole4, MonoUlawReloc8
	put_ecx	MonoUlawLastAddress, MonoUlawReloc9

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample + position whole

	mov	eax,[dsmMixLeftUlawVolTable]
	mov	ebx,[dsmMixRightUlawVolTable]

	shr	eax,2
	shr	ebx,2

	mov	al,[esi]
	mov	bl,al

	jmp short MonoUlawLoop

MonoUlawLoop:
	mov	ebp,[4*eax]
	add_edx MonoUlawStepFrac1

	adc_esi MonoUlawStepWhole1
	mov	ecx,[edi]

	add	ecx,ebp
	mov	ebp,[4*ebx]

	mov	[edi],ecx
	mov	al,[esi]
	
	mov	ecx,[edi+4]
	mov	bl,al

	add	ecx,ebp
	mov	ebp,[4*eax]

	mov	[edi+4],ecx
	add_edx	MonoUlawStepFrac2

	adc_esi	MonoUlawStepWhole2
	mov	ecx,[edi+8]
	
	add	ecx,ebp
	mov	ebp,[4*ebx]

	mov	[edi+8],ecx
	mov	al,[esi]

	mov	ecx,[edi+12]
	mov	bl,al

	add	ecx,ebp
	add_edx MonoUlawStepFrac3

	adc_esi MonoUlawStepWhole3
	mov	[edi+12],ecx

	mov	ebp,[4*eax]
	mov	ecx,[edi+16]

	add	ecx,ebp
	mov	al,[esi]

	mov	ebp,[4*ebx]
	mov	[edi+16],ecx

	mov	ecx,[edi+20]
	; empty v

	add	ecx,ebp
	mov	bl,al

	mov	ebp,[4*eax]
	mov	[edi+20],ecx

	mov	ecx,[edi+24]
	add_edx	MonoUlawStepFrac4

	adc_esi	MonoUlawStepWhole4
	add	ecx,ebp

	mov	[edi+24],ecx
	mov	ebp,[4*ebx]

	mov	ecx,[edi+28]
	mov	al,[esi]

	add	ecx,ebp
	mov	bl,al

	mov	[edi+28],ecx
	add	edi,32

	cmp_edi	MonoUlawLastAddress
	jne	near MonoUlawLoop	

	; 26c/4s -> 6.5c/smp?	


	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREOULAWMONO_I

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret


GLOBALLABEL dsmMixStereoUlawMonoAsmIReloc
	DD	$-dsmMixStereoUlawMonoAsmI
	DD	MonoUlawRelocTable
	DD	9

MonoUlawRelocTable:
	DD	MonoUlawReloc1
	DD	MonoUlawReloc2
	DD	MonoUlawReloc3
	DD	MonoUlawReloc4
	DD	MonoUlawReloc5
	DD	MonoUlawReloc6
	DD	MonoUlawReloc7
	DD	MonoUlawReloc8
	DD	MonoUlawReloc9



; Middle:
GLOBALLABEL dsmMixStereoUlawMonoAsmMI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREOULAWMONO_MI, [esp+20]

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
	put_ebp	MonoUlawMStepFrac1, MonoUlawMReloc1

	put_ebp MonoUlawMStepFrac2, MonoUlawMReloc2
	put_ebx MonoUlawMStepWhole1, MonoUlawMReloc3

	put_ebp MonoUlawMStepFrac3, MonoUlawMReloc4
	put_ebx MonoUlawMStepWhole2, MonoUlawMReloc5

	put_ebp MonoUlawMStepFrac4, MonoUlawMReloc6
	put_ebx MonoUlawMStepWhole3, MonoUlawMReloc7

	put_ebx	MonoUlawMStepWhole4, MonoUlawMReloc8
	put_ecx	MonoUlawMLastAddress, MonoUlawMReloc9

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample + position whole

	mov	eax,[dsmMixLeftUlawVolTable]
	mov	ebx,[dsmMixRightUlawVolTable]

	shr	eax,2
	shr	ebx,2

	mov	al,[esi]
	mov	bl,al

	jmp short MonoUlawMLoop

MonoUlawMLoop:
	mov	al,[esi]
	add_edx	MonoUlawMStepFrac1

	adc_esi	MonoUlawMStepWhole1
	mov	ecx,[edi]

	mov	ebp,[4*eax]
	mov	ebx,[edi+4]

	add	ecx,ebp
	add	ebx,ebp

	mov	[edi],ecx
	mov	al,[esi]

	mov	[edi+4],ebx
	add_edx	MonoUlawMStepFrac2

	adc_esi	MonoUlawMStepWhole2
	mov	ecx,[edi+8]

	mov	ebp,[4*eax]
	mov	ebx,[edi+12]

	add	ecx,ebp
	add	ebx,ebp

	mov	[edi+8],ecx
	mov	al,[esi]

	mov	[edi+12],ebx
	add_edx	MonoUlawMStepFrac3

	adc_esi	MonoUlawMStepWhole3
	mov	ecx,[edi+16]

	mov	ebp,[4*eax]
	mov	ebx,[edi+20]

	add	ecx,ebp
	add	ebx,ebp

	mov	[edi+16],ecx
	mov	al,[esi]

	mov	[edi+20],ebx
	add_edx	MonoUlawMStepFrac4

	adc_esi	MonoUlawMStepWhole4
	mov	ecx,[edi+24]

	mov	ebp,[4*eax]
	mov	ebx,[edi+28]

	add	ecx,ebp
	add	ebx,ebp

	mov	[edi+24],ecx
	mov	al,[esi]

	mov	[edi+28],ebx

	add	edi,32
	; bii

	cmp_edi	MonoUlawMLastAddress
	jb	near MonoUlawMLoop

	; 23c/4s -> 5.75c/smp?	


	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREOULAWMONO_MI

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret


GLOBALLABEL dsmMixStereoUlawMonoAsmMIReloc
	DD	$-dsmMixStereoUlawMonoAsmMI
	DD	MonoUlawMRelocTable
	DD	9

MonoUlawMRelocTable:
	DD	MonoUlawMReloc1
	DD	MonoUlawMReloc2
	DD	MonoUlawMReloc3
	DD	MonoUlawMReloc4
	DD	MonoUlawMReloc5
	DD	MonoUlawMReloc6
	DD	MonoUlawMReloc7
	DD	MonoUlawMReloc8
	DD	MonoUlawMReloc9





; Surround:
GLOBALLABEL dsmMixStereoUlawMonoAsmSI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREOULAWMONO_SI, [esp+20]

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
	put_ebp	MonoUlawSStepFrac1, MonoUlawSReloc1

	put_ebp MonoUlawSStepFrac2, MonoUlawSReloc2
	put_ebx MonoUlawSStepWhole1, MonoUlawSReloc3

	put_ebp MonoUlawSStepFrac3, MonoUlawSReloc4
	put_ebx MonoUlawSStepWhole2, MonoUlawSReloc5

	put_ebp MonoUlawSStepFrac4, MonoUlawSReloc6
	put_ebx MonoUlawSStepWhole3, MonoUlawSReloc7

	put_ebx	MonoUlawSStepWhole4, MonoUlawSReloc8
	put_ecx	MonoUlawSLastAddress, MonoUlawSReloc9

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample + position whole

	mov	eax,[dsmMixLeftUlawVolTable]
	mov	ebx,[dsmMixRightUlawVolTable]

	shr	eax,2
	shr	ebx,2

	mov	al,[esi]
	mov	bl,al

	jmp short MonoUlawSLoop

MonoUlawSLoop:
	mov	al,[esi]
	add_edx	MonoUlawSStepFrac1

	adc_esi	MonoUlawSStepWhole1
	mov	ecx,[edi]

	mov	ebp,[4*eax]
	mov	ebx,[edi+4]

	add	ecx,ebp
	sub	ebx,ebp

	mov	[edi],ecx
	mov	al,[esi]

	mov	[edi+4],ebx
	add_edx	MonoUlawSStepFrac2

	adc_esi	MonoUlawSStepWhole2
	mov	ecx,[edi+8]

	mov	ebp,[4*eax]
	mov	ebx,[edi+12]

	add	ecx,ebp
	sub	ebx,ebp

	mov	[edi+8],ecx
	mov	al,[esi]

	mov	[edi+12],ebx
	add_edx	MonoUlawSStepFrac3

	adc_esi	MonoUlawSStepWhole3
	mov	ecx,[edi+16]

	mov	ebp,[4*eax]
	mov	ebx,[edi+20]

	add	ecx,ebp
	sub	ebx,ebp

	mov	[edi+16],ecx
	mov	al,[esi]

	mov	[edi+20],ebx
	add_edx	MonoUlawSStepFrac4

	adc_esi	MonoUlawSStepWhole4
	mov	ecx,[edi+24]

	mov	ebp,[4*eax]
	mov	ebx,[edi+28]

	add	ecx,ebp
	sub	ebx,ebp

	mov	[edi+24],ecx
	mov	al,[esi]

	mov	[edi+28],ebx

	add	edi,32
	; bii

	cmp_edi	MonoUlawSLastAddress
	jb	near MonoUlawSLoop

	; 23c/4s -> 5.75c/smp?	


	shr	edx,16
	mov	eax,[dsmMixSample]

	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREOULAWMONO_SI

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret


GLOBALLABEL dsmMixStereoUlawMonoAsmSIReloc
	DD	$-dsmMixStereoUlawMonoAsmSI
	DD	MonoUlawSRelocTable
	DD	9

MonoUlawSRelocTable:
	DD	MonoUlawSReloc1
	DD	MonoUlawSReloc2
	DD	MonoUlawSReloc3
	DD	MonoUlawSReloc4
	DD	MonoUlawSReloc5
	DD	MonoUlawSReloc6
	DD	MonoUlawSReloc7
	DD	MonoUlawSReloc8
	DD	MonoUlawSReloc9




GLOBALLABEL dsmMixStereo16StereoAsmI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREO16STEREO_I, [esp+20]

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
	put_ebx	Stereo16StepWhole1, Stereo16Reloc1

	shr	esi,2
	put_ebp	Stereo16StepFrac1, Stereo16Reloc2

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample/4 + position whole

	mov	eax,[dsmMixLeftVolTable]
	mov	ebx,[dsmMixRightVolTable]

	shr	eax,2
	shr	ebx,2

	mov	[posFrac],edx
	put_ecx	Stereo16LastAddress, Stereo16Reloc3

	jmp short Stereo16Loop

Stereo16Loop:
	mov	edx,eax
	mov	al,[4*esi]

	mov	dl,[4*esi+1]
	add	al,0x80

	adc	dl,0x80
	mov	bl,[4*esi+2]

	mov	ebp,[4*eax]
	mov	ecx,[edi]

	sar	ebp,8
	add	edi,8

	add	ecx,ebp
	mov	ebp,[4*edx]

	add	ecx,ebp
	add	bl,0x80

	mov	[edi-8],ecx
	mov	ecx,[edi-4]

	mov	ebp,[4*ebx]
	mov	bl,[4*esi+3]

	adc	bl,0x80
	mov	edx,[posFrac]

	sar	ebp,8
	add_edx	Stereo16StepFrac1

	adc_esi	Stereo16StepWhole1
	add	ecx,ebp

	mov	ebp,[4*ebx]
	mov	[posFrac],edx

	add	ecx,ebp
	;...

	mov	[edi-4],ecx
	cmp_edi	Stereo16LastAddress

	jne	near Stereo16Loop

	; 16c/smp?


	shr	edx,16
	mov	eax,[dsmMixSample]

	shr	eax,2
	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREO16STEREO_I

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

GLOBALLABEL dsmMixStereo16StereoAsmIReloc
	DD	$-dsmMixStereo16StereoAsmI
	DD	Stereo16RelocTable
	DD	3

Stereo16RelocTable:
	DD	Stereo16Reloc1
	DD	Stereo16Reloc2
	DD	Stereo16Reloc3



GLOBALLABEL dsmMixStereo16StereoAsmMI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREO16STEREO_MI, [esp+20]

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
	put_ebx	Stereo16MStepWhole1, Stereo16MReloc1

	shr	esi,2
	put_ebp	Stereo16MStepFrac1, Stereo16MReloc2

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample/4 + position whole

	mov	eax,[dsmMixLeftVolTable]

	shr	eax,2

	mov	ebx,eax
	put_ecx	Stereo16MLastAddress, Stereo16MReloc3

	jmp short Stereo16MLoop

Stereo16MLoop:
	mov	al,[4*esi]
	mov	ecx,[edi]

	add	al,0x80
	mov	bl,[4*esi+1]

	adc	bl,0x80
	add	edi,8

	mov	ebp,[4*eax]
	mov	al,[4*esi+2]

	sar	ebp,8
	;...

	add	ecx,ebp
	mov	ebp,[4*ebx]

	add	ecx,ebp
	add	al,0x80

	mov	bl,[4*esi+3]
	mov	[edi-8],ecx

	adc	bl,0x80
	mov	ebp,[4*eax]

	sar	ebp,8
	mov	ecx,[edi-4]

	add	ecx,ebp
	mov	ebp,[4*ebx]

	add	ecx,ebp
	add_edx	Stereo16MStepFrac1

	adc_esi	Stereo16MStepWhole1
	mov	[edi-4],ecx

	cmp_edi	Stereo16MLastAddress
	jne	Stereo16MLoop

	; 14c/smp?


	shr	edx,16
	mov	eax,[dsmMixSample]

	shr	eax,2
	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREO16STEREO_MI

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

GLOBALLABEL dsmMixStereo16StereoAsmMIReloc
	DD	$-dsmMixStereo16StereoAsmMI
	DD	Stereo16MRelocTable
	DD	3

Stereo16MRelocTable:
	DD	Stereo16MReloc1
	DD	Stereo16MReloc2
	DD	Stereo16MReloc3



GLOBALLABEL dsmMixStereo16StereoAsmSI
	push	ebp
	push	edi
	push	esi
	push	ebx

	M_PROF_START DSMMIX_PROF_STEREO16STEREO_SI, [esp+20]

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
	put_ebx	Stereo16SStepWhole1, Stereo16SReloc1

	shr	esi,2
	put_ebp	Stereo16SStepFrac1, Stereo16SReloc2

	shl	edx,16			; edx = position fraction
	add	esi,eax			; esi = sample/4 + position whole

	mov	eax,[dsmMixLeftVolTable]

	shr	eax,2

	mov	ebx,eax
	put_ecx	Stereo16SLastAddress, Stereo16SReloc3

	jmp short Stereo16SLoop

Stereo16SLoop:
	mov	al,[4*esi]
	mov	ecx,[edi]

	add	al,0x80
	mov	bl,[4*esi+1]

	adc	bl,0x80
	add	edi,8

	mov	ebp,[4*eax]
	mov	al,[4*esi+2]

	sar	ebp,8
	;...

	add	ecx,ebp
	mov	ebp,[4*ebx]

	add	ecx,ebp
	add	al,0x80

	mov	bl,[4*esi+3]
	mov	[edi-8],ecx

	adc	bl,0x80
	mov	ebp,[4*eax]

	sar	ebp,8
	mov	ecx,[edi-4]

	sub	ecx,ebp
	mov	ebp,[4*ebx]

	sub	ecx,ebp
	add_edx	Stereo16SStepFrac1

	adc_esi	Stereo16SStepWhole1
	mov	[edi-4],ecx

	cmp_edi	Stereo16SLastAddress
	jne	Stereo16SLoop

	; 14c/smp?


	shr	edx,16
	mov	eax,[dsmMixSample]

	shr	eax,2
	mov	[dsmMixDest],edi

	sub	esi,eax
	
	shl	esi,16

	or	esi,edx

	mov	[dsmMixSrcPos],esi

	M_PROF_END DSMMIX_PROF_STEREO16STEREO_SI

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret

GLOBALLABEL dsmMixStereo16StereoAsmSIReloc
	DD	$-dsmMixStereo16StereoAsmSI
	DD	Stereo16SRelocTable
	DD	3

Stereo16SRelocTable:
	DD	Stereo16SReloc1
	DD	Stereo16SReloc2
	DD	Stereo16SReloc3






;* $Log: amix_si.nas,v $
;* Revision 1.7.2.2  1997/08/20 17:49:25  pekangas
;* Added assembler optimized versions of stereo integer mixing of
;* 16-bit stereo samples with middle and surround panning
;*
;* Revision 1.7.2.1  1997/08/20 14:07:49  pekangas
;* Added an assembler-optimized 16-bit stereo integer mixing routine
;*
;* Revision 1.7  1997/07/31 16:24:29  pekangas
;* Fixed a sound quality problem not caught by the tests
;*
;* Revision 1.6  1997/07/31 10:56:35  pekangas
;* Renamed from MIDAS Sound System to MIDAS Digital Audio System
;*
;* Revision 1.5  1997/07/30 19:04:02  pekangas
;* Added optimized mono u-law mixing routines and enabled u-law autoconversion
;* by default
;*
;* Revision 1.4  1997/07/30 13:33:44  pekangas
;* Added optimized integer middle mixing routines
;* Optimized integer surround routines
;*
;* Revision 1.3  1997/07/29 11:44:00  pekangas
;* Cleanup made possible by NASM 0.95 release
;*
;* Revision 1.2  1997/07/24 17:10:00  pekangas
;* Added surround support and support for separate surround and middle mixing
;* routines.
;*
;* Revision 1.1  1997/07/24 14:55:34  pekangas
;* Initial revision
;*
