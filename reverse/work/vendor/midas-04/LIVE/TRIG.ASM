;*      TRIG.ASM
;*
;* S2 The Party '94 64kb intro
;* -- Trigonometrical functions
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


DATASEG


sinTable        DD      ?
cosTable        DD      ?


; Precalculated sine values for angles 0-90 degrees:
LABEL   sinus   word
	DW	    0,   285,   571,   857,  1142,  1427,  1712,  1996,  2280
	DW	 2563,  2845,  3126,  3406,  3685,  3963,  4240,  4516,  4790
	DW	 5062,  5334,  5603,  5871,  6137,  6401,  6663,  6924,  7182
	DW	 7438,  7691,  7943,  8191,  8438,  8682,  8923,  9161,  9397
	DW	 9630,  9860, 10086, 10310, 10531, 10748, 10963, 11173, 11381
	DW	11585, 11785, 11982, 12175, 12365, 12550, 12732, 12910, 13084
	DW	13254, 13420, 13582, 13740, 13894, 14043, 14188, 14329, 14466
	DW	14598, 14725, 14848, 14967, 15081, 15190, 15295, 15395, 15491
	DW	15582, 15668, 15749, 15825, 15897, 15964, 16025, 16082, 16135
	DW	16182, 16224, 16261, 16294, 16321, 16344, 16361, 16374, 16381
	DW	16384



CODESEG




;/***************************************************************************\
;*
;* Function:     int MakeTrigTables(void);
;*
;* Description:  Initializes and allocates the trigonometrical tables
;*               (sinTable and cosTable)
;*
;* Returns:      MIDAS error code
;*
;\***************************************************************************/

PROC    MakeTrigTables          FAR
USES    si,di

        ; Allocate memory for sine/cosine tables:
        call    memAlloc C, 451*2, seg sinTable offset sinTable
        test    ax,ax
        jnz     @@err

        les     di,[sinTable]           ; point es:di to sine table
        mov     ax,di
        add     ax,2*90
        mov     [word cosTable],ax      ; point cosTable to cosine table
        mov     [word cosTable+2],es

        mov     si,offset sinus         ; point ds:si original sine table
        mov     cx,91
        cld
        rep     movsw                   ; copy values for 0-90 degrees

        sub     si,4                    ; point ds:si to value for 89 deg
        mov     cx,89

        ; build table for 91-179 degrees:
@@l1:	mov	ax,[si]
	stosw
	sub	si,2
	loop	@@l1

        push    ds
        lds     si,[sinTable]           ; point ds:si to 0 deg
	mov	cx,270

        ; build table for 180-450 degrees: (90-360 for cosine)
@@l2:	lodsw
	neg	ax
	stosw
	loop	@@l2

        pop     ds

        xor     ax,ax

@@err:
@@done:
        ret
ENDP



;/***************************************************************************\
;*
;* Function:     int iSin(int angle);
;*
;* Description:  Gets a sine value for an angle from the sine table.
;*
;* Input:        int angle               angle, in degrees
;*
;* Returns:      Sine value for the angle, multiplied by 16384.
;*
;\***************************************************************************/

PROC    iSin    FAR     angle : word

        xor     dx,dx

        mov     ax,[angle]
        test    ax,ax                   ; test if angle is negative
        jns     @@angok                 ; if is, make it positive and add 180
        neg     ax                      ; as sin (-x) = -sin (x) = sin (180+x)
        add     ax,180

@@angok:
        mov     bx,360                  ; dx = angle mod 360
        idiv    bx
        shl     dx,1                    ; dx = index to sine table

        les     bx,[sinTable]           ; point es:bx to sine table
        add     bx,dx                   ; point es:bx to sine value
        mov     ax,[es:bx]              ; ax = sine value
        ret
ENDP




;/***************************************************************************\
;*
;* Function:     int iCos(int angle);
;*
;* Description:  Gets a cosine value for and angle from the cosine table
;*
;* Input:        int angle               angle, in degrees
;*
;* Returns:      Cosine value for the angle, multiplied by 16384.
;*
;\***************************************************************************/

PROC    iCos    FAR     angle : word

        xor     dx,dx

        mov     ax,[angle]
        test    ax,ax                   ; test if angle is negative
        jns     @@angok                 ; if is, make it positive, as
        neg     ax                      ; cos(-x) = cos(x)

@@angok:
        mov     bx,360                  ; dx = angle mod 360
        idiv    bx
        shl     dx,1                    ; dx = index to cosine table

        les     bx,[cosTable]           ; point es:bx to cosine table
        add     bx,dx                   ; point es:bx to cosine value
        mov     ax,[es:bx]              ; ax = cosine value
        ret
ENDP




;/***************************************************************************\
;*
;* Function:     int iSinMult(int angle, int mult);
;*
;* Description:  Calculates a sine value multiplied by a number.
;*
;* Input:        int angle               angle, in degrees
;*               int mult                multiplier
;*
;* Returns:      mult * sin(angle)
;*
;\***************************************************************************/

PROC    iSinMult        FAR     angle : word, mult : word

        xor     dx,dx

        mov     ax,[angle]
        test    ax,ax                   ; test if angle is negative
        jns     @@angok                 ; if is, make it positive and add 180
        neg     ax                      ; as sin (-x) = -sin (x) = sin (180+x)
        add     ax,180

@@angok:
        mov     bx,360                  ; dx = angle mod 360
        div     bx
        shl     dx,1                    ; dx = index to sine table

        les     bx,[sinTable]           ; point es:bx to sine table
        add     bx,dx                   ; point es:bx to sine value
        mov     ax,[es:bx]              ; ax = sine value
        imul    [mult]                  ; multiply by the multiplier

        shr     ax,14
        shl     dx,2                    ; convert ax to normal integer
        or      ax,dx

        ret
ENDP




;/***************************************************************************\
;*
;* Function:     int iCosMult(int angle, int mult);
;*
;* Description:  Calculates a cosine value multiplied by a number.
;*
;* Input:        int angle               angle, in degrees
;*               int mult                multiplier
;*
;* Returns:      mult * cos(angle)
;*
;\***************************************************************************/

PROC    iCosMult        FAR     angle : word, mult : word

        xor     dx,dx

        mov     ax,[angle]
        test    ax,ax                   ; test if angle is negative
        jns     @@angok                 ; if is, make it positive, as
        neg     ax                      ; cos(-x) = cos(x)

@@angok:
        mov     bx,360                  ; dx = angle mod 360
        idiv    bx
        shl     dx,1                    ; dx = index to cosine table

        les     bx,[cosTable]           ; point es:bx to cosine table
        add     bx,dx                   ; point es:bx to cosine value
        mov     ax,[es:bx]              ; ax = cosine value
        imul    [mult]                  ; multiply by the multiplier

        shr     ax,14
        shl     dx,2                    ; convert ax to normal integer
        or      ax,dx

        ret
ENDP



END
