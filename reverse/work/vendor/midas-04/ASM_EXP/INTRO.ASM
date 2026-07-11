;*      INTRO.ASM
;*
;* Example on how to use screen synchronized timer and .MM modules linked
;* to executable, in the form of a small intro.
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

INCLUDE "midas.inc"


; Music in .MM format: (note that this definition may not be inside the
; data segment, as TASM+TLINK generate a fixup overflow in that case)
GLOBAL  music : FAR


DATASEG


module          DD      ?               ; pointer to module structure
scrStart        DW      ?               ; screen start address
frameCount      DD      0               ; frame counter
oldFrameCount   DD      0               ; previous frame counter
skipFrames      DD      ?               ; number of frames to skip
scrSync         DW      ?               ; timer screen synchronization value
fadeOut         DW      0               ; 1 if fading out
fadeVal         DW      64              ; fade out value

textPos         DW      offset text     ; current scroll text pointer

pixPanning      DB      ?               ; Pixel panning value
pixel           DB      80h             ; current scroller pixel mask

temp            DB      ?               ; plasma temporary variable

text            DB      " MIDAS Sound System v"
                DB      MVERSTR
                DB      " - a Sahara Surfers production of 1995     "
                DB      "Copyright 1995 Petteri Kangaslampi and "
                DB      "Jarno Paananen."
		DB	10 dup(" ")
		DB	0


        ; RGB palette:
LABEL           palette         byte
        ; Plasma palette: (0-128)
kala = 0
REPT	64
		DB	kala/2,kala/4,kala
kala = kala + 1
ENDM
kala = 0
REPT	32
		DB	31+kala,15,63-kala
kala = kala + 1
ENDM
kala = 31
REPT	32
		DB	63,kala/2,kala
kala = kala - 1
ENDM
                DB      63,0,0          ; Palette entry #128

                ; Scroller palette: (129-144)
IF 0
                DB      0,0,0
ENDIF
SBACKCOL1 = 129
                DB      7,0,15
		DB	11,0,23
		DB	15,0,31
		DB	19,0,39
		DB	23,0,47
		DB	27,0,56
		DB	29,0,59
		DB	31,0,63

IF 0
		DB	32,32,32
		DB	34,34,34
ENDIF
SCRCOL1 = 137
                DB      39,39,39
		DB	43,43,43
		DB	47,47,47
		DB	51,51,51
		DB	55,55,55
		DB	57,57,57
		DB	60,60,60
		DB	63,63,63

NUMCOLORS = 145


font8x8         DD      ?               ; pointer to system 8x8 font

startX1         DB      0               ; plasma position
startY1         DB      17
startX2         DB      99
startY2         DB      42

        ; Sine table: (ugly but easy and fast to use)
		DB	32,32,33,34,35,35,36,37
		DB	38,39,39,40,41,42,42,43
		DB	44,44,45,46,47,47,48,49
		DB	49,50,51,51,52,52,53,54
		DB	54,55,55,56,56,57,57,58
		DB	58,59,59,59,60,60,60,61
		DB	61,61,62,62,62,62,63,63
		DB	63,63,63,63,63,63,63,63
		DB	64,63,63,63,63,63,63,63
		DB	63,63,63,62,62,62,62,61
		DB	61,61,60,60,60,59,59,59
		DB	58,58,57,57,56,56,55,55
		DB	54,54,53,52,52,51,51,50
		DB	49,49,48,47,47,46,45,44
		DB	44,43,42,42,41,40,39,39
		DB	38,37,36,35,35,34,33,32
		DB	32,31,30,29,28,28,27,26
		DB	25,24,24,23,22,21,21,20
		DB	19,19,18,17,16,16,15,14
		DB	14,13,12,12,11,11,10,9
		DB	9,8,8,7,7,6,6,5
		DB	5,4,4,4,3,3,3,2
		DB	2,2,1,1,1,1,0,0
		DB	0,0,0,0,0,0,0,0
		DB	0,0,0,0,0,0,0,0
		DB	0,0,0,1,1,1,1,2
		DB	2,2,3,3,3,4,4,4
		DB	5,5,6,6,7,7,8,8
		DB	9,9,10,11,11,12,12,13
		DB	14,14,15,16,16,17,18,19
		DB	19,20,21,21,22,23,24,24
		DB	25,26,27,28,28,29,30,31
LABEL	sine	BYTE

REPT	4
		DB	32,32,33,34,35,35,36,37
		DB	38,39,39,40,41,42,42,43
		DB	44,44,45,46,47,47,48,49
		DB	49,50,51,51,52,52,53,54
		DB	54,55,55,56,56,57,57,58
		DB	58,59,59,59,60,60,60,61
		DB	61,61,62,62,62,62,63,63
		DB	63,63,63,63,63,63,63,63
		DB	64,63,63,63,63,63,63,63
		DB	63,63,63,62,62,62,62,61
		DB	61,61,60,60,60,59,59,59
		DB	58,58,57,57,56,56,55,55
		DB	54,54,53,52,52,51,51,50
		DB	49,49,48,47,47,46,45,44
		DB	44,43,42,42,41,40,39,39
		DB	38,37,36,35,35,34,33,32
		DB	32,31,30,29,28,28,27,26
		DB	25,24,24,23,22,21,21,20
		DB	19,19,18,17,16,16,15,14
		DB	14,13,12,12,11,11,10,9
		DB	9,8,8,7,7,6,6,5
		DB	5,4,4,4,3,3,3,2
		DB	2,2,1,1,1,1,0,0
		DB	0,0,0,0,0,0,0,0
		DB	0,0,0,0,0,0,0,0
		DB	0,0,0,1,1,1,1,2
		DB	2,2,3,3,3,4,4,4
		DB	5,5,6,6,7,7,8,8
		DB	9,9,10,11,11,12,12,13
		DB	14,14,15,16,16,17,18,19
		DB	19,20,21,21,22,23,24,24
		DB	25,26,27,28,28,29,30,31
ENDM


CODESEG


PUBLIC NOLANGUAGE main


;/***************************************************************************\
;*
;* Function:	main
;*
;* Description: Main program, called by MIDAS startup code
;*
;\***************************************************************************/

PROC NOLANGUAGE main	FAR

	; Set MIDAS Sound System defaults:
	call	midasSetDefaults LANG

        ; Run configuration:
        call    midasConfig LANG
        test    ax,ax                   ; Esc pressed?
        jz      @@exit                  ; if yes, exit

	; Set tweaked screen mode:
        call    SetScreenMode

	; Get timer sync value for screen mode:
	; This has to be done before midasInit, because it uses
	; the timer chip in hardware which might screw up the MIDAS
	; timer
        call    tmrGetScrSync LANG, seg scrSync offset scrSync

        ; EMS MUST be disabled when using MM modules:
        mov     [midasDisableEMS],1

	; Initialize MIDAS Sound System:
        call    midasInit LANG

	; Prepare MM module file using Protracker module player:
        call    midasPrepareMM LANG, seg music offset music,\
                seg mpMOD offset mpMOD
	mov	[word module],ax
	mov	[word module+2],dx

	; Start playing module:
        call    midasPlayModule LANG, [module], 0

        ; Get system 8x8 font pointer to font8x8:
	push	bp
	mov	ax,01130h
	mov	bh,3
	int	10h
	mov	[word font8x8],bp
	mov	[word font8x8+2],es
	pop	bp

        ; Reset screen start address and pixel panning:
        mov     [scrStart],168*82
	mov	[pixPanning],0

	; Set up screen synchronized timer:
        call    tmrSyncScr LANG, [scrSync], seg preVR offset preVR, \
                seg immVR offset immVR, seg inVR offset inVR

@@runintro:
        ; Wait for next frame:
        mov     eax,[frameCount]
@@frame:
        cmp     [frameCount],eax
        je      @@frame

        mov     ecx,[frameCount]
        mov     eax,[oldFrameCount]
        mov     [oldFrameCount],ecx
        sub     ecx,eax                 ; ecx = number of frames passed
        mov     [skipFrames],ecx

        cmp     [fadeOut],1             ; fading out?
        jne     @@run

        mov     ax,[fadeVal]
        sub     ax,cx                           ; decrease fading value
        jns     @@fadeok
        xor     ax,ax                           ; make sure we won't wrap

@@fadeok:
        mov     [fadeVal],ax
        mov     bl,al
        call    FadePalette                     ; set faded palette

        ; Set new master volume to Sound Device:
        les     bx,[midasSD]
        call    [es:bx+SoundDevice.SetMasterVolume] LANG, [fadeVal]

        ; In theory we should check for returned error code here, but
        ; SoundDevicce.SetMasterVolume() can not in practise fail.

        ; Advance plasma for correct number of frames:
@@run:  call    AdvancePlasma
        dec     [skipFrames]
        jnz     @@run

        ; Draw the plasma:
        call    DrawPlasma

        cmp     [fadeOut],1             ; fading out?
        jne     @@nofade
        cmp     [fadeVal],0             ; yes - exit if faded to black
        je      @@stop

@@nofade:
	; Check for key press:
	mov	ah,1
	int	16h
        jz      @@runintro

        ; Key pressed - start fading out:
        mov     [fadeOut],1

	; Flush the key press from the buffer
	xor	ah,ah
	int	16h

        jmp     @@runintro

@@stop:
	; Stop screen synchronization:
        call    tmrStopScrSync LANG

	; Stop playing module:
        call    midasStopModule LANG, [module]

	; Deallocate module:
        call    midasFreeMM LANG, [module]

	; Uninitialize MIDAS Sound System:
        call    midasClose LANG

@@exit:
	; Restore text mode:
	mov	ax,3
	int	10h

	; Exit to DOS with return code 0:
	xor	al,al
	ret
ENDP




;/***************************************************************************\
;*
;* Function:    AdvancePlasma
;*
;* Description: Advances the plasma parameters for one frame.
;*
;\***************************************************************************/

PROC NOLANGUAGE AdvancePlasma NEAR

        ; Advance plasma position
        sub     [startX1],2
        add     [startY1],3
        sub     [startX2],3
        add     [startY2],4
	ret
ENDP




;/***************************************************************************\
;*
;* Function:    DrawPlasma
;*
;* Description: Draws one frame of plasma. Slow, but who cares - this is
;*              supposed to be an example on how to use MIDAS, not how to
;*              code demo effects. To double the speed of this plasma,
;*              change it to work one row at a time. It is now drawn in
;*              columns as it originally had 1x1 pixel resolution.
;*
;\***************************************************************************/

PROC NOLANGUAGE DrawPlasma NEAR

	mov	ax,0a000h
	mov	es,ax
	xor	di,di

        mov     dx,03C4h
        mov     ax,0F02h                ; draw to all four planes - 4x1 pixels                  ; map mask register
        out     dx,ax

        mov     [temp],80

        ; Loop for all columns:
@@xloop:
        mov     al,[temp]
	xor	dh,dh
        mov     dl,[startX1]
        add     dl,al
	mov	bx,dx
	add	dl,[sine+bx]
        add     dl,[startY1]
	add	dx,offset sine

	xor	ch,ch
        mov     cl,[startX2]
        sub     cl,al
	mov	bx,cx
	add	cl,[sine+bx]
        sub     cl,[startY2]
	add	cx,offset sine

	mov	si,cx
	mov	bx,dx

	; Draw one vertical pixel row
a = 0
REPT    168
        mov     al,[bx+a]
        add     al,[si+a*2]
        mov     [es:di+a*82],al
a = a + 1
ENDM
        inc     di
        dec     [temp]
        jnz     @@xloop

        ret
ENDP




;/***************************************************************************\
;*
;* Function:	preVR
;*
;* Description: Routine called by the timer just before Vertical Retrace.
;*              Sets screen start address for scroller.
;*
;\***************************************************************************/

PROC NOLANGUAGE preVR FAR

        ; Set screen start address:
        mov     dx,03D4h
        mov     al,0Dh
        mov     ah,[byte scrStart]
	out	dx,ax
	dec	al
        mov     ah,[byte scrStart+1]
	out	dx,ax

	ret
ENDP




;/***************************************************************************\
;*
;* Function:	immVR
;*
;* Description: Routine called by the timer immediately after Vertical
;*              Retrace has started. Increments the frame counter and sets
;*              pixel panning value for the scroller
;*
;\***************************************************************************/

PROC NOLANGUAGE immVR FAR

        inc     [frameCount]

	; Set pixel panning value
	mov	dx,03dah
	in	al,dx
	mov	dl,0c0h
	mov	al,33h
	out	dx,al
	mov	al,[pixPanning]
	out	dx,al
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	inVR
;*
;* Description: Routine called by the timer during the Vertical Retrace.
;*              Updates scroller position and draws new characters.
;*
;\***************************************************************************/

PROC NOLANGUAGE inVR FAR

	add	[pixPanning],2
	cmp	[pixPanning],8
	jne	@@ok
	mov	[pixPanning],0
@@ok:	cmp	[pixPanning],0
	jne	@@dah

        call    WriteTextRow

@@dah:	ret
ENDP




;/***************************************************************************\
;*
;* Function:    WriteTextRow
;*
;* Description: Writes one "pixel" (=4 real pixels...) row of text to the
;*		right border
;*
;\***************************************************************************/

PROC NOLANGUAGE WriteTextRow NEAR

	mov	dx,03c4h
	mov	ax,0f02h		; Plot all four pixels
	out	dx,ax

	mov	ax,0a000h
	mov	es,ax
        mov     di,[scrStart]
	add	di,81

        mov     si,[textPos]
	xor	ax,ax
        mov     al,[si]                 ; get character
        test    al,al                   ; at the end of the string?
        jnz     @@noend

        mov     si,offset text          ; yes, reset scroll position
        mov     [textPos],si
        jmp     @@done

@@noend:
	shl	ax,3
        lgs     si,[font8x8]            ; point gs:si to font data for this
        add     si,ax                   ; character
        mov     dl,[pixel]              ; dl = pixel mask
	mov	cl,8
        mov     ch,SCRCOL1
        mov     bl,SBACKCOL1

        ; draw all rows:

@@loop: mov     al,[gs:si]              ; get font data for current row
	inc	si
        test    al,dl                   ; is there a pixel?
        jnz     @@pixel
        mov     [es:di],bl              ; no, draw background stripes
	mov	[es:di+82],bl
	mov	[es:di+164],bl
	mov	[es:di+246],bl
        jmp     @@nopix

@@pixel:
        mov     [es:di],ch              ; there is a pixel - draw it
	mov	[es:di+82],ch
	mov	[es:di+164],ch
	mov	[es:di+246],ch

@@nopix:
        inc     ch
	inc	bl
	add	di,82*4
	dec	cl
	jne	@@loop

        mov     [es:di],bl              ; draw stripe at the bottom of the
        mov     [es:di+82],bl           ; character
	mov	[es:di+164],bl
	mov	[es:di+246],bl

        inc     [scrStart]              ; scroll to new pixel

        ror     [pixel],1               ; set pixel mask to next pixel
        jnc     @@done
        inc     [textPos]               ; next character

@@done: ret
ENDP




;/***************************************************************************\
;*
;* Function:    SetScreenMode
;*
;* Description: Sets up the tweaked screen mode (320x200)
;*
;\***************************************************************************/

PROC NOLANGUAGE SetScreenMode NEAR

	mov	ax,0013h		; set mode 13h (standard 320x200x256)
	int	10h

        cld

	mov	dx,03C4h		; Sequencer address register
	mov	ax,0604h		; no Chain Four, Sequential mem, >64k
	out	dx,ax

	mov	dx,03D4h		; CRT Controller address register
	mov	al,14h
	out	dx,al
	inc	dx
	in	al,dx
	and	al,0BFh
	out	dx,al

	dec	dx
	mov	al,17h
	out	dx,al
	inc	dx
	in	al,dx
	or	al,40h
	out	dx,al
	dec	dx
	mov	dx,03D4h			; use double-scan
	mov	al,13h
	out	dx,al
	inc	dx
	mov	ax,40
	out	dx,al

	mov	ax,0a000h			; Clear video memory
	mov	es,ax
	xor	di,di
	mov	cx,16384
	xor	eax,eax
	cld
	rep	stosd

        mov     bl,64
        call    FadePalette

IF 0
	mov	si,offset ScrollRGB
	mov	al,238				; Set scroll palette
	mov	dx,03c8h
	out	dx,al
	inc	dl
	mov	cx,18*3
	rep	outsb

	mov	si,offset PlasmaRGB		; Set plasma palette
	xor	al,al
	mov	dl,0c8h
	out	dx,al
	inc	dl
	mov	cx,129*3
	rep	outsb
ENDIF

	mov	dx,03D4h			; Clear Line Compare bit 9
	mov	al,9
	out	dx,al
	inc	dl
	in	al,dx
	and	al,0bfh
	out	dx,al
	dec	dl

	mov	ax,04018h			; Set Line Compare bits 0-7
	out	dx,ax				; to 40h (=32 lo-res pixels)

	mov	al,7				; Clear Line Compare bit 8
	out	dx,al
	inc	dl
	in	al,dx
	and	al,0efh
	out	dx,al
	dec	dl

	mov	al,13h				; Make screen 82 bytes wide
	out	dx,al
	inc	dl
	mov	al,41				; Width in words
	out	dx,al

	mov	dl,0dah 			; Turn pixel panning
	in	al,dx				; compatibility on:
	mov	dl,0c0h
	mov	al,30h
	out	dx,al
	inc	dl
	in	al,dx
	or	al,20h
	mov	bl,al
	mov	dl,0dah
	in	al,dx
	mov	dl,0c0h
	mov	al,30h
	out	dx,al
	mov	al,bl
	out	dx,al

	; Draw the background stripes behind the scroll...
	mov	di,168*82
        mov     al,SBACKCOL1
	mov	cl,9
@@loo:
	mov	dx,82*4
@@loo2:
	mov	[es:di],al
	inc	di
	dec	dx
	jnz	@@loo2
	inc	al
	dec	cl
	jnz	@@loo
	ret
ENDP




;/***************************************************************************\
;*
;* Function:    FadePalette
;*
;* Description: Sets the RGB palette to VGA fading it
;*
;* Input:       bl              fade value (64-0), 64 = full intensity,
;*                              0 = black
;*
;* Destroys:    ax, cx, dx, si
;*
;\***************************************************************************/

PROC NOLANGUAGE FadePalette     NEAR

        mov     si,offset palette
        mov     cx,NUMCOLORS*3

        ; Start setting the palette from color 0:
        mov     dx,03C8h
        xor     al,al
        out     dx,al
        inc     dx

@@loop:
        mov     al,[si]                 ; get value from palette
        inc     si
        mul     bl                      ; multiply by fade value
        shr     ax,6                    ; and divide by 64
        out     dx,al                   ; set value to VGA
        dec     cx
        jnz     @@loop

        ret
ENDP



END
