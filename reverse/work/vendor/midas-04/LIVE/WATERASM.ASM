;*	WATERASM.ASM
;*
;* S2 The Party '94 64kb intro
;* -- "Water"
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

CODESEG

GLOBAL	DrawWater:FAR
GLOBAL	WaveWater:FAR


PROC	WaveWater FAR	bufSeg:word, sinSeg:word, angle:word, angle2:word,\
			inc1:word, inc2:word, inc3:word
USES	ds, si, di
LOCAL	temp:word, temp2:word

	mov	es,[bufSeg]
	xor	si,si
	mov	ds,[sinSeg]


	mov	bx,[angle]
	mov	[temp],bx
	shr	bx,4
	xor	bh,bh

	mov	di,[angle2]
	and	di,255
	mov	[temp2],di

	mov	dl,[byte inc1]

	mov	ch,255
@@kala:
	mov	cl,25
@@kala2:
a = 0
	rept	8
	mov	al,[ds:bx]
	add	bl,dl
	add	al,[ds:di+a]
	mov	[es:si+a],al
a = a + 1
	endm
	add	si,8
	add	di,8
	dec	cl
	jnz	@@kala2

	mov	bx,[temp]
	add	bx,[inc2]
	mov	[temp],bx
	shr	bx,4
	xor	bh,bh

	mov	di,[temp2]
	add	di,[inc3]
	and	di,255
	mov	[temp2],di

	dec	ch
	jnz	@@kala

	mov	ch,320-255
@@kala3:
	mov	cl,25
@@kala4:
a = 0
	rept	8
	mov	al,[ds:bx]
	add	bl,dl
	add	al,[ds:di+a]
	mov	[es:si+a],al
a = a + 1
	endm
	add	si,8
	add	di,8
	dec	cl
	jnz	@@kala4

	mov	bx,[temp]
	add	bx,[inc2]
	mov	[temp],bx
	shr	bx,4
	xor	bh,bh

	mov	di,[temp2]
	add	di,[inc3]
	and	di,255
	mov	[temp2],di

	dec	ch
	jnz	@@kala3
	ret
ENDP



PROC    DrawWater FAR   bufSeg:word, pageNum:word, height : word
LOCAL	bitti:word
USES	ds,si,di

	mov	ax,0a000h
	mov	bx,[pageNum]
	imul	bx,bx,80*200/16
	add	ax,bx
	mov	es,ax

	mov	di,80*199

	mov	ax,[bufSeg]
	mov	ds,ax
	xor	si,si

	mov	[bitti],1
@@pixelloop:
	mov	al,2
	mov	ah,[byte bitti]
	mov	dx,03C4h
	out	dx,ax

	mov	ch,80
@@xloop:
	mov	cl,200
        xor     dx,dx                           ; Base height for "perspective"
        sub     dx,[height]
	xor	bx,bx				; "Now" height
@@yloop:
	movsx	ax,[byte ds:si]
	add	ax,dx
	inc	si
	cmp	ax,bx
	jle	@@nope

	sub	ax,bx
	add	bx,ax
@@fill:
	mov	[es:di],cl
	sub	di,80
	dec	ax
	jnz	@@fill

@@nope: dec	cl

	movsx	ax,[byte ds:si]
	add	ax,dx
	inc	si
	cmp	ax,bx
	jg     @@huu

	inc	dx

	dec	cl
	jnz	@@yloop
	jmp	@@jup

@@huu:	sub	ax,bx
	add	bx,ax
@@fill2:
	mov	[es:di],cl
	sub	di,80
	dec	ax
	jnz	@@fill2

@@nope2:
	inc	dx

	dec	cl
	jnz	@@yloop

@@jup:  mov     ax,120
	sub	ax,bx

@@fill3:
	mov	[es:di],cl
	sub	di,80
	dec	ax
	jnz	@@fill3

        add     di,80*120+1

	add	si,200*3

	dec	ch
	jnz	@@xloop

	shl	[bitti],1
	sub	si,200*4*80-200
	sub	di,80

	cmp	[bitti],10h
	jne	@@pixelloop
	ret
ENDP

END

