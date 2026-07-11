;*      adsmmix.nas
;*
;* Digital Sound Mixer, assembler clearing and conversion functions
;*
;* $Id: adsmmix.nas,v 1.4 1997/07/31 10:56:35 pekangas Exp $
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

foo	DD	0


CODESEG



;void CALLING dsmConvertFloat16(unsigned numSamples, S16 *dest)
;{
;    float *buf = (float*) dsmMixBuffer;
;    int n;
;
;    while ( numSamples )
;    {
;        n = (int) (*(buf++));
;        if ( n < -32768 )
;            n = -32768;
;        if ( n > 32767 )
;            n = 32767;
;        *dest++ = (S16) n;
;        numSamples--;
;    }
;}


GLOBALLABEL dsmConvertFloat16

	push	ebx
	
	mov	edx,[dsmMixBuffer]	
	mov	ebx,[esp+12]

	mov	ecx,[esp+8]

	shl	ecx,1
	
	add	ecx,ebx

.loop:
	fld	dword [edx]

	fistp	dword [foo]

	mov	eax,[foo]

	cmp	eax,32767
	jg	.above

	cmp	eax,-32768
	jl	.below

	mov	word [ebx],ax

	add	ebx,2
	add	edx,4

	cmp	ebx,ecx
	jb	.loop

	jmp	.done

.above:
	mov	word [ebx],32767

	add	ebx,2
	add	edx,4

	cmp	ebx,ecx
	jb	.loop

	jmp	.done


.below:
	mov	word [ebx],-32768

	add	ebx,2
	add	edx,4

	cmp	ebx,ecx
	jb	.loop

.done:
	pop	ebx
	ret



;void CALLING dsmConvertInt16(unsigned numSamples, S16 *dest)
;{
;    int *buf = (int*) dsmMixBuffer;
;    int n;
;
;    while ( numSamples )
;    {
;        n = *(buf++);
;        if ( n < -32768 )
;            n = -32768;
;        if ( n > 32767 )
;            n = 32767;
;        *dest++ = (S16) n;
;        numSamples--;
;    }
;}

GLOBALLABEL dsmConvertInt16

	push	ebx
	push	edi
	push	esi
	
	mov	edx,[dsmMixBuffer]	
	mov	edi,[esp+20]

	mov	ecx,[esp+16]

	test	edi,2
	jz	.ok1

	call	.doOne

	dec	ecx

.ok1:
	mov	esi,ecx
	and	ecx,0xFFFFFFFE

	jz	near .done

	shl	ecx,1

	add	ecx,edi

.loop:
	mov	eax,[edx]
	mov	ebx,[edx+4]

	cmp	eax,32767
	jg	.above1

	cmp	eax,-32768
	jl	.below1

	cmp	ebx,32767
	jg	near .above2

	cmp	ebx,-32768
	jl	near .below2

	shl	ebx,16
	and	eax,0xFFFF

	or	eax,ebx
	add	edx,8

	mov	[edi],eax
	add	edi,4

	cmp	edi,ecx
	jb	.loop

	jmp	.done

.above1:
	mov	eax,32767

	cmp	ebx,32767
	jg	.above2

	cmp	ebx,-32768
	jl	near .below2

	shl	ebx,16
	and	eax,0xFFFF

	or	eax,ebx
	add	edx,8

	mov	[edi],eax
	add	edi,4

	cmp	edi,ecx
	jb	near .loop

	jmp	.done


.below1:
	mov	eax,-32768

	cmp	ebx,32767
	jg	.above2

	cmp	ebx,-32768
	jl	.below2

	shl	ebx,16
	and	eax,0xFFFF

	or	eax,ebx
	add	edx,8

	mov	[edi],eax
	add	edi,4

	cmp	edi,ecx
	jb	near .loop

	jmp	.done


.above2:
	mov	ebx,32767

	shl	ebx,16
	and	eax,0xFFFF

	or	eax,ebx
	add	edx,8

	mov	[edi],eax
	add	edi,4

	cmp	edi,ecx
	jb	near .loop

	jmp	.done


.below2:
	mov	ebx,-32768

	shl	ebx,16
	and	eax,0xFFFF

	or	eax,ebx
	add	edx,8

	mov	[edi],eax
	add	edi,4

	cmp	edi,ecx
	jb	near .loop

	jmp	.done


.done:
	test	esi,1
	jz	.quit

	call	.doOne

	jmp	.quit

.doOne:
	mov	eax,[edx]
	add	edx,4

	cmp	eax,32767
	jg	.aboveo

	cmp	eax,-32768
	jl	.belowo

	mov	[edi],ax
	add	edi,2

	ret

.aboveo:
	mov	word [edi],32767
	add	edi,2

	ret

.belowo:
	mov	word [edi],-32768
	add	edi,2

	ret


.quit:
	pop	esi
	pop	edi
	pop	ebx
	ret






;* $Log: adsmmix.nas,v $
;* Revision 1.4  1997/07/31 10:56:35  pekangas
;* Renamed from MIDAS Sound System to MIDAS Digital Audio System
;*
;* Revision 1.3  1997/07/30 16:19:36  pekangas
;* Added optimized 16-bit integer conversion
;*
;* Revision 1.2  1997/07/17 19:35:12  pekangas
;* The same assembler source should now compile to Win32, DOS and Linux
;* without modifications
;*
;* Revision 1.1  1997/06/02 15:22:54  pekangas
;* Initial revision
;*




