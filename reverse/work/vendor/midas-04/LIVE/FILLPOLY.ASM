;*      FILLPOLY.ASM
;*
;* S2 The Party '94 64kb intro
;* -- Polygon filler for Non Chain Four modes
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*


; Sorry for this terrible code, but this was "adapted" from a polygon filler
; of mine originally written for the 3D engine which was supposed to be used
; in The Square Christmas demo 1992, but which never made it there. So, in
; other words, this code originally was two years old.

; Note that this filler seems to fill an extra pixel at the right edge of
; every polygon. Probably that's the reason I've never seen any black dots
; in my 3D graphics routines... The bad thing, however, is that the letters
; in the writer look very ugly because of this.


IDEAL
P386
JUMPS

INCLUDE "lang.inc"
INCLUDE "intro.inc"


SCRWIDTH = 320
SCRHEIGHT = 200



DATASEG





CODESEG


leftMask        DB      0Fh,0Eh,0Ch,08h
rightMask       DB      0Fh,01h,03h,07h




PUBLIC  FillPoly







PROC    FillPoly  FAR   numpnts : WORD, poly : DWORD, pagestart : WORD, \
                        color : BYTE, lineTable : DWORD
USES	ds,si,di
LOCAL   xmin : word, xmax : word, ymin : word, ymax : word, y : word, \
        lstart : word, col : byte, pstart : word, sx : byte, ux : byte, \
        p1 : word, np1 : word, p2 : word, np2 : word, xs1 : dword, \
        xs2 : dword, yd1 : word, yd2 : word, dx1 : dword, dx2 : dword

	cmp	[numpnts],32
	jna	@@o1
	jmp	@@nov

@@o1:	lds	si,[poly]

	mov	cx,[numpnts]

	mov	[xmin],32767
	mov	[xmax],0
	mov	[ymin],32767
	mov	[ymax],0
	xor	dl,dl
	xor	dh,dh

@@minmax:
        mov     ax,[ds:si]
        add     si,2

	cmp	ax,[xmin]
	jnl	@@no1
	mov	[xmin],ax

@@no1:	cmp	ax,[xmax]
	jng	@@no2
	mov	[xmax],ax

@@no2:  mov     ax,[ds:si]
        add     si,2

	cmp	ax,[ymin]
	jnl	@@no3
	mov	[ymin],ax
	mov	dh,dl


@@no3:	cmp	ax,[ymax]
	jng	@@no4
	mov	[ymax],ax

@@no4:	inc	dl
        dec     cx
        jnz     @@minmax


	cmp	[xmin],319
	jg	@@no01
	cmp	[xmax],0
	jle	@@no01
        cmp     [ymin],199
	jg	@@no01
	cmp	[ymax],0
	jg	@@ok2

@@no01:	jmp	@@nov


@@ok2:
	mov	ax,[ymin]
	cmp	ax,[ymax]
	je	@@nov

	mov	ax,[xmin]
	cmp	ax,[xmax]
	je	@@nov

        cmp     [ymax],199
	jna	@@ok3
        mov     [ymax],199


@@ok3:	mov	cl,dh
	xor	ch,ch
	mov	dx,cx
	mov	[p2],cx
        mov     si,[word poly]
	mov	ax,[ymin]
	mov	di,[numpnts]


@@lft:	inc	cx
	cmp	cx,[numpnts]
	jb	@@1
	xor	cx,cx

@@1:	mov	bx,cx
	shl	bx,2
	cmp	[ds:si+bx+2],ax
	jne	@@01
	mov	dx,cx
	jmp	@@lft

@@01:	mov	[p1],dx

	mov	cx,[p2]
	mov	dx,cx

@@rgt:	dec	cx
	jns	@@2

	mov	cx,[numpnts]
	dec	cx

@@2:	mov	bx,cx
	shl	bx,2
	cmp	[ds:si+bx+2],ax
	jne	@@02
	mov	dx,cx
	jmp	@@rgt

@@02:	mov	[p2],dx

	mov	bx,dx
	shl	bx,2
	mov	ax,[ds:si+bx]
	shl	eax,16
	mov	[dx2],eax


	mov	bx,[p1]
	shl	bx,2

	mov	ax,[ds:si+bx]
	shl	eax,16
	mov	[dx1],eax

	mov	bx,[p1]
	inc	bx
	cmp	bx,[numpnts]
	jb	@@l1

	mov	bx,0

@@l1:	mov	[np1],bx
	shl	bx,2

	mov	ax,[ds:si+bx+2]
	mov	[yd1],ax
	sub	ax,[ymin]
	jnz	@@nz4

	xor	eax,eax
	jmp	@@ll4

@@nz4:  cwde
	mov	ecx,eax

	mov	ax,[ds:si+bx]
	shl	eax,16
	sub	eax,[dx1]

	cdq
	idiv	ecx
@@ll4:	mov	[xs1],eax

	mov	bx,[p2]
	dec	bx
	jns	@@l2

	mov	bx,[numpnts]
	dec	bx

@@l2:	mov	[np2],bx
	shl	bx,2

	mov	ax,[ds:si+bx+2]
	mov	[yd2],ax
	sub     ax,[ymin]
	jnz	@@nz3

	xor	eax,eax
	jmp	@@ll3

@@nz3:	cwde
	mov	ecx,eax

	mov	ax,[ds:si+bx]
	shl	eax,16
	sub	eax,[dx2]

	cdq
	idiv	ecx
@@ll3:	mov	[xs2],eax

        les     di,[lineTable]

	mov	ecx,[dx1]
	mov	edx,[dx2]
	mov	bx,[ymin]


@@loop1:
        test    bx,bx
	js	@@l3
        cmp     bx,SCRHEIGHT
        jge     @@l3

	mov	[sx],0
	mov	[ux],0

	mov	eax,edx
	shr	eax,16
        test    ax,ax
	jns     @@l9

	xor	ax,ax
	inc	[sx]
	jmp	@@l10

@@l9:	cmp	ax,319
        jle     @@l10

	mov	ax,319
	inc	[ux]

@@l10:	mov	[es:di],ax

	mov	eax,ecx
	shr	eax,16
        test    ax,ax
	jns	@@l11

	xor	ax,ax
	inc	[sx]
	jmp	@@l12

@@l11:	cmp	ax,319
	jna	@@l12

	mov	ax,319
	inc	[ux]

@@l12:	mov	[es:di+2],ax

	cmp	[sx],2
	jne	@@s1

	mov	[word ptr es:di+2],-123

@@s1:	cmp	[ux],2
	jne	@@s2

	mov	[word ptr es:di+2],-123

@@s2:	add	di,4

@@l3:	add	ecx,[xs1]
	add	edx,[xs2]

	inc	bx
	cmp	bx,[ymax]
	jng	@@l4
	jmp	@@done1

@@l4:	cmp	bx,[yd1]
	jl	@@l5
	jmp	@@nextp1

@@l5:   cmp	bx,[yd2]
	jl	@@l6
	jmp	@@nextp2

@@l6:   jmp	@@loop1


@@nextp1:
	mov	[y],bx
	push	edx

	mov	bx,[np1]
	shl	bx,2
	mov	cx,[ds:si+bx]
	shl	ecx,16
	mov	[dx1],ecx

	mov	bx,[np1]
	mov	[p1],bx
	inc	bx
	cmp	bx,[numpnts]
	jb	@@l7

	xor	bx,bx

@@l7:	mov	[np1],bx
	shl	bx,2
	mov	ax,[ds:si+bx+2]
	mov	[yd1],ax
	sub	ax,[y]
	jnz	@@nz1

	xor	eax,eax
	jmp	@@ll1

@@nz1:  cwde
	mov	ecx,eax

	mov     ax,[ds:si+bx]
	shl	eax,16
	sub	eax,[dx1]

	cdq
	idiv	ecx

@@ll1:	mov	[xs1],eax

	mov	ecx,[dx1]
	pop	edx
	mov	bx,[y]

	jmp	@@l5


@@nextp2:
	mov	[y],bx
	push	ecx

	mov	bx,[np2]
	shl	bx,2
	mov	dx,[ds:si+bx]
	shl	edx,16
	mov	[dx2],edx

	mov	bx,[np2]
	mov	[p2],bx
	dec	bx
	jns	@@l8

	mov	bx,[numpnts]
	dec	bx

@@l8:	mov	[np2],bx
	shl	bx,2
	mov	ax,[ds:si+bx+2]
	mov	[yd2],ax
	sub	ax,[y]
	jnz	@@nz2

	xor	eax,eax
	jmp	@@ll2

@@nz2:	cwde
	mov	ecx,eax

	mov     ax,[ds:si+bx]
	shl	eax,16
	sub	eax,[dx2]

	cdq
	idiv	ecx
@@ll2:	mov	[xs2],eax

	mov	edx,[dx2]
	pop	ecx
	mov	bx,[y]

	jmp	@@l6




@@done1:

	mov	ax,0A000h
	mov	es,ax

        lds     si,[lineTable]

	mov	bx,[ymin]
        test    bx,bx
	jns	@@heh

	xor	bx,bx

@@heh:
	mov	[y],bx

        mov     ax,SCRWIDTH / 4
	mul	bx		; starting address of the first line in
	add	ax,[pagestart]  ;  display memory
	mov	[lstart],ax

	mov	dx,03C4h
	mov	al,2
	out	dx,al
	inc	dx

@@flines:
	mov	ax,[y]
	cmp	ax,[ymax]
	jna	@@fok1
	jmp	@@fdone

@@fok1:
	mov	ax,[ds:si]
	mov	cx,[ds:si+2]

	cmp	cx,-123
	je	@@fl3

	cmp	ax,cx
	jl	@@ok111

	mov	bx,ax
	mov	ax,cx
	mov	cx,bx

@@ok111:

	inc	cx

	mov	di,ax
	shr     di,2
	add	di,[lstart]

	push	si
	mov	si,ax
	and	si,3
        mov     bh,[leftMask+si]
	mov	si,cx
	and	si,3
        mov     bl,[rightMask+si]
	pop	si

	dec	cx
	and	ax,not 3
	sub	cx,ax
	shr	cx,2
	jnz	@@fl1
	and	bh,bl

@@fl1:	mov	ah,[color]
	mov	al,bh
	out	dx,al
	mov	al,ah
	stosb
	dec	cx
	js	@@fl3
	jz	@@fl2

	mov	al,0Fh
	out	dx,al
	mov	al,ah

        cmp     cx,8
        jb      @@stosb

        test    di,di
        jz      @@stw1
        stosb
        dec     cx

@@stw1:
        shr     cx,1
        rep     stosw
        jnc     @@fl2
        stosb
        jmp     @@fl2

@@stosb:
        rep     stosb

@@fl2:	mov	al,bl
	out	dx,al
	mov	al,ah
	stosb

@@fl3:
	add	si,4
        add     [lstart],SCRWIDTH / 4
	inc	[y]
	jmp	@@flines


@@fdone:
@@nov:
	ret

ENDP





END
