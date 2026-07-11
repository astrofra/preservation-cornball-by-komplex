;*      VGATEXT.ASM
;*
;* VGA text output routines for MIDAS Sound System
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
INCLUDE "vgatext.inc"


DATASEG


IFDEF __PASCAL__
IFDEF __PROTMODE__
EXTRN   SegB800 : word
ENDIF
ENDIF



IDATASEG


hextable	DB	"0123456789ABCDEF"


CODESEG



;/***************************************************************************\
;*
;* Function:	 void vgaWriteText(int x, int y, char *txt);
;*
;* Description:  Writes text on the screen
;*
;* Input:	 int x			 X coordinate of string (up-left
;*					 corner is (1,1))
;*		 int y			 Y coordinate
;*		 char *txt		 pointer to null-terminated text
;*					 string, which may contain also the
;*					 following special characters:
;*					     \xFF - next char is attribute
;*					     \x7F - next char is RLE count for
;*						 the character following it
;*
;\***************************************************************************/

PROC	vgaWriteText	FAR	x : word, y : word, txt : dword
USES	ds,si,di

	mov	ax,160
	mov	bx,[y]
	dec	bx
	mul	bx
	mov	di,[x]			; point di to destination in display
	dec	di			; memory (160*y + 2*x)
	shl	di,1
	add	di,ax

IFDEF __REALMODE__
	mov	ax,0B800h		; point es to display memory - es:di
	mov	es,ax			; points to destination
ELSE
        mov     es,[SegB800]
ENDIF

	lds	si,[txt]		; point ds:si to string

	mov	ah,07h			; default attribute is 07h - white
					; on black

@@lp:	mov	al,[ds:si]		; get byte from string
	inc	si
	or	al,al			; zero? (string termination)
	jz	@@done

	cmp	al,0FFh 		; is next byte attribute?
	je	@@attr

	cmp	al,07Fh 		; is next byte RLE count?
	je	@@rle

	mov	[es:di],ax		; normal character - write to screen
	add	di,2
	jmp	@@lp			; and get next character

@@attr:
	mov	ah,[ds:si]		; get next attribute
	inc	si
	jmp	@@lp			; get next character

@@rle:
	movzx	cx,[ds:si]		; get RLE count
	mov	al,[ds:si+1]		; get RLE byte
	add	si,2
	rep	stosw			; draw characters
	jmp	@@lp			; get next character

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	 void vgaWriteStr(int x, int y, char *str, char attr);
;*
;* Description:  Writes a string on the screen
;*
;* Input:	 int x			 X coordinate of the string
;*		 int y			 Y coordinate
;*		 char *str		 pointer to a ASCIIZ string
;*		 char attr		 attribute for the string
;*		 int txtlen		 number of characters to be printed on
;*					 screen - padded with spaces
;*
;\***************************************************************************/

PROC	vgaWriteStr	FAR	x : word, y : word, str : dword, attr : byte,\
				maxlen : word
USES	ds,si,di

	mov	ax,160
	mov	bx,[y]
	dec	bx
	mul	bx
	mov	di,[x]			; point di to destination in display
	dec	di			; memory (160*y + 2*x)
	shl	di,1
	add	di,ax

IFDEF __REALMODE__
	mov	ax,0B800h		; point es to display memory - es:di
	mov	es,ax			; points to destination
ELSE
        mov     es,[SegB800]
ENDIF

        lds     si,[str]                ; point ds:si to string

	mov	ah,[attr]		; attribute
	mov	cx,[maxlen]		; maximum number of characters

@@lp:	lodsb				; get character
	or	al,al			; zero? (end of string)
	jz	@@send			; if is, stop
	stosw				; write character and attribute
	loop	@@lp			; and get next character
	jmp	@@done

@@send: mov	al,' '                  ; string end - pad with spaces
	rep	stosw

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	 void vgaWriteByte(int x, int y, uchar byte, char attr);
;*
;* Description:  Writes a hex byte on the screen
;*
;* Input:	 int x			 X coordinate
;*		 int y			 Y coordinate
;*		 uchar byte		 byte to be written
;*		 char attr		 attribute for the byte
;*
;\***************************************************************************/

PROC	vgaWriteByte	FAR	x : word, y : word, b : byte, attr : byte
USES	di

	mov	ax,160
	mov	bx,[y]
	dec	bx
	mul	bx
	mov	di,[x]			; point di to destination in display
	dec	di			; memory (160*y + 2*x)
	shl	di,1
	add	di,ax

IFDEF __REALMODE__
	mov	ax,0B800h		; point es to display memory - es:di
	mov	es,ax			; points to destination
ELSE
        mov     es,[SegB800]
ENDIF

	mov	ah,[attr]		; attribute
	mov	bl,[b]
	shr	bx,4			; upper nybble
	and	bx,0Fh
	mov	al,[hextable+bx]	; upper nybble character
	mov	[es:di],ax		; write upper nybble

	mov	bl,[b]
	and	bx,0Fh			; lower nybble
	mov	al,[hextable+bx]	; lower nybble character
	mov	[es:di+2],ax		; write lower nybble

	ret
ENDP



;/***************************************************************************\
;*
;* Function:     void vgaFillRect(int x1, int y1, int x2, int y2, char attr);
;*
;* Description:  Draws a filled rectangle on the screen
;*
;* Input:        int x1                  X-coordinate of upper left corner
;*               int y1                  Y-coordinate of upper left corner
;*               int x2                  X-coordinate of lower left corner
;*               int y2                  Y-coordinate of lower left corner
;*               char attr               rectangle attribute
;*
;\***************************************************************************/

PROC    vgaFillRect     FAR     x1 : word, y1 : word, x2 : word, y2 : word, \
                                attr : byte
USES    si,di

        cld

        mov     ax,160
        mov     bx,[y1]
	dec	bx
	mul	bx
        mov     di,[x1]                 ; point di to destination in display
        dec     di                      ; memory (160*y1 + 2*x1)
	shl	di,1
	add	di,ax

IFDEF __REALMODE__
	mov	ax,0B800h		; point es to display memory - es:di
	mov	es,ax			; points to destination
ELSE
        mov     es,[SegB800]
ENDIF

        mov     bx,[y2]
        sub     bx,[y1]                 ; bx = row counter (y2-y1+1)
        inc     bx

        mov     dx,[x2]
        sub     dx,[x1]                 ; dx = number of columes (x2-x1+1)
        inc     dx

        mov     ax,dx
        shl     ax,1                    ; si = number of bytes to skip
        mov     si,160                  ; at the end of each row
        sub     si,ax                   ; (160 - 2*width)

        mov     ah,[attr]               ; ah = attribute
        mov     al,' '

@@rowlp:
        mov     cx,dx
        rep     stosw                   ; draw one row's space+attr pairs
        add     di,si                   ; point di to beginning of next row
        dec     bx
        jnz     @@rowlp                 ; do next row

        ret
ENDP




;/***************************************************************************\
;*
;* Function:     void vgaDrawChar(int x, int y, char ch, char attr);
;*
;* Description:  Draws a single character on the screen
;*
;* Input:        int x                   character X-coordinate
;*               int y                   character Y-coordinate
;*               char ch                 character
;*               char attr               character attribute
;*
;\***************************************************************************/

PROC    vgaDrawChar     FAR     x : word, y : word, cha : byte, attr : byte

        mov     ax,160
	mov	bx,[y]
	dec	bx
	mul	bx
        mov     bx,[x]                  ; point bx to destination in display
        dec     bx                      ; memory (160*y + 2*x)
        shl     bx,1
        add     bx,ax

IFDEF __REALMODE__
        mov     ax,0B800h               ; point es to display memory - es:bx
	mov	es,ax			; points to destination
ELSE
        mov     es,[SegB800]
ENDIF

        mov     ah,[attr]               ; ah = attribute
        mov     al,[cha]                ; al = character
        mov     [es:bx],ax              ; draw character & attribute

        ret
ENDP




;/***************************************************************************\
;*
;* Function:     void vgaSetMode(int mode)
;*
;* Description:  Sets a VGA BIOS display mode
;*
;* Input:        int mode                BIOS mode number
;*
;\***************************************************************************/

PROC    vgaSetMode      FAR     mode : word

        mov     al,[byte mode]
        xor     ah,ah                   ; int 10h, function 0 - set display
        int     10h                     ; mode

        ret
ENDP




;/***************************************************************************\
;*
;* Function:     void vgaMoveCursor(int x, int y);
;*
;* Description:  Moves the text mode cursor to a new location
;*
;* Input:        int x                   cursor X-coordinate
;*               int y                   cursor Y-coordinate
;*
;\***************************************************************************/

PROC    vgaMoveCursor   FAR     x : word, y : word

        mov     ax,0200h                ; int 10h, function 2 - set cursor
                                        ; location
        xor     bx,bx                   ; bh = display page (0)
        mov     dl,[byte x]             ; dl = column (0-based)
        dec     dl
        mov     dh,[byte y]             ; dh = row (0-based)
        dec     dh
        int     10h

        ret
ENDP




;/***************************************************************************\
;*
;* Function:     void vgaDrawChars(int x, int y, char ch, char attr, int num);
;*
;* Description:  Draws many charactersr on the screen
;*
;* Input:        int x                   character X-coordinate
;*               int y                   character Y-coordinate
;*               char ch                 character
;*               char attr               character attribute
;*               int num                 number characters to draw
;*
;\***************************************************************************/

PROC    vgaDrawChars    FAR     x : word, y : word, cha : byte, attr : byte, \
                                num : word
USES    di

        mov     ax,160
        mov     di,[y]
        dec     di
        mul     di
        mov     di,[x]                  ; point di to destination in display
        dec     di                      ; memory (160*y + 2*x)
        shl     di,1
        add     di,ax

IFDEF __REALMODE__
	mov	ax,0B800h		; point es to display memory - es:di
	mov	es,ax			; points to destination
ELSE
        mov     es,[SegB800]
ENDIF

        mov     ah,[attr]               ; ah = attribute
        mov     al,[cha]                ; al = character
        mov     cx,[num]                ; number of characters to draw
        cld
        rep     stosw                   ; draw characted-attribute pairs

        ret
ENDP





END
