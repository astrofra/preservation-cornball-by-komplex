;*      MMEM.ASM
;*
;* MIDAS Sound System memory handling routines for pure assembler. Uses
;* DOS memory allocation/deallocation functions. Program memory block
;* MUST have been shrinked by the startup code before using these
;* functions.
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


CODESEG


;/***************************************************************************\
;*
;* Function:    int memAlloc(unsigned short len, void **blk);
;*
;* Description: Allocates a block of conventional memory
;*
;* Input:       unsigned short len      Memory block length in bytes
;*              void **blk              Pointer to memory block pointer
;*
;* Returns:     MIDAS error code.
;*              Pointer to allocated block stored in *blk, NULL if error.
;*
;\***************************************************************************/

PROC    memAlloc        FAR     len : word, blk : dword

        mov     bx,[len]
        test    bx,bx                   ; check that block length is nonzero
        jnz     @@1

        mov     ax,errInvalidBlock      ; invalid memory block length
        jmp     @@err

@@1:
        add     bx,15                   ; convert bx to number of paragraphs
        shr     bx,4

        ; Allocate memory for block:
        mov     ax,4800h
        int     21h
        jnc     @@allocok

        ; Memory allocation failed - get DOS extended error code:
        mov     ax,5900h
        xor     bx,bx
        push    ds si di
        int     21h
        pop     di si ds

        cmp     ax,8                    ; error code 8 - out of memory?
        je      @@outofmem
        cmp     ax,7                    ; error code 7 - heap corrupted?
        je      @@heapcorrupt

        mov     ax,errUndefined         ; none of the above - undefined error
        jmp     @@err

@@outofmem:
        mov     ax,errOutOfMemory       ; out of memory
        jmp     @@err

@@heapcorrupt:
        mov     ax,errHeapCorrupted     ; heap corrupted
        jmp     @@err

@@allocok:
        ; Memory allocated succesfully - resulting block is ax:0. Write its
        ; address to *blk and exit:
        les     bx,[blk]
        mov     [word es:bx],0
        mov     [word es:bx+2],ax

        xor     ax,ax                   ; success
        jmp     @@done

@@err:
        ERROR   ID_memAlloc

@@done:
        ret
ENDP




;/***************************************************************************\
;*
;* Function:     int memFree(void *blk);
;*
;* Description:  Deallocates a memory block allocated with memAlloc()
;*
;* Input:        void *blk               Memory block pointer
;*
;* Returns:      MIDAS error code.
;*
;\***************************************************************************/

PROC    memFree         FAR     blk : dword

        cmp     [word blk],0            ; offset of memory block must always
        jne     @@badblock              ; be zero
        cmp     [word blk+2],0          ; segment of memory block can not be
        jne     @@blkok                 ; zero

@@badblock:
        mov     ax,errInvalidBlock      ; invalid memory block
        jmp     @@err

@@blkok:
        ; Deallocate memory block
        mov     ax,4900h
        mov     es,[word blk+2]
        int     21h
        jnc     @@ok

        ; Deallocation unsuccessful - get DOS extended error code:
        mov     ax,5900h
        xor     bx,bx
        push    ds si di
        int     21h
        pop     di si ds

        cmp     ax,7                    ; error code 7 - heap corrupted?
        je      @@heapcorrupted
        cmp     ax,9                    ; error code 9 - invalid block?
        je      @@badblock

        mov     ax,errUndefined         ; none of the above - undefined error
        jmp     @@err

@@heapcorrupted:
        mov     ax,errHeapCorrupted     ; heap corrupted
        jmp     @@err

@@ok:
        xor     ax,ax                   ; success
        jmp     @@done

@@err:
        ERROR   ID_memFree
@@done:
        ret
ENDP




END
