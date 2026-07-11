;*	SB.ASM
;*
;* Sound Blaster series Sound Device, v2.15
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
INCLUDE "sdevice.inc"
INCLUDE "dsm.inc"
INCLUDE "dma.inc"
INCLUDE "mutils.inc"



;/***************************************************************************\
;*	 enum sbFunctIDs
;*	 ----------------
;* Description:  ID numbers for SB Sound Device functions
;\***************************************************************************/

enum	sbFunctIDs \
	ID_sbDetect = ID_sb, \
	ID_sbInit, \
	ID_sbClose, \
	ID_sbStartPlay



;/***************************************************************************\
;*	ENUM sbCardTypes
;*	----------------
;* Description: Sound Card type number for SB Sound Device
;\***************************************************************************/

ENUM	sbCardTypes \
	sbAutoType = 0, \		; autodetect card type
	sb10, \ 			; Sound Blaster 1.0 (DSP v1.xx)
	sb15, \ 			; Sound Blaster 1.5 (DSP v2.00)
	sb20, \ 			; Sound Blaster 2.0 (DSP v2.01)
	sbPro, \			; Sound Blaster Pro (DSP v3.xx)
	sb16				; Sound Blaster 16 (DSP v4.00+)



DATASEG

oldIRQ		DD	?		; old IRQ vector
oldIRQmask	DB	?		; old IRQ mask
sb22C		DW	?		; SB DSP data port (2xCh)
sbTimeConstant	DB	?		; SB Transfer Time Constant
sbRate		DW	?		; SB actual playing rate
sbVersion	DW	?		; DSP version number
sbMode		DW	?		; actual output mode
sbInterrupt	DB	?		; IRQ interrupt number
sbBlockLength	DW	?		; DSP playing block length
sbOutputFilter	DB	?		; initial output filter status
sbStereoOK	DB	?		; flag used by sbSetStereo()
convBuffer	DB	4 DUP (?)	; string to number conversion buffer
sbDMABuffer     dmaBuffer       ?       ; DMA playing buffer
sbDMAPos        DW      ?               ; DMA playing position



IFDEF __PASCAL__
EXTRN   SB : SoundDevice                ; Sound Device for Pascal version
ENDIF




IDATASEG


SBCONFIGBITS = sdUsePort or sdUseIRQ or sdUseDMA or sdUseMixRate or \
               sdUseOutputMode or sdUseDSM
SBMODEBITS = sdMono or sdStereo or sd8bit or sd16bit

; If compiling for Pascal, Sound Device name is sbSD, from which the data
; will be copied to Sound Device SB, defined in Pascal.

IFDEF   __PASCAL__
SDNAM           equ     sbSD
ELSE
SDNAM           equ     SB
ENDIF

GLOBAL  SDNAM : SoundDevice

SDNAM   SoundDevice     < \
 0, \
 SBCONFIGBITS, \
 220h, 05h, 01h, \
 sbAutoType, 5, \
 sdUnInitialized, \
 SBMODEBITS, \
 far ptr sbSDName, \
 far ptr sbCardNames, \
 6, far ptr sbPortAddresses, \
 far ptr sbDetect, \
 far ptr sbInit, \
 far ptr sbClose, \
 far ptr dsmGetMixRate, \
 far ptr dsmGetMode, \
 far ptr dsmOpenChannels, \
 far ptr dsmCloseChannels, \
 far ptr dsmClearChannels, \
 far ptr dsmMute, \
 far ptr dsmPause, \
 far ptr dsmSetMasterVolume, \
 far ptr dsmGetMasterVolume, \
 far ptr dsmSetAmplification, \
 far ptr dsmGetAmplification, \
 far ptr dsmPlaySound, \
 far ptr dsmStopSound, \
 far ptr dsmSetRate, \
 far ptr dsmGetRate, \
 far ptr dsmSetVolume, \
 far ptr dsmGetVolume, \
 far ptr dsmSetInstrument, \
 far ptr dsmGetInstrument, \
 far ptr dsmSetPosition, \
 far ptr dsmGetPosition, \
 far ptr dsmSetPanning, \
 far ptr dsmGetPanning, \
 far ptr dsmMuteChannel, \
 far ptr dsmAddInstrument, \
 far ptr dsmRemInstrument, \
 far ptr dsmSetUpdRate, \
 far ptr sbStartPlay, \
 far ptr dsmPlay >

sbSDName        DB      "Sound Blaster series Sound Device v2.15",0

sbCardNames	DD	far ptr sb10Name
		DD	far ptr sb15Name
		DD	far ptr sb20Name
		DD	far ptr sbProName
		DD	far ptr sb16Name

sb10Name	DB	"Sound Blaster 1.0 or clone (DSP v1.xx)", 0
sb15Name	DB	"Sound Blaster 1.5 (DSP v2.00)", 0
sb20Name	DB	"Sound Blaster 2.0 (DSP v2.01)", 0
sbProName	DB	"Sound Blaster Pro (DSP v3.xx)", 0
sb16Name	DB	"Sound Blaster 16 (DSP v4.00+)", 0

sbPortAddresses DW	210h, 220h, 230h, 240h, 250h, 260h

blasterStr	DB	"BLASTER", 0



	; "fake" one-byte DMA buffer used by sbSetStereo()
sbStereoDMABuffer dmaBuffer	< 0, 0, 1, 0, -1 >



CODESEG



PUBLIC	sbDetect
PUBLIC	sbInit
PUBLIC	sbClose




;/***************************************************************************\
;*
;* Function:	sbWait
;*
;* Description: Waits until data can be written to the DSP command/data port
;*		2xCh
;*
;* Destroys:	ax, cx, dx. dx now contains the DSP command/data port value,
;*		2xCh.
;*
;\***************************************************************************/

PROC NOLANGUAGE sbWait	NEAR

	mov	dx,[sb22C]
	mov	cx,0FFFFh

@@wait:
	in	al,dx			; read port 22Ch
	test	al,al			; is bit 7 set?
	jns	@@ok			; if not, DSP is ready
	loop	@@wait			; read maximum of 0FFFFh times


	; The bit is still set after 0FFFFh reads, so apparently the DSP
	; is for some reason locked up. Return error.

	mov	ax,errSDFailure 	; Sound Device hardware failure
	jmp	@@done

@@ok:
	xor	ax,ax

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Macro:	SBCMD
;*
;* Description: Writes a command to SB's DSP. Jumps to label @@err if an
;*		error occurs, with the error code in ax
;*
;* Input:	command 	command
;*
;* Destroys:	see function sbCommand
;*
;\***************************************************************************/

MACRO	SBCMD	command
	mov	bl,command
	call	sbCommand
	test	ax,ax
	jnz	@@err
ENDM




;/***************************************************************************\
;*
;* Function:	sbCommand
;*
;* Description: Writes a command to SB's DSP
;*
;* Input:	bl	command
;*
;* Returns:	MIDAS error code in ax
;*
;* Destroys:	ax, dx, cx
;*
;\***************************************************************************/

PROC NOLANGUAGE sbCommand	NEAR

	call	sbWait			; wait until data or command can be
	test	ax,ax			; written to the DSP
	jnz	@@done

	mov	al,bl			; write the command
	out	dx,al

	xor	ax,ax

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	sbRead
;*
;* Description: Reads a byte from the DSP data port
;*
;* Returns:	bl		byte read
;*		ax		MIDAS error copde
;*
;* Destroys:	ax, cx, dx
;*
;\***************************************************************************/

PROC NOLANGUAGE sbRead		NEAR

	mov	dx,[SB.port]
	add	dx,0Eh			; dx = 2xEh = SB DSP Data Available
	mov	cx,0FFFFh		; port
@@wait:
	in	al,dx
	test	al,al			; wait until bit 7 is set
	js	@@dok
	loop	@@wait

	; Read port 2xEh 65535 time and bit 7 is still zero - failure
	mov	ax,errSDFailure
	jmp	@@done

@@dok:	add	dx,0Ah-0Eh		; dx = 2xAh = SB DSP Data port
	in	al,dx			; read data from port
	mov	bl,al			; and store it in bl

	xor	ax,ax			; success

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:    sbReset
;*
;* Description: Resets the Sound Blaster DSP
;*
;\***************************************************************************/

PROC    sbReset         NEAR

        mov     dx,[SB.port]
	add	dx,6
	mov	al,1			; reset SB DSP by first writing 1 to
	out	dx,al			; port 2x6h
	mov	cx,8
@@delay:
	in	al,dx			; wait for a while (3 usecs)
	loop	@@delay
	xor	al,al			; and write 0 to port 2x6h
	out	dx,al

	mov	dx,[SB.port]
	add	dx,0Eh			; SB data available port 2xEh
	mov	cx,1000

@@wd1:	in	al,dx
	test	al,al
	js	@@ok1			; wait until bit 7 (data available)
	loop	@@wd1			; is 1 or 1000 times
        jmp     @@err                   ; no data - no SB

@@ok1:	add	dx,0Ah-0Eh		; read data port (2xAh)
	mov	cx,1000

@@wd2:	in	al,dx
	cmp	al,0AAh 		; wait until data is 0AAh or 1000
	je	@@sbok			; times
	loop	@@wd2
        jmp     @@err                   ; no 0AAh - no SB

@@sbok:
        xor     ax,ax                   ; SB resetted succesfully
        jmp     @@done

@@err:
        mov     ax,errSDFailure

@@done:
        ret
ENDP




;/***************************************************************************\
;*
;* Function:	sbGetVersion
;*
;* Description: Get Sound Blaster DSP version and set up SB.cardType and
;*		sbVersion accordingly
;*
;\***************************************************************************/

PROC NOLANGUAGE sbGetVersion	NEAR

	SBCMD	0E1h			; Get DSP version number
	call	sbRead			; read version high byte
	test	ax,ax
	jnz	@@err
	mov	bh,bl
	call	sbRead			; read version low byte
	test	ax,ax
	jnz	@@err
	mov	[sbVersion],bx		; store version number

	cmp	bx,200h 		; DSP version < 2.00?
	jb	@@sb10			; if yes, SB version 1.0
	cmp	bx,200h 		; DSP version 2.00?
	je	@@sb15			; if yes, SB version 1.5
	cmp	bx,300h 		; DSP version < 3.00?
	jb	@@sb20			; if yes, SB version 2.0
	cmp	bx,400h 		; DSP version < 4.00?
	jb	@@sbPro 		; if yes, SB Pro

	; DSP version >= 4.00 - Sound Blaster 16
	mov	[SB.cardType],sb16
        mov     [SB.modes],sdMono or sdStereo or sd8bit or sd16bit
        jmp     @@ok

@@sb10:
	; SB version 1.0
	mov	[SB.cardType],sb10
        mov     [SB.modes],sdMono or sd8bit
	jmp	@@ok

@@sb15:
	; SB version 1.5
	mov	[SB.cardType],sb15
        mov     [SB.modes],sdMono or sd8bit
	jmp	@@ok

@@sb20:
	; SB version 2.0
	mov	[SB.cardType],sb20
        mov     [SB.modes],sdMono or sd8bit
	jmp	@@ok

@@sbPro:
	; SB Pro
	mov	[SB.cardType],sbPro
        mov     [SB.modes],sdMono or sdStereo or sd8bit

@@ok:
	xor	ax,ax

@@err:
	ret
ENDP



;/***************************************************************************\
;*
;* Function:	int sbDetect(int *result);
;*
;* Description: Detects Sound Blaster soundcard
;*
;* Returns:	MIDAS error code.
;*		1 stored to *result if SB was detected, 0 if not.
;*
;\***************************************************************************/

PROC	sbDetect	FAR	result : dword
USES	si,di
LOCAL	port : word, IRQ : byte, DMA : byte, highDMA : byte

	; Search for "BLASTER" environment string:
	call	mGetEnv LANG, seg blasterStr offset blasterStr
	mov	bx,dx			; was "BLASTER" environment found?
	or	bx,ax			; if not, no SB
	jz	@@nosb

	mov	es,dx			; point es:si to environment
	mov	si,ax			; string

	mov	[port],-1		; no port found
	mov	[IRQ],-1		; no IRQ found
	mov	[DMA],-1		; no DMA found
	mov	[highDMA],-1		; no High DMA found

@@envloop:
	mov	al,[es:si]		; get character from string
	inc	si
	test	al,al			; end of string?
	jz	@@strend

	and	al,not 20h		; convert to uppercase if a letter

	cmp	al,'A'                  ; Axxx - port address
	je	@@port

	cmp	al,'I'                  ; Ix - IRQ number
	je	@@irq

	cmp	al,'D'                  ; Dx - DMA channel number
	je	@@dma

	cmp	al,'H'                  ; Hx - High DMA channel number
	je	@@highdma

	jmp	@@envloop


@@port:
	; port - the following characters up to next space or \0, maximum
	; 3, are the I/O port number in hexadecimal

	mov	cx,4
	mov	di,offset convBuffer

@@ploop:
	mov	al,[es:si]
	inc	si
	cmp	al,' '
	je	@@port1
	test	al,al
	jz	@@port1
	mov	[ds:di],al		; copy port number to conversion
	inc	di			; buffer
	loop	@@ploop 		; max 3 characters

	jmp	@@nosb			; over 3 characters - bad environment

@@port1:
	mov	[byte ds:di],0		; append terminating zero

	; convert hex string to number:
	push	es
	call	mHex2Long LANG, seg convBuffer offset convBuffer
	pop	es
	cmp	ax,-1
	je	@@nosb

	mov	[port],ax
	jmp	@@envloop


@@irq:
	; IRQ - the following characters up to next space or \0, maximum
	; 2, are the IRQ number in decimal

	xor	ax,ax
	mov	al,[es:si]		; get first character
	inc	si
	cmp	al,'0'                  ; below '0'?
	jb	@@nosb			; if is, bad environment
	sub	al,'0'

	xor	bx,bx
	mov	bl,[es:si]		; next character
	cmp	bl,' '                  ; space?
	je	@@irq1
	test	bl,bl			; terminating zero?
	jz	@@irq1
	cmp	bl,'0'                  ; below '0'?
	jb	@@nosb			; if is, bad environment
	imul	ax,ax,10		; IRQ = 10*first + second
	add	ax,bx

@@irq1:
	mov	[IRQ],al
	jmp	@@envloop


@@dma:
	; DMA - the following character is the DMA channel number

	mov	al,[es:si]		; get first character
	inc	si
	cmp	al,'0'                  ; below '0'?
	jb	@@nosb			; if is, bad environment
	sub	al,'0'
	mov	[DMA],al
	jmp	@@envloop


@@highdma:
	; High DMA - the following character is the High DMA channel number

	mov	al,[es:si]		; get first character
	inc	si
	cmp	al,'0'                  ; below '0'?
	jb	@@nosb			; if is, bad environment
	sub	al,'0'
	mov	[highDMA],al
	jmp	@@envloop


@@strend:
	; End of environment string. If port, IRQ or DMA value was not found,
	; the environment string is bad
	cmp	[port],-1
	je	@@nosb
	cmp	[IRQ],-1
	je	@@nosb

	cmp	[highDMA],-1		; was high DMA channel number found?
	jne	@@high1

	cmp	[DMA],-1		; no, use normal DMA
	je	@@nosb
	jmp	@@set

@@high1:
	; High DMA channel number was found - use it as DMA channel
	mov	al,[highDMA]
	mov	[DMA],al

@@set:
	; Set detected values to card structure:
	mov	ax,[port]
	mov	[SB.port],ax
        add     ax,0Ch
        mov     [sb22C],ax
	mov	al,[IRQ]
	mov	[SB.IRQ],al
	mov	al,[DMA]
	mov	[SB.DMA],al

        call    sbReset                 ; reset the DSP
        test    ax,ax
        jnz     @@err

        cmp     [SB.cardType],sbAutoType        ; has a card type been set?
        jne     @@cardtypeset                   ; if not, detect it

        call    sbGetVersion
        test    ax,ax
        jnz     @@err

@@cardtypeset:
	les	bx,[result]
	mov	[word es:bx],1		; Sound Blaster was detected
	xor	ax,ax
	jmp	@@done

@@nosb:
	les	bx,[result]
	mov	[word es:bx],0		; Sound Blaster not detected
	xor	ax,ax
	jmp	@@done

@@err:
	ERROR	ID_sbDetect

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	int sbInit(ushort mixRate, ushort mode);
;*
;* Description: Initializes Sound Blaster
;*
;* Input:	mixRate 	mixing rate
;*		mode		output mode (see enum sdMode)
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	sbInit		FAR	mixRate : word, mode : word

	mov	ax,[SB.port]
	add	ax,0Ch			; set sb22C variable to real SB DSP
	mov	[sb22C],ax		; command port

        call    sbReset                 ; reset the DSP
        jnz     @@err

        cmp     [SB.cardType],sbAutoType        ; has a card type been set?
	jne	@@typeset
        call    sbGetVersion                    ; if not, detect it

@@typeset:
	cmp	[SB.cardType],sb16	; Sound Blaster 16?
	je	@@modeall		; if yes, all modes supported
	cmp	[SB.cardType],sbPro	; Sound Blaster Pro?
	jae	@@modestereo		; if yes, stereo is supported

	; normal Sound Blaster - only 8-bit mono
	mov	[sbMode],sd8bit or sdMono
	jmp	@@moded

@@modestereo:
	; Sound Blaster Pro - only 8-bit mono or stereo
	mov	ax,sd8bit		; 8-bit output
	test	[mode],sdMono		; is mono mode forced?
	jnz	@@smono
	or	ax,sdStereo		; no, use stereo
	jmp	@@sok
@@smono:
	or	ax,sdMono		; yes, use mono
@@sok:
	mov	[sbMode],ax		; store output mode
	jmp	@@moded


@@modeall:
	; Sound Blaster 16 - all output modes
	test	[mode],sd8bit		; force 8-bit?
	jnz	@@8b
	mov	ax,sd16bit		; if not, use 16 bits
	jmp	@@bit
@@8b:	mov	ax,sd8bit

@@bit:	test	[mode],sdMono		; force mono?
	jnz	@@mono
	or	ax,sdStereo		; if not, use stereo
	jmp	@@mst
@@mono: or	ax,sdMono

@@mst:	mov	[sbMode],ax

@@moded:
	mov	al,[SB.IRQ]
	cmp	al,7			; IRQ number > 7 ?
	ja	@@i8

	add	al,8			; no, interrupt number is IRQ+8
	jmp	@@ivect

@@i8:	add	al,70h-8		; yes, interrupt number is IRQ+68h

@@ivect:
	mov	[sbInterrupt],al	; save interrupt number

	mov	ah,35h
	int	21h			; save old IRQ vector
	mov	[word oldIRQ],bx
	mov	[word oldIRQ+2],es

	mov	al,[SB.IRQ]
	cmp	al,7			; is IRQ > 7 ?
	ja	@@i82

	mov	cl,al			; no
	in	al,21h
	mov	[oldIRQmask],al 	; save old IRQ mask
	mov	bl,not 1
	rol	bl,cl			; enable SB's IRQ
	and	al,bl
	out	21h,al
	jmp	@@idone

@@i82:	mov	cl,al
	sub	cl,8
	in	al,0A1h
	mov	[oldIRQmask],al 	; save old IRQ mask
	mov	bl,not 1
	rol	bl,cl			; enable SB's IRQ
	and	al,bl
	out	0A1h,al

@@idone:

	cmp	[SB.cardType],sb16	; Sound Blaster 16?
	jae	@@userate		; if so, the sampling rate is directly
					; used

	cmp	[SB.cardType],sb20	; Sound Blaster version < 2.0?
	jb	@@limit1		; if yes, rate limit is 21739Hz

	; Sound Blaster 2.0 or Pro - sampling rate limit is 43478Hz, so the
	; maximum Time Constant is 233
	mov	ecx,233
	jmp	@@timeconstant

@@limit1:
	; Sound Blaster 1.0 or 1.5 - sampling rate limit is 21739Hz, making
	; the maximum Time Constant 210
	mov	ecx,210

@@timeconstant:
	; Calculate the Transfer Time Constant

	movzx	ebx,[mixRate]
	test	[sbMode],sdStereo	; use stereo?
	jz	@@nostt 		; if yes, multiply rate with 2 when
	shl	ebx,1			; calculating Time Constant

@@nostt:
	mov	eax,1000000		; eax = Time Constant =
	cdq				; 256 - (1000000 / rate)
	div	ebx
	neg	eax
	add	eax,256

	test	eax,eax
	jns	@@non1			; Time Constant must be nonnegative
	xor	eax,eax

@@non1: cmp	eax,ecx 		; ecx is the maximum Time Constant
	jbe	@@noa1
	mov	eax,ecx 		; limit Time Constant to ecx value

@@noa1: mov	[sbTimeConstant],al	; store Transfer Time Constant

	mov	ebx,256
	sub	ebx,eax
	mov	eax,1000000		; calculate actual playing rate
	cdq				; (= 1000000 / (256 - TimeConstant))
	div	ebx

	test	[sbMode],sdStereo	; using stereo?
	jz	@@nostt2
	shr	eax,1			; divide with 2 to get rate

@@nostt2:
	mov	[sbRate],ax
	jmp	@@initdsm


@@userate:
	; Sound Blaster 16 - output uses the sampling rate directly
	mov	ax,[mixRate]
	mov	[sbRate],ax


@@initdsm:
        ; Calculate required mixing buffer size: (1/25th of a second)
        mov     ax,[sbRate]             ; ax = number of elements per second
        mov     bx,DMABUFLEN
        xor     dx,dx
        div     bx                      ; ax = number of elements in buffer

        mov     cx,[sbMode]             ; 16-bit output mode?
        test    cx,sd16bit
        jz      @@n16b                  ; if yes, multiply buffer length by
        shl     ax,1                    ; two - elements are two bytes

@@n16b:
        test    cx,sdStereo             ; stereo output?
        jz      @@nstb                  ; if yes, multiply buffer length by
        shl     ax,1                    ; two - double the number of elements
                                        ; compared to mono
@@nstb:
        add     ax,16                   ; make buffer length a multiple of 16
        and     ax,0FFF0h

        ; ax is now the DMA buffer length - allocate DMA buffer:
        call    dmaAllocBuffer LANG, ax, seg sbDMABuffer offset sbDMABuffer
        test    ax,ax                   ; buffer allocated succesfully?
        jnz     @@err

        mov     es,[sbDMABuffer.bsegment]
        xor     bx,bx

        ; Initialize Digital Sound Mixer using the correct mixing rate and
        ; output mode, mixing to the newly allocated buffer:
        call    dsmInit LANG, [sbRate], [sbMode], es bx, \
                [sbDMABuffer.blength]
        test    ax,ax                   ; error initializing DSM?
        jnz     @@err

        cmp     [SB.cardType],sbPro     ; if playing stereo on SB Pro
	jne	@@dmaok 		; set stereo mode and output one
	test	[sbMode],sdStereo	; silent byte before starting the
	jz	@@dmaok 		; actual transfer

	call	sbSetStereo
	test	ax,ax
	jnz	@@err

@@dmaok:
        ; Start playing the DMA buffer:
        movzx   ax,[SB.DMA]
	mov	bx,1			; use auto-initialization
        call    dmaPlayBuffer LANG, seg sbDMABuffer offset sbDMABuffer, \
                ax, bx
        test    ax,ax
        jnz     @@err


        mov     [sbBlockLength],0FFF0h  ; set DSP block length to 0FFF0h
					; samples - autoinit DMA mode takes
					; care of wrapping

	mov	bx,[SB.cardType]
	cmp	bx,sb10 		; Sound Blaster 1.0?
	je	@@v100			; if is, auto-initialize mode is not
					; available

	; set up interrupt service routine for auto-initialize mode:
	push	ds
	mov	ah,25h
	mov	al,[sbInterrupt]
	mov	dx,seg sbAutoinitIRQ
	mov	ds,dx
	mov	dx,offset sbAutoinitIRQ
	int	21h
	pop	ds

	cmp	bx,sb16 		; Sound Blaster 16?
	je	@@v400			; if is, use DSP 4.00 playing mode
					; for all output modes

	cmp	bx,sb20 		; Sound Blaster 2.0 or Pro?
	jae	@@v201			; if is, high-speed output is
					; available

	jmp	@@v200


@@v100:
	; Sound Blaster 1.0 - play using mono single-cycle mode

	; set up interrupt service routine for single-cycle mode:
	push	ds
	mov	ah,25h
	mov	al,[sbInterrupt]
	mov	dx,seg sbSingleCycleIRQ
	mov	ds,dx
	mov	dx,offset sbSingleCycleIRQ
	int	21h
	pop	ds

	; start playing:
	call	sbPlayMonoSingleCycle
	jmp	@@playing

@@v200:
	; Sound Blaster 1.5 - play using mono auto-initialize mode
	call	sbPlayMonoAutoinit
	jmp	@@playing

@@v201:
	; Sound Blaster 2.0 or Pro - high-speed output is available
	test	[sbMode],sdStereo	; use stereo?
	jnz	@@plstereo		; if yes, play using stereo mode

	cmp	[sbRate],22000		; is sampling rate over 22000Hz?
	ja	@@highspeed		; if is, use high-speed mode

	; Sound Blaster 2.0 or Pro, mono, rate <= 22000Hz - play using mono
	; auto-initialize mode
	call	sbPlayMonoAutoinit
	jmp	@@playing

@@highspeed:
	; Sound Blaster 2.0 or Pro, mono, rate > 22000Hz - play using mono
	; high-speed (auto-initialize) mode
	call	sbPlayMonoHighSpeed
	jmp	@@playing

@@plstereo:
	; Sound Blaster Pro, stereo - play using stereo
	; high-speed auto-initialize mode
	call	sbPlayStereo
	jmp	@@playing

@@v400:
	; Sound Blaster 16 - use DSP v4.00 auto-initialize mode for all output
	; modes
	call	sbPlay400

@@playing:
	test	ax,ax
	jnz	@@err

	mov	[SB.status],sdOK
	xor	ax,ax			; SB succesfully initialized
	jmp	@@done

@@sberr:
	mov	ax,errSDFailure 	; Hardware failure

@@err:	ERROR	ID_sbInit

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	sbPlayMonoSingleCycle
;*
;* Description: Starts playing the buffer using 8-bit mono Single-Cycle mode
;*
;\***************************************************************************/

PROC NOLANGUAGE sbPlayMonoSingleCycle	NEAR

	SBCMD	0D1h			; turn on DAC speaker
	SBCMD	40h			; set Transfer Time Constant
	SBCMD	[sbTimeConstant]	; Time Constant
	SBCMD	14h			; 8-bit PCM output
	SBCMD	<[byte sbBlockLength]>	  ; block length low byte
	SBCMD	<[byte sbBlockLength+1]>  ; block length high byte

	xor	ax,ax

@@err:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	sbSingleCycleIRQ
;*
;* Description: SB DSP interrupt service routine for 8-bit Single-Cycle mode
;*
;\***************************************************************************/

PROC NOLANGUAGE sbSingleCycleIRQ

	sti
	push	ax
	push	cx
	push	dx			; save all registers that will be
	push	ds			; changed

	mov	ax,@data
	mov	ds,ax

	SBCMD	14h			; 8-bit PCM output
	SBCMD	<[byte sbBlockLength]>	  ; block length low byte
	SBCMD	<[byte sbBlockLength+1]>  ; block length high byte

@@err:	; no error handling can be done here

	mov	dx,[SB.port]
	add	dx,0Eh			; acknowledge DSP interrupt
	in	al,dx

	cmp	[SB.IRQ],7
	ja	@@upirq

	mov	al,20h			; send End Of Interrupt command to
	out	20h,al			; PIC
	jmp	@@done

@@upirq:
	mov	al,20h			; send EOI to PIC #2 (IRQ > 7)
	out	0A0h,al

@@done:
	pop	ds
	pop	dx
	pop	cx
	pop	ax

	iret
ENDP




;/***************************************************************************\
;*
;* Function:	sbPlayMonoAutoinit
;*
;* Description: Starts playing the buffer using 8-bit Auto-initialize mode
;*
;\***************************************************************************/

PROC NOLANGUAGE sbPlayMonoAutoinit	NEAR

	SBCMD	0D1h			; turn on DAC speaker
	SBCMD	40h			; set DSP Transfer Time Constant
	SBCMD	[sbTimeConstant]	; Transfer Time Constant
	SBCMD	48h			; set DSP transfer block size
	SBCMD	<[byte sbBlockLength]>	  ; block length low byte
	SBCMD	<[byte sbBlockLength+1]>  ; block length high byte
	SBCMD	1Ch			; start 8-bit PCM output

	xor	ax,ax

@@err:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	sbAutoinitIRQ
;*
;* Description: SB DSP interrupt service routine for 8-bit Auto-initialize
;*		mode
;*
;\***************************************************************************/

PROC NOLANGUAGE sbAutoinitIRQ

	sti
	push	ax
	push	cx
	push	dx			; save all registers that will be
	push	ds			; changed

	mov	ax,@data
	mov	ds,ax

	test	[sbMode],sd16bit	; 16-bit output mode?
	jnz	@@16

	mov	dx,[SB.port]
	add	dx,0Eh			; acknowledge DSP interrupt
	in	al,dx
	jmp	@@ackdone

@@16:
	mov	dx,[SB.port]
	add	dx,0Fh			; acknowledge DSP interrupt
	in	al,dx

@@ackdone:
	cmp	[SB.IRQ],7
	ja	@@upirq

	mov	al,20h			; send End Of Interrupt command to
	out	20h,al			; PIC
	jmp	@@done

@@upirq:
	mov	al,20h			; send EOI to PIC #2 (IRQ > 7)
	out	0A0h,al

@@done:
	pop	ds
	pop	dx
	pop	cx
	pop	ax

	iret
ENDP




;/***************************************************************************\
;*
;* Function:	sbPlayMonoHighSpeed
;*
;* Description: Starts playing the buffer using 8-bit mono High-Speed
;*		Auto-initialize mode
;*
;\***************************************************************************/

PROC NOLANGUAGE sbPlayMonoHighSpeed	NEAR

	SBCMD	0D1h			; turn on DAC speaker
	SBCMD	40h			; set DSP transfer Time Constant
	SBCMD	[sbTimeConstant]	; transfer Time Constant
	SBCMD	48h			; set DSP transfer block size
	SBCMD	<[byte sbBlockLength]>	  ; block length low byte
	SBCMD	<[byte sbBlockLength+1]>  ; block length high byte
	SBCMD	90h			; 8-bit PCM high-speed output

	xor	ax,ax

@@err:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	sbSetStereo
;*
;* Description: Sets the SB hardware to stereo mode and plays a single
;*		silent byte. Called before starting stereo transfer on
;*		DSP < 4.00 to make sure that the channels are the right
;*		way and not reversed (left comes from left and right from
;*		right).
;*
;\***************************************************************************/

PROC NOLANGUAGE sbSetStereo	NEAR

	SBCMD	0D1h

	; set up the IRQ handler for transfer:
	mov	dx,[SB.port]
	add	dx,04h
	mov	al,0Eh
	out	dx,al			; set the mixer to stereo mode
	inc	dx
	in	al,dx
	or	al,2
	out	dx,al

;	 xor ax,ax ;!!!
;	 ret ;!!!

	push	ds
	mov	ah,25h
	mov	al,[sbInterrupt]
	mov	dx,seg @@irqhandler
	mov	ds,dx
	mov	dx,offset @@irqhandler
	int	21h
	pop	ds

	; program the DMA controller for single-cycle output:
	movzx	ax,[SB.DMA]
	call	dmaPlayBuffer LANG, \
		seg sbStereoDMABuffer offset sbStereoDMABuffer, ax, 0
	test	ax,ax
	jnz	@@err

	mov	[sbStereoOK],0

	SBCMD	14h
	SBCMD	0			; program the DSP to output one
	SBCMD	0			; silent byte (80h)

	; wait until the IRQ occurs:
@@w:
	cmp	[sbStereoOK],1
	jne	@@w

	xor	ax,ax

@@err:
	ret


@@irqhandler:
	; IRQ handler routine:

	push	ax
	push	cx
	push	dx			; save all registers that will be
	push	ds			; changed

	mov	ax,@data
	mov	ds,ax

	mov	[sbStereoOK],1		; set interrupt flag

	mov	dx,[SB.port]
	add	dx,0Eh			; acknowledge DSP interrupt
	in	al,dx

	cmp	[SB.IRQ],7
	ja	@@upirq

	mov	al,20h			; send End Of Interrupt command to
	out	20h,al			; PIC
	jmp	@@done

@@upirq:
	mov	al,20h			; send EOI to PIC #2 (IRQ > 7)
	out	0A0h,al

@@done:
	pop	ds
	pop	dx
	pop	cx
	pop	ax

	iret
ENDP




;/***************************************************************************\
;*
;* Function:	sbPlayStereo
;*
;* Description: Starts playing the buffer using 8-bit stereo High-Speed
;*		Auto-initialize mode
;*
;\***************************************************************************/

PROC NOLANGUAGE sbPlayStereo	FAR

	SBCMD	0D1h			; turn on DAC speaker
	SBCMD	40h			; set DSP transfer Time Constant
	SBCMD	[sbTimeConstant]	; transfer Time Constant

	; save output filter status and turn it off:
	mov	dx,[SB.port]
	add	dx,04h
	mov	al,0Ch
	out	dx,al
	inc	dx
	in	al,dx
	mov	[sbOutputFilter],al
	or	al,20h
	out	dx,al

	SBCMD	48h			; set DSP transfer block size
	SBCMD	<[byte sbBlockLength]>	  ; block length low byte
	SBCMD	<[byte sbBlockLength+1]>  ; block length high byte
	SBCMD	90h			; 8-bit PCM high-speed output

	xor	ax,ax

@@err:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	sbPlay400
;*
;* Description: Starts playing the buffer using the DSP 4.00 Auto-initialize
;*		transfer
;*
;\***************************************************************************/

PROC NOLANGUAGE sbPlay400	NEAR

	SBCMD	41h			; set DSP output sampling rate
	SBCMD	<[byte sbRate+1]>	; sampling rate high byte
	SBCMD	<[byte sbRate]> 	; sampling rate low byte

	test	[sbMode],sd8bit 	; 8-bit mode?
	jnz	@@8bit

	SBCMD	0B4h			; 16-bit output
	test	[sbMode],sdMono 	; mono?
	jnz	@@mono16
	SBCMD	30h			; 16-bit stereo signed PCM
	jmp	@@setlen
@@mono16:
	SBCMD	10h			; 16-bit mono signed PCM
	jmp	@@setlen

@@8bit:
	SBCMD	0C6h			; 8-bit output
	test	[sbMode],sdMono 	; mono?
	jnz	@@mono8
	SBCMD	20h			; 8-bit stereo unsigned PCM
	jmp	@@setlen
@@mono8:
	SBCMD	00h			; 8-bit mono unsigned PCM

@@setlen:
	SBCMD	<[byte sbBlockLength]>	  ; transfer length low byte
	SBCMD	<[byte sbBlockLength+1]>  ; transfer length high byte

@@err:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	int sbClose(void)
;*
;* Description: Uninitializes Sound Blaster
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	sbClose 	FAR

	cmp	[SB.status],sdOK
	je	@@sok

	mov	ax,errSDFailure
	jmp	@@err

@@sok:
	; Reset DSP _twice_ to stop playing and reset it: (In High-Speed mode
	; the first DSP reset just stops the playing. Besides, this should
	; not hurt anyone anyway.)
	mov	bx,2
	mov	dx,[SB.port]
	add	dx,06h

@@reset:
	mov	al,1			; reset SB DSP by first writing 1 to
	out	dx,al			; port 2x6h
	mov	cx,8
@@delay:
	in	al,dx			; wait for a while (3 usecs)
	loop	@@delay
	xor	al,al			; and write 0 to port 2x6h
	out	dx,al

	mov	cx,8
@@delay2:				; another delay
	in	al,dx
	loop	@@delay2

	dec	bx			; and reset again
	jnz	@@reset


	; stop DMA playing:
	movzx	ax,[SB.DMA]
	call	dmaStop LANG, ax
	test	ax,ax
	jnz	@@err

	mov	bl,[SB.IRQ]
	cmp	bl,7			; is IRQ number > 7 ?
	ja	@@i8

	mov	al,[oldIRQmask]
	out	21h,al			; restore old IRQ mask, IRQ <= 7
	jmp	@@ivect

@@i8:	mov	al,[oldIRQmask] 	; restore old IRQ mask, IRQ > 7
	out	0A1h,al

@@ivect:
	push	ds
	mov	al,[sbInterrupt]
	mov	ah,25h			; restore old IRQ vector
	lds	dx,[oldIRQ]
	int	21h
	pop	ds

	; uninitialize DSM:
	call	dsmClose LANG
	test	ax,ax
	jnz	@@err

        ; Deallocate DMA buffer:
        call    dmaFreeBuffer LANG, seg sbDMABuffer offset sbDMABuffer
        test    ax,ax
        jnz     @@err

        SBCMD   0D3h                    ; turn off the DAC speaker

	cmp	[SB.cardType],sbPro
	jne	@@stok			; using stereo on SB Pro?
	test	[sbMode],sdStereo
	jz	@@stok

	; stereo on SB Pro - restore the output filter status and set
	; hardware to mono mode:

	mov	dx,[SB.port]
	add	dx,04h			; write 04h to port 2x4h
	mov	al,0Ch
	out	dx,al
	inc	dx
	mov	al,[sbOutputFilter]	; write output filter value to 2x5h
	out	dx,al

	dec	dx
	mov	al,0Eh
	out	dx,al
	inc	dx			; turn off stereo mode
	in	al,dx
	and	al,not 02h
	out	dx,al

@@stok:
	mov	[SB.status],sdUnInitialized
	xor	ax,ax
	jmp	@@done

@@err:	ERROR	ID_sbClose

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	 int sbStartPlay(void);
;*
;* Description:  Reads DMA buffer position to dsmDMAPos. MUST be called ONCE
;*		 each time before calling loop of SoundDevice.Play() and
;*		 ModulePlayer.Play();
;*
;* Returns:	 MIDAS error code
;*
;\***************************************************************************/

PROC	sbStartPlay	FAR

        ; Get DMA playing position and store it to sbDMAPos:
        call    dmaGetPos LANG, seg sbDMABuffer offset sbDMABuffer, \
                seg sbDMAPos offset sbDMAPos
        test    ax,ax
        jnz     @@err

        ; Give the new DMA playing position to DSM:
        call    dsmSetPlayPos LANG, [sbDMAPos]
        test    ax,ax
        jnz     @@err

        jmp     @@done

@@err:
	ERROR	ID_sbStartPlay

@@done:
	ret
ENDP







END
