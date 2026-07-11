;*      adpcmasm.nas
;*
;* Assembler optimized ADPCM decoding functions
;*
;* $Id: adpcmasm.nas,v 1.1.2.1 1997/08/21 20:26:45 pekangas Exp $
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

DATASEG

EXTERNSYMBOL mAdpcmPredDiffTable
EXTERNSYMBOL mAdpcmStepSizeAdj


DATASEG

lastAddress	DD	0
input		DD	0
output		DD	0
outval		DD	0


CODESEG


;extern void mAdpcmDecodeMono(mAdpcmChannelState *channels, uchar *input,
;                             short *output, unsigned numSamples);

GLOBALLABEL mAdpcmDecodeMono

	push	ebp
	push	ebx
	push	esi
	push	edi

	mov	ebx,[esp+20]	; channels
	mov	esi,[esp+24]	; input

	mov	edi,[esp+28]	; output
	mov	ecx,[esp+32]	; numSamples

	shl	ecx,1
	mov	[input],esi

	add	ecx,edi
	mov	ebp,[ebx+4]	; channels[0].stepSizeIndex

	mov	esi,[ebx]	; channels[0].predValue
	mov	eax,[input]

.loop:
	xor	ebx,ebx
	;...

	shl	ebp,6
	mov	bl,[eax]

	mov	edx,ebx
	shr	ebx,4

	and	edx,0xF
	nop     ;agi

	mov	eax,[mAdpcmPredDiffTable + ebp + 4*ebx]
	and	ebx,7

	shr	ebp,6
	add	esi,eax

	cmp	esi,32767
	jg	near .abovePred1

	cmp	esi,-32768
	jl	near .belowPred1

.pred1Clipped:
	mov	eax,[mAdpcmStepSizeAdj + 4*ebx]
	;...

	add	ebp,eax
	;...

	cmp	ebp,88
	jg	near .aboveStep1

	cmp	ebp,0
	jl	near .belowStep1

.step1Clipped:
	shl	ebp,6
	mov	eax,esi

	shl	eax,16
	nop ;agi

	mov	ebx,[mAdpcmPredDiffTable + ebp + 4*edx]
	and	edx,7

	shr	ebp,6
	add	esi,ebx

	cmp	esi,32767
	jg	near .abovePred2

	cmp	esi,-32768
	jl	near .belowPred2

.pred2Clipped:
	mov	ebx,[mAdpcmStepSizeAdj + 4*edx]
	mov	edx,esi

	add	ebp,ebx
	and	edx,0xFFFF

	cmp	ebp,88
	jg	near .aboveStep2

	cmp	ebp,0
	jl	near .belowStep2

.step2Clipped:
	or	edx,eax
	mov	eax,[input]

	mov	[edi],edx
	inc	eax

	add	edi,4
	mov	[input],eax

	cmp	edi,ecx
	jb	near .loop


	jmp	.done


.abovePred1:
	mov	esi,32767
	jmp	.pred1Clipped
.belowPred1:
	mov	esi,-32767
	jmp	.pred1Clipped

.aboveStep1:
	mov	ebp,88
	jmp	.step1Clipped
.belowStep1:
	mov	ebp,0
	jmp	.step1Clipped

.abovePred2:
	mov	esi,32767
	jmp	.pred2Clipped
.belowPred2:
	mov	esi,-32768
	jmp	.pred2Clipped

.aboveStep2:
	mov	ebp,88
	jmp	.step2Clipped
.belowStep2:
	mov	ebp,0
	jmp	.step2Clipped


.done:
	mov	eax,[esp+20]	; channels

	mov	[eax],esi	; channels[0].predValue
	mov	[eax+4],ebp	; channels[0].stepSizeIndex

	pop	edi
	pop	esi
	pop	ebx
	pop	ebp

	ret



;extern void mAdpcmDecodeStereo(mAdpcmChannelState *channels, uchar *input,
;                               short *output, unsigned numSamples);

GLOBALLABEL mAdpcmDecodeStereo

	push	ebp
	push	ebx
	push	esi
	push	edi

	mov	ebx,[esp+20]	; channels
	mov	esi,[esp+24]	; input

	mov	edi,[esp+28]	; output
	mov	ecx,[esp+32]	; numSamples

	shl	ecx,2
	mov	[output],edi

	add	ecx,edi
	mov	ebp,[ebx+4]	; channels[0].stepSizeIndex

	mov	[lastAddress],ecx
	mov	edi,[ebx]	; channels[0].predValue

	mov	ecx,[ebx+8]	; channels[1].predValue
	mov	[input],esi

	mov	esi,[ebx+12]	; channels[1].stepSizeIndex
	mov	eax,[input]

.loop:
	xor	ebx,ebx
	;...

	shl	ebp,6
	mov	bl,[eax]

	mov	edx,ebx
	shr	ebx,4

	and	edx,0xF
	shl	esi,6

	mov	eax,[mAdpcmPredDiffTable + ebp + 4*ebx]
	and	ebx,7

	shr	ebp,6
	add	edi,eax

	cmp	edi,32767
	jg	near .abovePredLeft

	cmp	edi,-32768
	jl	near .belowPredLeft

.predLeftClipped:
	mov	eax,[mAdpcmStepSizeAdj + 4*ebx]
	mov	ebx,[mAdpcmPredDiffTable + esi + 4*edx]

	add	ebp,eax
	and	edx,7

	cmp	ebp,88
	jg	near .aboveStepLeft

	cmp	ebp,0
	jl	near .belowStepLeft

.stepLeftClipped:
	shr	esi,6
	add	ecx,ebx

	cmp	ecx,32767
	jg	near .abovePredRight

	cmp	ecx,-32768
	jl	near .belowPredRight

.predRightClipped:
	mov	ebx,[mAdpcmStepSizeAdj + 4*edx]
	mov	eax,ecx

	add	esi,ebx
	mov	ebx,edi

	cmp	esi,88
	jg	near .aboveStepRight

	cmp	esi,0
	jl	near .belowStepRight

.stepRightClipped:
	shl	eax,16
	and	ebx,0xFFFF

	or	eax,ebx
	mov	ebx,[output]

	mov	edx,[lastAddress]
	nop

	mov	[ebx],eax
	add	ebx,4

	mov	[output],ebx
	mov	eax,[input]

	inc	eax
	;...

	mov	[input],eax
	cmp	ebx,edx

	jne	near .loop


	jmp	.done


.abovePredLeft:
	mov	edi,32767
	jmp	.predLeftClipped
.belowPredLeft:
	mov	edi,-32767
	jmp	.predLeftClipped

.aboveStepLeft:
	mov	ebp,88
	jmp	.stepLeftClipped
.belowStepLeft:
	mov	ebp,0
	jmp	.stepLeftClipped

.abovePredRight:
	mov	ecx,32767
	jmp	.predRightClipped
.belowPredRight:
	mov	ecx,-32768
	jmp	.predRightClipped

.aboveStepRight:
	mov	esi,88
	jmp	.stepRightClipped
.belowStepRight:
	mov	esi,0
	jmp	.stepRightClipped


.done:
	mov	eax,[esp+20]	; channels

	mov	[eax],edi	; channels[0].predValue
	mov	[eax+4],ebp	; channels[0].stepSizeIndex
	mov	[eax+8],ecx	; channels[1].predValue
	mov	[eax+12],esi	; channels[1].stepSizeIndex

	pop	edi
	pop	esi
	pop	ebx
	pop	ebp

	ret


;* $Log: adpcmasm.nas,v $
;* Revision 1.1.2.1  1997/08/21 20:26:45  pekangas
;* Initial revision
;*
