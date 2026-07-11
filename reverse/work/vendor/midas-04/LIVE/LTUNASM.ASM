;*	LTUNASM.ASM
;*
;* S2 The Party '94 64kb intro
;* -- Line tunnel
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*


IDEAL
P386
JUMPS

INCLUDE "lang.inc"
INCLUDE "errors.inc"
INCLUDE "mmem.inc"
INCLUDE "intro.inc"


BUFWIDTH = 240				; buffer width in pixels
BUFHEIGHT = 272 			; buffer height in pixels

DATASEG

linelength	dw	BUFWIDTH

LABEL	linestart WORD
rv0	= 0

REPT	BUFHEIGHT

		dw	rv0

rv0	= rv0 + BUFWIDTH
ENDM


GLOBAL	DoTunnel:FAR
GLOBAL	CopyTweakBuffer:FAR
GLOBAL	SetTweakMode:FAR

CODESEG

PROC	SetTweakMode FAR
USES	di

	cld				; just to make sure...

	mov	ax,0013h		; set mode 13h (standard 320x200x256)
	int	10h

	cli				; disable interrupts for safety

	mov	dx,03C4h		; Sequencer address register
	mov	ax,0604h		; no Chain Four, Sequential mem, >64k
	out	dx,ax

	mov	dx,03D4h		; CRT Controller address register
	mov	al,14h
	out	dx,al
	inc	dx
	in	al,dx
	and	al,0BFh
	out	dx,al

	dec	dx
	mov	al,17h
	out	dx,al
	inc	dx
	in	al,dx
	or	al,40h
	out	dx,al
	dec	dx

	sti

@@DS:	mov	dx,03D4h		; use double-scan
	mov	al,13h
	out	dx,al
	inc	dx
	mov	ax,40
	out	dx,al

	xor	di,di
	mov	ax,0A000h
	mov	es,ax
	mov	cx,16384
	xor	eax,eax
	rep	stosd
	ret
ENDP


PROC	CopyTweakBuffer FAR	bufSeg:word, pageNum:word
USES	ds,si,di
LOCAL	X:word

	mov	ax,[bufSeg]
	mov	ds,ax
	xor	si,si

	mov	ax,0A000h
	mov	bx,[pageNum]
	imul	bx,bx,80*200/16
	add	ax,bx
	mov	es,ax

	mov	bx,272
	mov	[X],(320-272)/2
@@xloop:
	mov	al,02h
	mov	cl,[byte X]		; Sequencer, index 2 (map mask)
	and	cl,3			;  selects the plane(s) where writes
	mov	ah,1			;  will take place
	shl	ah,cl
	mov	dx,03C4h
	out	dx,ax
	mov	cx,[X]
	shr	cx,2
	mov	di,cx

	mov	cl,200
@@yloop:
	mov	al,[ds:si]
	inc	si
	mov	[es:di],al
	add	di,80
	dec	cl
	jnz	@@yloop
	inc	[X]
	dec	bx
	jnz	@@xloop
	ret
ENDP


PROC	DoTunnel	FAR	angle:word, bufSeg:word
USES	si,di,bp

	call	iSinMult, [angle], 99
	push	ax
	call	iCosMult, [angle], 99
	push	ax

	mov	si,[angle]
	add	si,100
	call	iSinMult, si, 99
	push	ax
	call	iCosMult, si, 99
	mov	si,ax
	pop	cx
	pop	bx
	pop	dx
	add	dx,BUFWIDTH/2
	add	cx,BUFWIDTH/2
	add	bx,BUFHEIGHT/2
	add	si,BUFHEIGHT/2

	mov	es,[bufSeg]
	cld

	mov	al,64
	call	line

	ret
ENDP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;± in  -     al  color
;±	     cx  x2
;±	     dx  x1
;±	     bx  y1
;±	     si  y2
;±	     ds  @data
;±	     es  'a000'
;±	     df  '0'
;± corrupts: ah cx dx bx bp si di
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
PROC	line	NEAR
		xor	ah,ah

		cmp	dx,cx
		jb	@@o0
;		 je	 vdraw

		xchg	dx,cx
		xchg	bx,si

@@o0:		mov	bp,[linelength]
		sub	cx,dx
		sub	si,bx
		ja	@@o1
;		 je	 hdraw

		neg	bp
		neg	si

@@o1:		add	bx,bx
		mov	di,[linestart + bx]
		add	di,dx

		cmp	cx,si
		ja	@@cx
		xchg	cx,si
		mov	ah,1

@@cx:		add	si,si
		mov	dx,si
		sub	dx,cx
		mov	bx,dx
		sub	bx,cx
		inc	cx

		or	ah,ah
		jnz	@@ly

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
@@lx:		stosb
		dec	cx
		jcxz	@@x
		or	dx,dx
		js	@@ox
		add	di,bp
		add	dx,bx
		jmp	@@lx
@@ox:		add	dx,si
		jmp	@@lx
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
@@ly:		mov	[es:di],al
		dec	cx
		jcxz	@@x
		add	di,bp
		or	dx,dx
		js	@@oy
		inc	di
		add	dx,bx
		jmp	@@ly
@@oy:		add	dx,si
		jmp	@@ly
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

@@x:		ret

ENDP

END


