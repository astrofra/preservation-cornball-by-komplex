;*	MUTILS.ASM
;*
;* Miscellaneous utility functions for MIDAS Sound System used
;* by various system components.
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

INCLUDE "lang.inc"
INCLUDE "mutils.inc"



CODESEG


;/***************************************************************************\
;*
;* Function:	 int mGetKey(void)
;*
;* Description:  Waits for a keypress and returns the read key
;*
;* Returns:	 ASCII code for the key pressed. Extended keycodes are
;*		 returned with bit 8 set, eg. up arrow becomes \x148.
;*
;\***************************************************************************/

PROC	mGetKey 	FAR

	mov	ax,0700h		; read key without echo
	int	21h

	xor	ah,ah
	test	al,al			; read key zero?
	jnz	@@keyok 		; if not, it is the key ASCII code

	; the read key was zero - now read the actual extended keycode and
	; return it with bit 8 set.

	mov	ax,0700h		; read key without echo
	int	21h
	mov	ah,1			; set bit 8 to 1

@@keyok:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	 int mStrLength(char *str)
;*
;* Description:  Calculates the length of a ASCIIZ string
;*
;* Input:	 char *str		 pointer to string
;*
;* Returns:	 String length excluding the terminating '\0'.
;*
;\***************************************************************************/

PROC	mStrLength	FAR	str : dword

	les	bx,[str]		; point es:bx to string
	xor	ax,ax			; current length = 0

@@lp:
	cmp	[byte es:bx],0		; is current byte 0
	je	@@done			; if is, we are finished
	inc	bx			; next byte
	inc	ax
	jmp	@@lp

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	 void mStrCopy(char *dest, char *src);
;*
;* Description:  Copies an ASCIIZ string from *src to *dest.
;*
;* Input:	 char *dest		 pointer to destination string
;*		 char *src		 pointer to source string
;*
;\***************************************************************************/

PROC	mStrCopy	FAR	dest : dword, src : dword
USES	ds,si,di

	; Clear enough as it is? If not, maybe you shouldn't be reading
	; this source after all...

	cld
	lds	si,[src]
	les	di,[dest]

@@lp:	lodsb
	stosb
	test	al,al
	jnz	@@lp

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	 void mStrAppend(char *dest, char *src);
;*
;* Description:  Appends an ASCIIZ string to the end of another.
;*
;* Input:	 char *dest		 pointer to destination string
;*		 char *src		 pointer to source string
;*
;\***************************************************************************/

PROC	mStrAppend	FAR	dest : dword, src : dword
USES	ds,si,di

	; Again, if you don't understand this you should perhaps consider
	; switching hobby...

	cld
	lds	si,[src]
	les	di,[dest]

@@l1:	mov	al,[es:di]
	test	al,al
	jz	@@lp
	inc	di
	jmp	@@l1

@@lp:	lodsb
	stosb
	test	al,al
        jz      @@done
	jmp	@@lp

@@done:
	ret
ENDP



;/***************************************************************************\
;*
;* Function:	 void mMemCopy(char *dest, char *src, unsigned numBytes);
;*
;* Description:  Copies a memory block from *src to *dest.
;*
;* Input:	 char *dest		 pointer to destination
;*		 char *src		 pointer to source
;*		 unsigned numBytes	 number of bytes to copy
;*
;\***************************************************************************/

PROC	mMemCopy	FAR	dest : dword, src : dword, numBytes : word
USES	ds,si,di

	cld
	lds	si,[src]
	les	di,[dest]
	mov	cx,[numBytes]
	rep	movsb

	ret
ENDP




;/***************************************************************************\
;*
;* Function:	 int mMemEqual(char *m1, char *m2, unsigned numBytes);
;*
;* Description:  Compares two memory blocks.
;*
;* Input:	 char *m1		 pointer to memory block #1
;*		 char *m2		 pointer to memory block #2
;*		 unsigned numBytes	 number of bytes to compare
;*
;* Returns:	 1 if the memory blocks are equal, 0 if not.
;*
;\***************************************************************************/

PROC	mMemEqual	FAR	m1 : dword, m2 : dword, numBytes : word
USES	ds,si,di

	cld
	lds	si,[m1]
	les	di,[m2]
	mov	cx,[numBytes]
	repe	cmpsb
	test	cx,cx
	jnz	@@une
	mov	ax,1
	jmp	@@done

@@une:	xor	ax,ax

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	 long mHex2Long(char *hex)
;*
;* Description:  Converts a hexadecimal string to a long integer.
;*
;* Input:	 char *hex		 pointer to hex string, ASCIIZ
;*
;* Returns:	 Value of the string or -1 if conversion failure.
;*
;\***************************************************************************/

PROC	mHex2Long	FAR	hex : dword

	les	bx,[hex]		; point es:bx to string
	xor	edx,edx 		; current number is zero

@@lp:	mov	al,[es:bx]		; get character from string
	inc	bx
	test	al,al			; terminating '\0'?
	jz	@@strend

	shl	edx,4			; move previous string data to left

	cmp	al,'0'                  ; character below '0'?
	jb	@@err			; if yes, error
	cmp	al,'9'                  ; character <= '9'?
	jbe	@@decdigit		; if yes, it's a decimal digit

	cmp	al,'A'                  ; character below 'A'?
	jb	@@err			; if yes, error
	cmp	al,'F'                  ; character <= 'F'?
	jbe	@@chexdigit		; if yes, it's a capital hex digit

	cmp	al,'a'                  ; character below 'a'?
	jb	@@err			; if yes, error
	cmp	al,'f'                  ; character <= 'f'?
	jbe	@@shexdigit		; if yes, it's a small hex digit

	jmp	@@err			; unrecognized character - error

@@decdigit:
	sub	al,'0'                  ; al = current digit value
	or	dl,al			; add it to edx
	jmp	@@lp			; and get next character

@@chexdigit:
	sub	al,'A'-0Ah              ; al = current digit value
	or	dl,al			; add it to edx
	jmp	@@lp			; and get next character

@@shexdigit:
	sub	al,'a'-0Ah              ; al = current digit value
	or	dl,al			; add it to edx
	jmp	@@lp			; and get next character

@@strend:
	mov	ax,dx			; move result from edx to dx:ax
	shr	edx,16
	jmp	@@done			; and leave

@@err:
	mov	ax,-1
	mov	dx,-1

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	 long mDec2Long(char *dec)
;*
;* Description:  Converts an unsigned decimal string to a long integer
;*
;* Input:	 char *dec		 pointer to string, ASCIIZ
;*
;* Returns:	 Value of the string or -1 if conversion failure.
;*
;\***************************************************************************/

PROC	mDec2Long	FAR	dec : dword

	les	bx,[dec]		; point es:bx to string
	xor	edx,edx 		; current value is zero
        xor     eax,eax

@@lp:
	mov	al,[es:bx]		; get character from string
	inc	bx
	test	al,al			; terminating '\0'?
	jz	@@strend

	imul	edx,edx,10		; multiply current value by 10 to
					; make room for the new digit
	cmp	al,'0'                  ; character below '0'?
	jb	@@err			; if yes, error
	cmp	al,'9'                  ; character above '9'?
	ja	@@err			; if yes, error

	sub	al,'0'                  ; al = current digit value
        add     edx,eax                 ; add it to edx
	jmp	@@lp			; and get next character

@@strend:
	mov	ax,dx			; move result from edx to dx:ax
	shr	edx,16
	jmp	@@done			; and leave

@@err:
	mov	ax,-1
	mov	dx,-1

@@done:
	ret
ENDP




;/**************************************************************************\
;*
;* Function:	 char *mGetEnv(char *envVar);
;*
;* Description:  Searches a string from the environment
;*
;* Input:	 envVar 		 environment variable name, ASCIIZ
;*
;* Returns:	 Pointer to environment string value (ASCIIZ), NULL if string
;*		 was not found.
;*
;\**************************************************************************/

PROC	mGetEnv 	FAR	envVar : dword
USES	ds,si,di

	cld
	les	di,[envVar]		; point es:di and es:bx to environment
	mov	bx,di			; variable name
	xor	al,al
	mov	cx,7FFFh
	repne	scasb			; scan for first 0 in string
	sub	di,bx
	dec	di
	mov	dx,di			; dx = string length (excl. '\0')

	test	dx,dx			; skip if zero length string
	jz	@@notfound

	mov	ah,62h			; int 21h, function 62h - get PSP seg
	int	21h			; bx = PSP segment

	lds	si,[envVar]		; point ds:si to variable name
	mov	es,bx			; es = PSP segment
	mov	es,[es:2ch]		; es = Environment segment
	xor	di,di			; point es:di to environment

	mov	ah,[ds:si]		; ah = first character of string
	xor	al,al			; al = 0 !!!

@@search:
	cmp	[es:di],ah		; Is the first letter of the current
	jne	@@skipend		; environment string same as the one
					; we are looking for? If not, skip
					; the whole string.

	push	si
	mov	cx,dx
	repe	cmpsb			; check if the whole string is equal
	pop	si
	jne	@@skipend		; skip if not
	cmp	[byte es:di],"="        ; check is the next character is '='
	jne	@@skipend		; skip if not
	inc	di			; point es:di to start of environment
	mov	dx,es			; return pointer in dx:ax
	mov	ax,di
	jmp	@@done

@@skipend:
	mov	cx,7FFFh
	repne	scasb			; search for first '\0'.
	cmp	[byte es:di],0		; is the next byte also 0?
	je	@@notfound		; if is, end of environment
	jmp	@@search

@@notfound:
	xor	ax,ax			; string was not found - return NULL
	xor	dx,dx

@@done:
	ret
ENDP




END
