;*	DMA.ASM
;*
;* DMA handling routines, v1.11
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
INCLUDE "dma.inc"
INCLUDE "mmem.inc"
INCLUDE "mglobals.inc"

IFDEF __DPMI__
INCLUDE "dpmi.inc"
ENDIF


DATASEG

bptr            DD      ?
vdsUsed         DW      ?               ; is Virtual DMA used?



IDATASEG



;/***************************************************************************\
;*     DMA channel data
;\***************************************************************************/

dmaChns dmaChannel      < 0, 1, 000h, 001h, 009h, 00Ah, 00Bh, 00Ch, 087h >
	dmaChannel	< 1, 2, 002h, 003h, 009h, 00Ah, 00Bh, 00Ch, 083h >
	dmaChannel	< 2, 4, 004h, 005h, 009h, 00Ah, 00Bh, 00Ch, 081h >
	dmaChannel	< 3, 8, 006h, 007h, 009h, 00Ah, 00Bh, 00Ch, 082h >
	dmaChannel	< 4, 1, 0C0h, 0C2h, 0D2h, 0D4h, 0D6h, 0D8h, 08Fh >
	dmaChannel	< 5, 2, 0C4h, 0C6h, 0D2h, 0D4h, 0D6h, 0D8h, 08Bh >
	dmaChannel	< 6, 4, 0C8h, 0CAh, 0D2h, 0D4h, 0D6h, 0D8h, 089h >
	dmaChannel	< 7, 8, 0CCh, 0CEh, 0D2h, 0D4h, 0D6h, 0D8h, 08Ah >



CODESEG



;/***************************************************************************\
;*
;* Function:     int dmaAllocBuffer(ushort size, dmaBuffer *buf);
;*
;* Description:  Allocates a DMA buffer (totally inside a 64K physical page)
;*
;* Input:        ushort size             size of buffer in bytes
;*               dmaBuffer *buf          ptr to buffer structure to be filled
;*
;* Returns:      MIDAS error code.
;*               DMA buffer data is strored in *buf.
;*
;\***************************************************************************/

PROC    dmaAllocBuffer  FAR     bsize : word, buf : dword
USES    di,si

	cmp	[bsize],32000		; buffer size must not be > 32000
        jbe     @@bok                   ; bytes
        mov     ax,errInvalidArguments
        jmp     @@err

@@bok:
IFDEF __REALMODE__
        cmp     [useVDS],0              ; should VDS be used?
        je      @@novds

        mov     ax,0040h
        mov     es,ax                   ; if bit 5 of the byte at 0040h:007Bh
        mov     al,[es:007Bh]           ; is clear, VDS is not present
        test    al,32
        jz      @@novds

        mov     ax,8102h                ; get VDS version
        xor     dx,dx
        int     4Bh
        jnc     @@vdsnerr               ; carry set if error

        mov     ax,errVDSUsage          ; unable to use VDS
        jmp     @@err

@@vdsnerr:
        cmp     ax,0100h                ; version >= 1.00?
        jae     @@vdsverok
        mov     ax,errBadVDS            ; no - invalid VDS version error
        jmp     @@err

@@vdsverok:
        mov     [vdsUsed],1             ; VDS version OK - VDS will be used

	mov	ax,[bsize]		; ax = buffer size
        add     ax,64                   ; add 64 bytes for alignment purposes
        shl     ax,1                    ; multiply by 2 to ensure the buffer
                                        ; can be fit between 64k boundaries

        ; Allocate memory for buffer:
        les     bx,[buf]
        lea     dx,[bx+dmaBuffer.memBlk]  ; point es:dx to buffer.memBlk
        push    es bx
        call    memAlloc LANG, ax, es dx  ; allocate the memory
        pop     bx es
        test    ax,ax                   ; error? (ax non-zero)
        jnz     @@err

        xor     eax,eax
        mov     dx,[word es:bx+2+dmaBuffer.memBlk]      ; point dx:ax to
        mov     ax,[word es:bx+dmaBuffer.memBlk]        ; buffer memory

        ; point es:di to VDS DDS structure:
        lea     di,[es:bx+dmaBuffer.vdsDMADDS]

        ; put buffer segment:offset in DDS structure:
        mov     [es:di+vdsDDS.bufferOffset],eax
        mov     [es:di+vdsDDS.bufferSegment],dx

        ; put buffer size to DDS stucture:
        mov     ax,[bsize]
        add     ax,64
        shl     ax,1
        mov     [es:di+vdsDDS.regionSize],eax

        mov     ax,8103h                ; VDS Lock DMA Buffer function
        mov     dx,00100b
        int     4Bh
        jnc     @@vdsallocok

@@allocerr:
        mov     ax,errVDSLock           ; unable to allocate VDS DMA buffer
        jmp     @@err

@@vdsallocok:
        mov     esi,[es:di+vdsDDS.physicalAddr]
        mov     dx,[word es:bx+2+dmaBuffer.memBlk]      ; point dx:ax to
        mov     ax,[word es:bx+dmaBuffer.memBlk]        ; buffer memory

        shr     ax,4
	add	dx,ax			; dx = buffer segment
	inc	dx			; (segment + offset / 16 + 1)

        and     esi,0FFFFFFF0h          ; esi = 16 * (address / 16 + 1)
        add     esi,000000010h

        mov     eax,esi                 ; ax = address AND 0FFFFh
        and     eax,0FFFFh

        mov     cx,[bsize]              ; cx = -size
        neg     cx

        cmp     ax,cx                   ; does buffer cross a 64K page
        jb      @@ook                   ; boundary? (ax >= cx)

        mov     ecx,esi
        and     esi,0F0000h             ; yes, move it to the beginning
        add     esi,010000h             ; of next page
        and     ecx,0FFFFh
        shr     ecx,4
        add     dx,1000h
        sub     dx,cx

@@ook:
        ; set buffer physical address to dmaBuffer.address field
        mov     [es:bx+dmaBuffer.address],esi
        mov     [es:bx+dmaBuffer.bsegment],dx   ; store segment

        jmp     @@allocdone

@@novds:
ENDIF
        mov     [vdsUsed],0             ; VDS is not used
	mov	ax,[bsize]		; ax = buffer size
	shl	ax,1			; multiply by two so that it can
        add     ax,64                   ; always be fitted inside one 64K
                                        ; page and add 64 for segment
                                        ; alignment

IFDEF __REALMODE__
        ; Allocate memory for buffer:
        les     bx,[buf]
        lea     dx,[bx+dmaBuffer.memBlk]  ; point es:dx to buffer.memBlk
        push    es bx
        call    memAlloc LANG, ax, es dx  ; allocate the memory
        pop     bx es
        test    ax,ax                   ; error? (ax non-zero)
        jnz     @@err

        mov     dx,[word es:bx+2+dmaBuffer.memBlk]      ; point dx:ax to
        mov     ax,[word es:bx+dmaBuffer.memBlk]        ; buffer memory

	shr	ax,4
	add	dx,ax			; dx = buffer segment
	inc	dx			; (segment + offset / 16 + 1)

	mov	ax,dx			; ax = segment AND 0FFFh
	and	ax,0FFFh

	mov	cx,[bsize]
	add	cx,15
	shr	cx,4			; cx = 0FFFh - (size + 15) / 16
	neg	cx
        add     cx,0FFFh

        cmp     ax,cx                   ; does buffer cross a 64K page
	jb	@@ok			; boundary? (ax >= cx)

	and	dx,0F000h		; yes, move it to the beginning of
	add	dx,01000h		; next page

@@ok:	mov	[es:bx+dmaBuffer.bsegment],dx	; store buffer segment
	movzx	eax,dx
	shl	eax,4
	mov	[es:bx+dmaBuffer.address],eax	; address = segment * 16
ELSE
IFDEF __DPMI__
        ; Allocate DOS memory for buffer:
        les     bx,[buf]
        lea     dx,[bx+dmaBuffer.memBlk]  ; point es:dx to buffer.memBlk
        mov     cx,dx
        add     cx,2                    ; point es:cx to buffer.memBlk+2
        add     ax,0Fh                  ; number of paragraphs
        shr     ax,4
        push    es bx
        call    dpmiAllocDOSMem LANG, ax, es dx, es cx
        pop     bx es
        test    ax,ax
        jnz     @@err

        ; Note! For Protected mode under DPMI the dmaBuffer memBlk
        ; field contains the real mode segment and protected mode selector.

        ; Get allocated memory block linear start address:
        push    es bx
        call    dpmiGetSegmentBase LANG, [word es:bx+2+dmaBuffer.memBlk], \
                seg bptr offset bptr
        pop     bx es
        test    ax,ax
        jnz     @@err
        mov     eax,[bptr]                              ; segment base

        add     eax,0Fh                 ; align to paragraph boundary
        and     eax,not 0Fh

        mov     esi,eax
        and     eax,0FFFFh              ; eax = address and 0FFFFh

        mov     cx,[bsize]              ; cx = -size
        neg     cx

        cmp     ax,cx                   ; does buffer cross a 64K page
        jb      @@okd                   ; boundary? (ax >= cx)

        and     esi,0F0000h             ; yes, move it to the beginning
        add     esi,010000h             ; of next page

@@okd:
        ; esi is now the buffer linear start address
        mov     [es:bx+dmaBuffer.address],esi

        ; Allocate descriptor for DMA buffer:
        push    es bx
        lea     dx,[bx+dmaBuffer.bsegment]  ; point es:dx to buffer.segment
        call    dpmiAllocDescriptor LANG, es dx
        pop     bx es
        test    ax,ax
        jnz     @@err

        ; Set DMA buffer segment base address:
        push    es bx
        call    dpmiSetSegmentBase LANG, [es:bx+dmaBuffer.bsegment], esi
        pop     bx es
        test    ax,ax
        jnz     @@err

        ; Set DMA buffer segment limit:
        xor     eax,eax
        mov     ax,[bsize]
        dec     eax
        push    es bx
        call    dpmiSetSegmentLimit LANG, [es:bx+dmaBuffer.bsegment], eax
        pop     bx es
        test    ax,ax
        jnz     @@err
ENDIF
ENDIF

@@allocdone:
	mov	ax,[bsize]			; store buffer length
	mov	[es:bx+dmaBuffer.blength],ax

	mov	[es:bx+dmaBuffer.channel],-1	; buffer is not being played

        xor     ax,ax                   ; allocation succesful
        jmp     @@done

@@err:
        ERROR ID_dmaAllocBuffer

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:     int dmaFreeBuffer(dmaBuffer *buf);
;*
;* Description:  Deallocates an allocated DMA buffer
;*
;* Input:        dmaBuffer *buf          ptr to buffer to be deallocated
;*
;* Returns:      MIDAS error code
;*
;\***************************************************************************/

PROC	dmaFreeBuffer	FAR	buf : dword
USES    di

        cmp     [vdsUsed],1             ; is VDS used?
        jne     @@novds

        ; point es:di to VDS DDS stucture:
        les     bx,[buf]
        lea     di,[es:bx+dmaBuffer.vdsDMADDS]

        mov     ax,8104h                ; VDS Unlock DMA Buffer Region
        xor     dx,dx
        int     4Bh
        jnc     @@novds

        mov     ax,errVDSUsage
        jmp     @@err


@@novds:
IFDEF __REALMODE__
        les     bx,[buf]                ; point es:bx to buffer
	call	memFree LANG, [es:bx+dmaBuffer.memBlk]	; free buffer memory
        test    ax,ax
        jnz     @@err
ELSE
IFDEF __DPMI__
        ; Deallocate buffer DOS memory:
        les     bx,[buf]
        push    es bx
        call    dpmiFreeDOSMem LANG, [word es:bx+2+dmaBuffer.memBlk]
        pop     bx es
        test    ax,ax
        jnz     @@err

        ; Deallocate buffer segment descriptor:
        call    dpmiFreeDescriptor LANG, [es:bx+dmaBuffer.bsegment]
        test    ax,ax
        jnz     @@err
ENDIF
ENDIF

        xor     ax,ax
        jmp     @@done

@@err:
        ERROR ID_dmaFreeBuffer

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:     int dmaPlayBuffer(dmaBuffer *buf, ushort channel,
;*                                 ushort autoInit);
;*
;* Description:  Plays a DMA buffer
;*
;* Input:        dmaBuffer *buf          buffer to be player
;*               ushort channel          DMA channel number
;*               ushort autoInit         use autoinitialization?
;*
;* Returns:      MIDAS error code
;*
;\***************************************************************************/

PROC	dmaPlayBuffer	FAR	buf : DWORD, channel : WORD, autoInit : WORD
USES	si,di

	les	di,[buf]		; es:di points to DMA buffer struct

        cmp     [vdsUsed],1             ; is VDS used?
        jne     @@novds

        ; VDS is used - disable DMA translation for current DMA channel:
        mov     ax,810Bh                ; VDS Disable DMA Translation function
        mov     bx,[channel]
        xor     dx,dx
        int     4Bh
        jnc     @@novds

        mov     ax,errVDSUsage
        jmp     @@err

@@novds:
        ; si is the offset to right channel structure from the
        ; beginning of dmaChns:
        imul    si,[channel],SIZE dmaChannel

        mov     al,[si+dmaChns.number]
	xor	ah,ah
	mov	[es:di+dmaBuffer.channel],ax
	and	al,3				; reset DMA request
        mov     dx,[si+dmaChns.request]
	out	dx,al

	or	al,4
        mov     dx,[si+dmaChns.singleMask]      ; mask out the channel
	out	dx,al

        mov     al,[si+dmaChns.number]
	and	al,3
	or	al,8 or 64		; read mode, single mode
	cmp	[autoInit],1		; use auto-initialization?
	jne	@@noai
	or	al,16			; enable auto-initialization

@@noai: mov     dx,[si+dmaChns.mode]
	out	dx,al

	mov	ebx,[es:di+dmaBuffer.address]	; physical start addr

        mov     dx,[si+dmaChns.page]
	mov	eax,ebx
	shr	eax,16			; al = DMA page
	out	dx,al

        mov     dx,[si+dmaChns.clearFF] ; clear byte pointer flip-flop
	xor	al,al			; so that next write to a 16-bit reg
	out	dx,al			; will go to the low byte

        mov     dx,[si+dmaChns.baseAddr]
        cmp     [si+dmaChns.number],3   ; 16-bit channel?
	jbe	@@no16
	shr	ebx,1			; starting word instead of byte

@@no16:
	mov	al,bl
	out	dx,al			; set base address low byte
	mov	al,bh
	out	dx,al			; set base address high byte

@@based:
	mov	bx,[es:di+dmaBuffer.blength]	; buffer length
        cmp     [si+dmaChns.number],3           ; 16-bit channel?
	jbe	@@no16_2
	shr	bx,1			; convert to _word_ (16-bit) count

@@no16_2:
	dec	bx			; word count = length - 1
        mov     dx,[si+dmaChns.wordCount]
	mov	al,bl
	out	dx,al			; set word count low byte
	mov	al,bh
	out	dx,al			; set word count high byte

@@countd:
        mov     al,[si+dmaChns.number]
	and	al,3
        mov     dx,[si+dmaChns.singleMask]      ; enable channel
	out	dx,al

        xor     ax,ax                   ; success
        jmp     @@done

@@err:
        ERROR   ID_dmaPlayBuffer

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:     int dmaStop(ushort channel);
;*
;* Description:  Stops DMA playing
;*
;* Input:        ushort channel          DMA channel number
;*
;* Returns:      MIDAS error code
;*
;\***************************************************************************/

PROC	dmaStop 	FAR	channel : WORD
USES	si

        cmp     [vdsUsed],1             ; is VDS used?
        jne     @@novds

        ; VDS is used - enable DMA translation again for current DMA channel:
        mov     ax,810Ch                ; VDS Disable DMA Translation function
        mov     bx,[channel]
        xor     dx,dx
        int     4Bh
        jnc     @@novds

        mov     ax,errVDSUsage
        jmp     @@err

@@novds:
        ; si is the offset to right channel structure from the
        ; beginning of dmaChns:
        imul    si,[channel],SIZE dmaChannel

        mov     dx,[si+dmaChns.singleMask]
        mov     al,[si+dmaChns.number]
	and	al,3			; mask out (disable) DMA channel
	or	al,4
	out	dx,al

        mov     dx,[si+dmaChns.clearFF]
	xor	al,al			; clear byte pointer flip-flop
	out	dx,al

        xor     ax,ax
        jmp     @@done

@@err:
        ERROR   ID_dmaStop

@@done:
	ret
ENDP



;/***************************************************************************\
;*
;* Function:     int dmaGetPos(dmaBuffer *buf, ushort *pos);
;*
;* Description:  Gets the DMA playing position
;*
;* Input:        dmaBuffer *buf          buffer that is being played
;*               ushort *pos             pointer to return value
;*
;* Returns:      MIDAS error code.
;*               DMA playing position from the beginning of the buffer,
;*               in bytes, is stored in *pos.
;*
;\***************************************************************************/

PROC    dmaGetPos       FAR     buf : dword, pos : dword
USES	si,di


        les     di,[buf]                ; point es:di to buffer structure
	mov	bx,[es:di+dmaBuffer.channel]

        ; si is the offset to right channel structure from the
        ; beginning of dmaChns:
        imul    si,bx,SIZE dmaChannel

        mov     dx,[si+dmaChns.clearFF] ; clear Flip-Flop port
	xor	al,al			; clear byte-pointer flip-flop
	out	dx,al

        mov     dx,[si+dmaChns.wordCount]       ; word count port
	cli				; disable interrupts for more
@@read: 				; accurate results.
	in	al,dx
	mov	ah,al
	in	al,dx			; bx = first word count
	xchg	al,ah
	mov	bx,ax

	in	al,dx
	mov	ah,al			; ax = second word count
	in	al,dx
	xchg	al,ah

@@countd:
	sub	bx,ax			; now compare the two results
	cmp	bx,4			; if the difference is over 4, read
	jg	@@read			; again - there might be an EMM
					; disturbing...
	cmp	bx,-4
	jl	@@read

	cmp	ax,[es:di+dmaBuffer.blength]	; check for bogus values
	jae	@@read

	sti

        cmp     [si+dmaChns.number],3   ; is the DMA channel 16-bit?
	jbe	@@no16
	shl	ax,1			; convert to bytes if a 16-bit chan
@@no16:
	neg	ax			; position = buffer size - count
	add	ax,[es:di+dmaBuffer.blength]

        les     di,[pos]                ; store position in *pos
        mov     [es:di],ax

        xor     ax,ax                   ; success

	ret
ENDP





END
