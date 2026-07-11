;*      cputype.nas
;*
;* CPU type detection routines
;*
;* $Id: cputype.nas,v 1.1.2.1 1997/09/06 17:18:56 pekangas Exp $
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

intelID		DB	"GenuineIntel"
idString	DB	"            "


CODESEG


; int mCpuIsPPro(void)

GLOBALLABEL mCpuIsPPro

	push	ebx
	push	esi
	push	edi

	; First, check if we have CPUID support (Intel's code, hopefully
	; works)

        pushfd
	pop	eax
	mov	ecx,eax			; get current flags
	xor	eax,200000h		; flip CPUID bit
	push	eax
	popfd				; set back
	pushfd
	pop	eax			; get updated flags
	xor	eax,ecx
	jnz	.havecpuid

	; No CPUID support - certainly not a PPro/PII
	xor	eax,eax
	jmp	.done

.havecpuid:
	; Get vendor ID:

	xor	eax,eax
	cpuid

	mov	[idString],ebx
	mov	[idString+4],edx
	mov	[idString+8],ecx

	mov	esi,intelID
	mov	edi,idString
	mov	ecx,11

	; Check if it is GenuineIntel:

.loop:	mov	dl,[esi+ecx]
	cmp	dl,[edi+ecx]
	jne	.notPPro
	dec	ecx
	jns	.loop

	; OK, it is, check if it supports eax=1, and if so, get model info:
	cmp	eax,1
	jb	.notPPro

	mov	eax,1
	cpuid

	; Is it of the PPro family or higher?
	and	eax,0F00h
	cmp	eax,0600h
	jb	.notPPro

	; Yes, we have a PPro!
	mov	eax,1
	jmp	.done

.notPPro:
	xor	eax,eax

.done:
	pop	edi
	pop	esi
	pop	ebx

	ret


;* $Log: cputype.nas,v $
;* Revision 1.1.2.1  1997/09/06 17:18:56  pekangas
;* Initial revision
;*
