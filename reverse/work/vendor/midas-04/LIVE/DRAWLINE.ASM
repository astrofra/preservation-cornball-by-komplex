;*	DRAWLINE.ASM
;*
;* S2 The Party '94 64kb intro
;* -- Line drawing routines
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
INCLUDE "intro.inc"


SCRWIDTH = 320
SCRHEIGHT = 200



DATASEG

LineColor	DB	?



CODESEG



GLOBAL	DrawLine : far
GLOBAL	DrawLine2 : far
GLOBAL  LineColor : byte


;	Outcodes used by line clipping routine:
;	---------------------------------------
;     ³    ³
; 0101³0001³1001
;     ³    ³
; ÄÄÄÄÅÄÄÄÄÅÄÄÄÄ
;     ³°°°°³
; 0100³0000³1000
;     ³°°°°³
; ÄÄÄÄÅÄÄÄÄÅÄÄÄÄ
;     ³    ³
; 0110³0010³1010
;     ³    ³


;/***************************************************************************\
;*
;* Macro:	Outcode x, y, ocode
;*
;* Description: Calculates an outcode for a pixel
;*
;* Input:	x			x coordinate (register)
;*		y			y coordinate (register)
;*		ocode			destination register for outcode
;*
;\***************************************************************************/

MACRO	Outcode 	x, y, ocode
LOCAL	no1, no2, no3, dox

	xor	ocode,ocode

	test	y,y
	jns	no1
	or	ocode,0001b
	jmp	dox

no1:	cmp	y,SCRHEIGHT
	jl	dox
	or	ocode,0010b

dox:	test	x,x
	jns	no2
	or	ocode,0100b
	jmp	no3

no2:	cmp	x,SCRWIDTH
	jl	no3
	or	ocode,1000b

no3:
ENDM



;/***************************************************************************\
;*
;* Function:	ClipLine
;*
;* Description: Clips the endpoints of a line to the screen
;*
;* Input:	bx			x-coordinate for point 1
;*		cx			y-coordinate for point 1
;*		si			x-coordinate for point 2
;*		di			y-coordinate for point 2
;*
;* Returns:	ax = 1 if the line is completely out of the screen, 0 if not.
;*		line endpoint coordinates are updated
;*
;* Destroys:
;*
;\***************************************************************************/

PROC	ClipLine	NEAR

	push	bp

@@doclip:
	cmp	bx,si			; is the line vertical?
	je	@@vert
	cmp	cx,di
	je	@@horz			; or horizontal?

	Outcode bx,cx,al		; AL = OutCode for first point
	Outcode si,di,ah		; AH = OutCode for second point

	mov	dl,al			; don't draw if the line is fully
	and	dl,ah			; outside the screen (outcodes
	jnz	@@outside		; have at least 1 same bit set

	mov	dl,al
	or	dl,ah			; if both outcodes are zero, the line
	jz	@@done			; is completely on screen

	test	al,al
	jnz	@@cp1
	mov	al,ah			; swap points if point 1 doesn't need
	xchg	bx,si			; clipping
	xchg	cx,di

@@cp1:
	mov	bp,ax			; bp = outcode for point 1

	test	al,0001b
	jz	@@no11

	mov	ax,si			; outcode AND 1 != 0 --> point is
	sub	ax,bx			; above the window
	neg	cx
	imul	cx
	add	cx,di			; x1 = x1 + (x2-x1) * (0-y1) / (y2-y1)
	idiv	cx
	add	bx,ax

	xor	cx,cx			; y1 = 0

	jmp	@@no12


@@no11: test	bp,0010b
	jz	@@no12

	mov	ax,si			; outcode AND 2 != 0 --> point is
	sub	ax,bx			; below the window
	mov	dx,SCRHEIGHT-1
	sub	dx,cx
	imul	dx
	neg	cx
	add	cx,di			; x1 = x1 + (x2-x1) * (yMax-y1)
	idiv	cx			;	/ (y2-y1)
	add	bx,ax

	mov	cx,SCRHEIGHT-1		; y1 = yMax


@@no12: cmp	bx,si
	je	@@horz

	test	bp,0100b
	jz	@@no13

	mov	ax,di			; outcode AND 4 != 0 --> point is
	sub	ax,cx			; to left of the window
	neg	bx
	imul	bx
	add	bx,si
	idiv	bx			; y1 = y1 + (y2-y1) * (0-x1) / (x2-x1)
	add	cx,ax

	xor	bx,bx			; x1 = 0

	jmp	@@doclip


@@no13: test	bp,1000b
	jz	@@doclip

	mov	ax,di			; outcode AND 8 != 0 --> point is
	sub	ax,cx			; to right of the window
	mov	dx,SCRWIDTH-1
	sub	dx,bx
	imul	dx
	neg	bx
	add	bx,si
	idiv	bx			; y1 = y1 + (y2-y1) * (xMax-x1)
	add	cx,ax			;	/ (x2-x1)

	mov	bx,SCRWIDTH-1		; x1 = xMax

	jmp	@@doclip

@@outside:
	mov	ax,1
	jmp	@@exit


@@vert:
	; vertical line
	cmp	cx,di
	jle	@@v1			; swap endpoints if y2<y1
	xchg	cx,di
	xchg	bx,si

@@v1:	test	di,di
	js	@@outside		; is the line completely outside the
	cmp	cx,SCRHEIGHT		; screen?
	jge	@@outside
	cmp	bx,SCRWIDTH		; in X direction?
	jae	@@outside

	cmp	di,SCRHEIGHT
	jl	@@v2
	mov	di,SCRHEIGHT-1

@@v2:	test	cx,cx
	jns	@@done
	xor	cx,cx
	jmp	@@done


@@horz:
	; horizontal line
	cmp	bx,si
	jle	@@h1			; swap endpoints if x2<x1
	xchg	bx,si
	xchg	cx,di

@@h1:	test	si,si
	js	@@outside		; is the line completely outside the
	cmp	bx,SCRWIDTH		; screen?
	jge	@@outside
	cmp	cx,SCRHEIGHT		; in Y direction?
	jae	@@outside

	cmp	si,SCRWIDTH
	jl	@@h2
	mov	si,SCRWIDTH-1

@@h2:	test	bx,bx
	jns	@@done
	xor	bx,bx


@@done:
	xor	ax,ax

@@exit:
	pop	bp

	ret
ENDP



;/***************************************************************************\
;*
;* Function:	 void DrawLine(int x1, int y1, int x2, int y2, int color);
;*
;* Description:  Draws a line to the screen
;*
;* Input:	 int x1 		 x-coordinate for first endpoint
;*		 int y1 		 y-coordinate for first endpoint
;*		 int x2 		 x-coordinate for second endpoint
;*		 int y2 		 y-coordinate for second endpoint
;*		 int color		 color for the line
;*
;\***************************************************************************/

PROC    DrawLine        FAR     x1 : word, y1 : word, x2 : word, y2 : word

USES	si,di

	push	bp

	mov	bx,[x1]
	mov	cx,[y1]
	mov	si,[x2]
	mov	di,[y2]
	call	ClipLine		; clip the line
	test	ax,ax			; fully outside the screen?
	jnz	@@done

	mov	ax,0A000h
	mov	es,ax

	cmp	bx,si
	jle	@@noswap		; swap the endpoints if x1>x2
	xchg	si,bx
	xchg	di,cx

@@noswap:
	mov	ax,SCRWIDTH
	mul	cx			; bp = line start address on screen
	mov	bp,bx
	add	bp,ax

	mov	ax,si			; ax = x2-x1
	sub	ax,bx

	mov	dx,di			; dx = y2-y1
	sub	dx,cx
	mov	di,dx
	mov	bx,SCRWIDTH		; bx = address increment for new y
	test	dx,dx			; if (y2-y1) is zero, the line is
	jz	@@horz			; horizontal
	jns	@@nndy			; di = abs(dx)
	neg	di
	neg	bx

@@nndy:
	test	ax,ax			; if (x2-x1) is zero, the line is
	jz	@@vert			; vertical

	cmp	ax,di			; if abs(y2-y1) >= (x2-x1), y is the
	jbe	@@ymajor		; independent variable (y-major)

	; x-major line:
	mov	cx,ax			; cx = (x2-x1) + 1
	inc	cx			; = number of pixels

	mov	si,di			; si = incr1 = 2 * abs(y2-y1)
	shl	si,1

	sub	di,ax			; di = incr2 = 2 * (abs(y2-y1)
	shl	di,1			;	- (x2-x1))

	mov	dx,si			; dx = decision variable
	sub	dx,ax			; = 2 * (y2-y1) - (x2-x1)

	mov	al,[LineColor]


@@xlp:	and	[es:bp],al
	inc	bp
	test	dx,dx
	js	@@xn

	add	dx,di
	add	bp,bx
	dec	cx
	jnz	@@xlp
	jmp	@@done

@@xn:	add	dx,si
	dec	cx
	jnz	@@xlp
	jmp	@@done



@@ymajor:
	; Y-major line
	mov	cx,di			; cx = abs(y2-y1) + 1
	inc	cx

	mov	si,ax			; si = incr1 = 2 * (x2-x1)
	shl	si,1

	neg	di			; di = incr2 = 2 * ((x2-x1) -
	add	di,ax			;	abs(y2-y1))
	shl	di,1

	neg	dx			; dx = decision variable
	add	dx,si			; = 2 * (x2-x1) - (y2-y1)

	mov	al,[LineColor]


@@ylp:	and	[es:bp],al
	add	bp,bx
	test	dx,dx
	js	@@yn

	inc	bp
	add	dx,di
	dec	cx
	jnz	@@ylp
	jmp	@@done

@@yn:	add	dx,si
	dec	cx
	jnz	@@ylp
	jmp	@@done


@@vert:
	; Vertical line

	mov	cx,di
	inc	cx
	mov	al,[LineColor]

@@vlp:	and	[es:bp],al
	add	bp,bx
	dec	cx
	jnz	@@vlp
	jmp	@@done


@@horz:
	; Horizontal line

	mov	cx,ax
	inc	cx
	mov	al,[LineColor]

@@hlp:	and	[es:bp],al
	inc	bp
	dec	cx
	jnz	@@hlp

@@done:
	pop	bp

	ret
ENDP


PROC    DrawLine2       FAR     x1 : word, y1 : word, x2 : word, y2 : word
USES	si,di

	push	bp

	mov	bx,[x1]
	mov	cx,[y1]
	mov	si,[x2]
	mov	di,[y2]
	call	ClipLine		; clip the line
	test	ax,ax			; fully outside the screen?
	jnz	@@done

	mov	ax,0A000h
	mov	es,ax

	cmp	bx,si
	jle	@@noswap		; swap the endpoints if x1>x2
	xchg	si,bx
	xchg	di,cx

@@noswap:
	mov	ax,SCRWIDTH
	mul	cx			; bp = line start address on screen
	mov	bp,bx
	add	bp,ax

	mov	ax,si			; ax = x2-x1
	sub	ax,bx

	mov	dx,di			; dx = y2-y1
	sub	dx,cx
	mov	di,dx
	mov	bx,SCRWIDTH		; bx = address increment for new y
	test	dx,dx			; if (y2-y1) is zero, the line is
	jz	@@horz			; horizontal
	jns	@@nndy			; di = abs(dx)
	neg	di
	neg	bx

@@nndy:
	test	ax,ax			; if (x2-x1) is zero, the line is
	jz	@@vert			; vertical

	cmp	ax,di			; if abs(y2-y1) >= (x2-x1), y is the
	jbe	@@ymajor		; independent variable (y-major)

	; x-major line:
	mov	cx,ax			; cx = (x2-x1) + 1
	inc	cx			; = number of pixels

	mov	si,di			; si = incr1 = 2 * abs(y2-y1)
	shl	si,1

	sub	di,ax			; di = incr2 = 2 * (abs(y2-y1)
	shl	di,1			;	- (x2-x1))

	mov	dx,si			; dx = decision variable
	sub	dx,ax			; = 2 * (y2-y1) - (x2-x1)

	mov	al,[LineColor]


@@xlp:	or	[es:bp],al
	inc	bp
	test	dx,dx
	js	@@xn

	add	dx,di
	add	bp,bx
	dec	cx
	jnz	@@xlp
	jmp	@@done

@@xn:	add	dx,si
	dec	cx
	jnz	@@xlp
	jmp	@@done



@@ymajor:
	; Y-major line
	mov	cx,di			; cx = abs(y2-y1) + 1
	inc	cx

	mov	si,ax			; si = incr1 = 2 * (x2-x1)
	shl	si,1

	neg	di			; di = incr2 = 2 * ((x2-x1) -
	add	di,ax			;	abs(y2-y1))
	shl	di,1

	neg	dx			; dx = decision variable
	add	dx,si			; = 2 * (x2-x1) - (y2-y1)

	mov	al,[LineColor]


@@ylp:	or	[es:bp],al
	add	bp,bx
	test	dx,dx
	js	@@yn

	inc	bp
	add	dx,di
	dec	cx
	jnz	@@ylp
	jmp	@@done

@@yn:	add	dx,si
	dec	cx
	jnz	@@ylp
	jmp	@@done


@@vert:
	; Vertical line

	mov	cx,di
	inc	cx
	mov	al,[LineColor]

@@vlp:	or	[es:bp],al
	add	bp,bx
	dec	cx
	jnz	@@vlp
	jmp	@@done


@@horz:
	; Horizontal line

	mov	cx,ax
	inc	cx
	mov	al,[LineColor]

@@hlp:	or	[es:bp],al
	inc	bp
	dec	cx
	jnz	@@hlp

@@done:
	pop	bp

	ret
ENDP

END
