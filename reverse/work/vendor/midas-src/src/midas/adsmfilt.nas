;*      adsmfilt.nas
;*
;* Digital Sound Mixer, assembler filtering
;*
;* $Id: adsmfilt.nas,v 1.1.2.2 1997/09/08 16:54:17 pekangas Exp $
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

d1l	DD	0
d1r	DD	0
d2l	DD	0
d2r	DD	0
lastAddr DD	0
_1_per_3 DD	0.333333333333333333333333333
d1lf	DD	0.0
d1rf	DD	0.0
_2	DD	2.0

CODESEG

;extern void CALLING dsmFilterIntStereo1(void *data, unsigned numSamples);

;                s = d1l + ((*p) << 1);
;                s = (s >> 2) + (s >> 3) - (s >> 5);
;                d1l = *p;
;                *(p++) = s;


GLOBALLABEL dsmFilterIntStereo1

	push	ebx
	push	edi
	push	esi
	push	ebp

	mov	edi,[esp+20]
	mov	ecx,[esp+24]

	shl	ecx,3

	add	ecx,edi

	mov	[lastAddr],ecx
	mov	ecx,[d1l]

.loop:
	mov	eax,[edi]	; eax = *p
	mov	edx,ecx		; edx = d1l

	mov	ecx,eax		; d1l = *p
	mov	ebx,[edi+4]	; ebx = *(p+1)

	shl	eax,1		; eax = 2 * (*p)
	mov	esi,[d1r]	; esi = d1r

	mov	[d1r],ebx	; d1r = *(p+1)
	add	eax,edx		; eax = s = d1l + 2*(*p)

	shl	ebx,1		; ebx = 2*(*(p+1))
	mov	edx,eax

	sar	eax,2		; eax = s >> 2
	mov	ebp,edx

	sar	edx,3		; edx = s >> 3;
	add	ebx,esi		; ebx = sr = d1r * 2*(*(p+1))

	sar	ebp,5		; ebp = s >> 5
	add	eax,edx		; eax = (s >> 2) + (s >> 3)

	sub	eax,ebp		; eax = (s >> 2) + (s >> 3) - (s >> 5)
	mov	edx,ebx

	sar	ebx,2		; ebx = sr >> 2
	mov	ebp,edx

	sar	edx,3		; edx = sr >> 3
	mov	[edi],eax	; *p = s

	sar	ebp,5		; ebx = sr >> 5
	add	ebx,edx		; ebx = (sr >> 2) + (sr >> 3)

	sub	ebx,ebp		; ebx = (sr >> 2) + (sr >> 3) - (sr >> 5)
	mov	eax,[lastAddr]

	mov	[edi+4],ebx
	add	edi,8

	cmp	edi,eax
	jne	.loop

	mov	[d1l],ecx

	pop	ebp
	pop	esi
	pop	edi
	pop	ebx

	ret



GLOBALLABEL dsmFilterIntMono1

	push	ebx
	push	edi
	push	esi
	push	ebp

	mov	edi,[esp+20]
	mov	ecx,[esp+24]

	mov	eax,ecx

	shl	ecx,2

	add	ecx,edi

	test	eax,1
	jz	.loop

	; Do the odd one first if necessary:

	mov	eax,[edi]	; eax = *p
	mov	edx,[d1l]	; edx = d1l
	mov	[d1l],eax	; d1l = *p
	shl	eax,1		; eax = 2 * (*p)
	add	eax,edx		; eax = s = d1l + 2*(*p)
	mov	edx,eax
	sar	eax,2		; eax = s >> 2
	mov	ebp,edx
	sar	edx,3		; edx = s >> 3;
	add	ebx,esi		; ebx = sr = d1r * 2*(*(p+1))
	sar	ebp,5		; ebp = s >> 5
	add	eax,edx		; eax = (s >> 2) + (s >> 3)
	sub	eax,ebp		; eax = (s >> 2) + (s >> 3) - (s >> 5)
	mov	[edi],eax	; *p = s
	add	edi,4

	cmp	edi,ecx
	jae	.pois


.loop:
	; Two at a pop

	mov	eax,[edi]	; eax = *p
	mov	edx,[d1l]	; edx = d1l

	mov	esi,eax		; esi = d1l = *p
	mov	ebx,[edi+4]	; ebx = *(p+1)

	shl	eax,1		; eax = 2 * (*p)
	mov	[d1l],ebx	; d1r = *(p+1)

	shl	ebx,1		; ebx = 2*(*(p+1))
	add	eax,edx		; eax = s = d1l + 2*(*p)

	mov	edx,eax
	mov	ebp,eax

	sar	eax,2		; eax = s >> 2
	add	ebx,esi		; ebx = s1 = d1l * 2*(*(p+1))

	sar	edx,3		; edx = s >> 3;
	;...

	sar	ebp,5		; ebp = s >> 5
	add	eax,edx		; eax = (s >> 2) + (s >> 3)

	sub	eax,ebp		; eax = (s >> 2) + (s >> 3) - (s >> 5)
	mov	edx,ebx

	sar	ebx,2		; ebx = s1 >> 2
	mov	ebp,edx

	sar	edx,3		; edx = s1 >> 3
	mov	[edi],eax	; *p = s

	sar	ebp,5		; ebx = s1 >> 5
	add	ebx,edx		; ebx = (s1 >> 2) + (sr >> 3)

	sub	ebx,ebp		; ebx = (s1 >> 2) + (sr >> 3) - (sr >> 5)
	;...

	mov	[edi+4],ebx
	add	edi,8

	cmp	edi,ecx
	jne	.loop

.pois:
	pop	ebp
	pop	esi
	pop	edi
	pop	ebx

	ret



GLOBALLABEL dsmFilterFloatStereo1

	push	ebx
	push	edi
	push	esi
	push	ebp

	mov	edi,[esp+20]
	mov	ecx,[esp+24]

	shl	ecx,3

	add	ecx,edi

	fld	dword [d1lf]	; d1l
	fld	dword [d1rf]	; d1r, d1l
	fld	dword [edi]	; *p, d1r, d1l
	jmp	.start

.loop:
	; d1r+2*(*(p+1))(2), d1r, destl(3), d1l
	fmul	dword [_1_per_3] ; destr(0), d1r, destl, d1l
	fxch	st2,st0
	fstp	dword [edi-8]	; d1r, destr(2), d1l
	fld	dword [edi]	; *p, d1r, destr(3), d1l
	fxch	st2,st0
	fstp	dword [edi-4]	; d1r, *p, d1l
	fxch	st1,st0		; *p, d1r, d1l
	; 7
.start:
	fmul	dword [_2]	; 2*(*p)(0), d1r, d1l
	fld	dword [edi]	; *p, 2*(*p)(1), d1r, d1l
	fxch	st0,st3		; d1l = *p -> d1l, 2*(*p)(1), d1r, d1l
	fld	dword [edi+4]	; *(p+1), d1l, 2*(*p)(2), d1r, d1l
	fxch	st2,st0
	faddp	st1,st0		; d1l+2*(*p)(0), *(p+1), d1r, d1l
	fxch	st1,st0
	fmul	dword [_2]	; 2*(*(p+1))(0), d1l+2*(*p)(1), d1r, d1l
	fld	dword [edi+4]	; *(p+1),2*(*(p+1))(1),d1l+2*(*p)(2),d1r,d1l
	fxch	st2,st0
	fmul	dword [_1_per_3] ; destl(0), 2*(*(p+1))(2), *(p+1), d1r, d1l
	fxch	st3,st0
	faddp	st1,st0		; d1r+2*(*(p+1))(0), d1r, destl(1), d1l

	add	edi,8		; d1r+2*(*(p+1))(1), d1r, destl(2), d1l
	; tuut

	cmp	edi,ecx		; d1r+2*(*(p+1))(2), d1r, destl(3), d1l
	jb	.loop

	; 17c/smp?

	; d1r+2*(*(p+1))(2), d1r, destl(3), d1l
	fmul	dword [_1_per_3] ; destr(0), d1r, destl, d1l
	fxch	st2,st0
	fstp	dword [edi-8]	; d1r, destr(2), d1l
	fxch	st1,st0		; destr(3), d1r, d1l
	fstp	dword [edi-4]	; d1r, d1l

	fstp	dword [d1rf]
	fstp	dword [d1lf]

	pop	ebp
	pop	esi
	pop	edi
	pop	ebx

	ret



GLOBALLABEL dsmFilterFloatMono1

	push	ebx
	push	edi
	push	esi
	push	ebp

	mov	edi,[esp+20]
	mov	ecx,[esp+24]

	mov	eax,ecx
	shl	ecx,2

	add	ecx,edi

	fld	dword [d1lf]	; d1l

	test	eax,1
	jz	.mainloop

	; Do the odd one first if necessary:

	fld	dword [edi]	; *p, d1l
	fmul	dword [_2]	; 2*(*p)(0), d1l
	fld	dword [edi]	; *p, 2*(*p)(1), d1l
	fxch	st2,st0
	faddp	st1,st0		; d1l+2*(*p)(0), d1l
	fmul	dword [_1_per_3] ; dest(0), d1l
	fstp	dword [edi]	; d1l
	add	edi,4

	cmp	edi,ecx
	jae	.done

.mainloop:
	fld	dword [edi]
	jmp	.start

.loop:
	; dest(2), d1l+2pn, d1l
	fld	dword [edi]	; *p, dest(3), d1l+2pn, d1l
	fxch	st2,st0
	fmul	dword [_1_per_3] ; destn(0), dest(3), *p, d1l
	fxch	st1,st0
	fstp	dword [edi-8]	; destn(2), *p, d1l
	; stall (eek)
	fstp	dword [edi-4]	; *p, d1l
	; 6
.start:
	; *p, d1l
	fmul	dword [_2]	; 2*(*p)(0), d1l
	fld	dword [edi]	; *p, 2*(*p)(1), d1l
	fxch	st2,st0		; d1l, 2*(*p)(1), d1ln
	fld	dword [edi+4]	; *(p+1), d1l, 2*(*p)(2), d1ln
	fmul	dword [_2]	; 2*(*(p+1))(0), d1l, 2*(*p), d1ln
	fxch	st1,st0
	faddp	st2,st0		; 2*(*(p+1))(1), d1l+2p(0), d1l
	fld	dword [edi+4]	; *(p+1), 2*(*(p+1))(2), d1l+2p(1), d1l
	fxch	st3,st0
	faddp	st1,st0		; d1l+2pn(0), d1l+2p(2), d1l
	fxch	st1,st0
	fmul	dword [_1_per_3] ; dest(0), d1l+2pn(1), d1l

	add	edi,8		; dest(1), d1l+2pn(2), d1l
	; tuut

	cmp	edi,ecx		; dest(2), d1l+2pn, d1l
	jb	.loop

	; 17c/2smp -> 8.5c/smp?

	; dest(2), d1l+2pn, d1l
	fstp	dword [edi-8]
	fmul	dword [_1_per_3]
	fstp	dword [edi-4]

.done:
	fstp	dword [d1lf]

	pop	ebp
	pop	esi
	pop	edi
	pop	ebx

	ret




;* $Log: adsmfilt.nas,v $
;* Revision 1.1.2.2  1997/09/08 16:54:17  pekangas
;* Added hand-optimized versions of floating point filter 1 functions
;*
;* Revision 1.1.2.1  1997/09/06 17:15:33  pekangas
;* Fixed slowdown problems with PPro optimizations
;*
;* Revision 1.1  1997/08/19 17:21:15  pekangas
;* Initial revision
;*
