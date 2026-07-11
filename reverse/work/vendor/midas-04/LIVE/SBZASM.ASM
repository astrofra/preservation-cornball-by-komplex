;*	SBZASM.ASM
;*
;* S2 The Party '94 64kb intro
;* -- Shadebob zoomer
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



BUFWIDTH = 320				; buffer width in pixels
BUFHEIGHT = 200 			; buffer height in pixels
BOBSIZE = 16				; shadebob size

SRCWIDTH = 310                          ; zoom source width - zooming
SRCHEIGHT = 190                         ; zoom source height




CODESEG


GLOBAL	PutShadebob : far
GLOBAL	ZoomBuffer : far

kala:
	mov	al,[ds:si+1234h]
	mov	ah,[ds:si+1234h]
        mov     [es:di],ax
        mov     [gs:di],ax
        add     di,2

;/***************************************************************************\
;*
;* Macro:	INCR start, num
;*
;* Description: Creates a code with num "inc [ds:bx+x]", from x = start to
;*		x = start+num
;*
;* Input:	start			first value for x
;*		num			number of instructions
;*
;\***************************************************************************/

MACRO	INCR	start, num
x = start
REPT	num
	inc	[byte ds:bx+x]
x = x + 1
ENDM
ENDM


;/***************************************************************************\
;*
;* Function:	 void PutShadebob(ushort bufSeg, int x, int y);
;*
;* Description:  Puts a shadebob into a buffer
;*
;* Input:	 ushort bufSeg		 buffer segment
;*		 int x			 x-coordinate
;*		 int y			 y-coordinate
;*
;\***************************************************************************/

PROC	PutShadebob	FAR	bufSeg : word, x : word, y : word

	mov	ax,BUFWIDTH
	mul	[y]
	mov	bx,[x]
	add	bx,ax

	push	ds

	mov	ax,[bufSeg]
	mov	ds,ax
	mov	cx,BOBSIZE

@@ylp:
a = 0
REPT	BOBSIZE
	inc	[byte ds:bx+a]
a = a + 1
ENDM
	add	bx,BUFWIDTH
	dec	cx
	jnz	@@ylp

	pop	ds

	ret
ENDP




;/***************************************************************************\
;*
;* Function:	 void ZoomBuffer(ushort srcSeg, ushort destSeg,
;*			 int xOffset, int yOffset);
;*
;* Description:  Zoom data from one buffer to another and to screen
;*
;* Input:	 ushort srcSeg		 source buffer segment
;*		 ushort destSeg 	 destination buffer segment
;*		 int xOffset		 zooming center X offset
;*		 int yOffset		 zooming center Y offset
;*
;\***************************************************************************/

PROC	ZoomBuffer	FAR	srcSeg : word, destSeg : word, \
				xOffset : word, yOffset : word
LOCAL   yIncWhole : word, yIncFract : word, y : word, yFract : word, \
        yCount : word
USES	ds,si,di

	mov	ax,[yOffset]
	add	ax,(BUFHEIGHT / 2) - (SRCHEIGHT / 2)
	mov	[y],ax
	mov	[yFract],0
	mov	eax,SRCHEIGHT * 65536
	xor	edx,edx
	mov	ebx,BUFHEIGHT
	div	ebx
	mov	[yIncFract],ax
	shr	eax,16
	mov	[yIncWhole],ax

        mov     ax,cs
        mov     es,ax

        mov     eax,SRCWIDTH * 65536
	xor	edx,edx
	mov	ebx,BUFWIDTH
	div	ebx

        mov     bx,ax
        shr     eax,16
        mov     cx,ax
        mov     ax,(BUFWIDTH / 2) - (SRCWIDTH / 2)
        add     ax,[xOffset]
        xor     dx,dx
        mov     di,offset @@xlp
        mov     si,BUFWIDTH/2

@@makex:
        mov     [es:di+2],ax
        add     dx,bx
        adc     ax,cx
        mov     [es:di+6],ax
        add     dx,bx
        adc     ax,cx
        add     di,17
        dec     si
        jnz     @@makex

	mov	[yCount],BUFHEIGHT
	mov	ds,[srcSeg]
	mov	es,[destSeg]
	xor	di,di

	mov	ax,0A000h
	mov	gs,ax

@@ylp:	mov	ax,BUFWIDTH
	mul	[y]
	mov	si,ax

@@xlp:
	REPT	BUFWIDTH/2

        DB      8Ah, 84h, 34h, 12h              ; mov     al,[ds:si+1234h]
        DB      8Ah, 0A4h, 34h, 12h             ; mov     ah,[ds:si+1234h]
        DB      26h, 89h, 05h                   ; mov     [es:di],ax
        DB      65h, 89h, 05h                   ; mov     [gs:di],ax
        DB      83h, 0C7h, 02h                  ; add     di,2

	ENDM

	mov	ax,[yIncFract]
	add	[yFract],ax
	mov	ax,[yIncWhole]
	adc	[y],ax

	dec	[yCount]
	jnz	@@ylp

	ret
ENDP







END
