;*      PAS.ASM
;*
;* Pro Audio Spectrum Sound Device, v1.22
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*

;* NOTE! A lot of this code is ripped more or less directly from the PAS
;* SDK and might therefore seem messy.
;* I really do not understand some parts of this code... Perhaps I'll clear
;* it up some day when I have time.
;* (PK)



IDEAL
P386
JUMPS

INCLUDE "pas.inc"
INCLUDE "lang.inc"
INCLUDE "errors.inc"
INCLUDE "sdevice.inc"
INCLUDE "dsm.inc"
INCLUDE "dma.inc"
IFDEF __DPMI__
INCLUDE "dpmi.inc"
ENDIF



;/***************************************************************************\
;*       enum pasFunctIDs
;*       ----------------
;* Description:  ID numbers for PAS Sound Device functions
;\***************************************************************************/

enum    pasFunctIDs \
        ID_pasDetect = ID_pas, \
        ID_pasInit, \
        ID_pasClose, \
        ID_pasStartPlay



;/***************************************************************************\
;*      ENUM pasCardTypes
;*      ----------------
;* Description: Sound Card type number for Pro Audio Spectrum Sound Device
;\***************************************************************************/

ENUM    pasCardTypes \
        pasAutoType = 0, \              ; autodetect card type
        pasNormal, \                    ; Pro Audio Spectrum
        pasPlus, \                      ; Pro Audio Spectrum plus
        pas16                           ; Pro Audio Spectrum 16




DATASEG


pasSpeed	DW	?		; output rate value
pasRate 	DW	?		; actual output rate
pasMode 	DW	?		; output mode

pasDMABuffer    dmaBuffer       ?       ; DMA playing buffer
pasDMAPos       DW      ?               ; DMA playing position

IFDEF __DPMI__
regs            dpmiRealCallRegs  ?     ; DPMI interrupt calling registers
descrAllocated  DW      ?               ; 1 if hardware state table
                                        ; descriptor is allocated
ENDIF



IFDEF __PASCAL__
EXTRN   PAS : SoundDevice
ENDIF


IDATASEG

PASCONFIGBITS = sdUsePort or sdUseIRQ or sdUseDMA or sdUseMixRate or \
        sdUseOutputMode or sdUseDSM
PASMODEBITS = sdMono or sdStereo or sd8bit or sd16bit



; If compiling for Pascal, Sound Device name is pasSD, from which the data
; will be copied to Sound Device PAS, defined in Pascal.

IFDEF   __PASCAL__
SDNAM           equ     pasSD
ELSE
SDNAM           equ     PAS
ENDIF

GLOBAL  SDNAM : SoundDevice

SDNAM   SoundDevice     < \
 0, \
 PASCONFIGBITS, \
 388h, 15, 5, \
 pasAutoType, 3, \
 sdUnInitialized, \
 PASMODEBITS, \
 far ptr pasSDName, \
 far ptr pasCardNames, \
 4, far ptr pasPortAddresses, \
 far ptr pasDetect, \
 far ptr pasInit, \
 far ptr pasClose, \
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
 far ptr pasStartPlay, \
 far ptr dsmPlay >


pasSDName       DB      "Pro Audio Spectrum series Sound Device v1.22", 0

pasCardNames    DD      far ptr pasName
                DD      far ptr pasPlusName
                DD      far ptr pas16Name

pasName         DB      "Pro Audio Spectrum", 0
pasPlusName     DB      "Pro Audio Spectrum plus", 0
pas16Name       DB      "Pro Audio Spectrum 16", 0

pasPortAddresses  DW    388h, 384h, 38Ch, 288h




DATASEG


_mvTranslateCode        dw      ?       ; I/O base xor default_base

mvhwShadowPointer       dd      ?      ; points to the start of the data table

;
; These variables mirror the hardware state
;
HardwareShadowTable     db      (size MVState) dup (?)




CODESEG



PUBLIC	pasDetect
PUBLIC	pasInit
PUBLIC	pasClose




PROC	pasSearchHW	FAR
USES	si,di

;
; calculate the translation code
;
	mov	[PAS.port],di

	xor	di,DEFAULT_BASE 	; di holds the translation code

;
; grab the version # in the interrupt mask. The top few bits hold the version #
;
	mov	dx,INTRCTLR		; board ID is in MSB 3 bits
	xor	dx,di			; adjust to other address
	in	al,dx
	cmp	al,-1			; bus float meaning not present?
	je	@@bad			; yes, there is no card here

	mov	ah,al			; save an original copy
	xor	al,fICrevbits		; the top bits wont change

	out	dx,al			; send out the inverted bits
	jmp	$+2
	jmp	$+2
	in	al,dx			; get it back...

	cmp	al,ah			; both should match now...
	xchg	al,ah			; (restore without touching the flags)
	out	dx,al

	jnz	@@bad			; we have a bad board

;
; We do have hardware!
;

        mov     ax,1                    ; a valid PAS card found
        jmp     @@done


@@bad:
        xor     ax,ax                   ; we got here due to a bad board

@@done:
	ret
ENDP



PROC    pasSetVersion   NEAR
USES    di

        mov     di,[PAS.port]
	xor	di,DEFAULT_BASE 	; di holds the translation code

;
; grab the version # in the interrupt mask. The top few bits hold the version #
;
	mov	dx,INTRCTLR		; board ID is in MSB 3 bits
	xor	dx,di			; adjust to other address
	in	al,dx
	cmp	al,-1			; bus float meaning not present?
	je	@@bad			; yes, there is no card here

	mov	ah,al			; save an original copy
	xor	al,fICrevbits		; the top bits wont change

	out	dx,al			; send out the inverted bits
	jmp	$+2
	jmp	$+2
	in	al,dx			; get it back...

	cmp	al,ah			; both should match now...
	xchg	al,ah			; (restore without touching the flags)
	out	dx,al

	jnz	@@bad			; we have a bad board

	and	ax,fICrevbits		; isolate the ID bits & clear AH
	mov	cl,fICrevshr		; shift the bits into a meaningful
	shr	al,cl			; position (least signficant bits)
	mov	si,ax			; save the version #
;
; We do have hardware! Load the product bit definitions
;
	or	al,al			; is this the first version of h/w?
        jz      @@vpas                  ; yes, it's a standard PAS

        ; The card is either PAS plus or PAS 16. The only thing that
        ; actually matters here is whether the card has a 16-bit DAC or
        ; not. If yes, it's a PAS 16, otherwise assume it's PAS plus.

;
; determine the CDROM drive type, FM chip, 8/16 bit DAC, and mixer
;
	mov	dx,SLAVEMODRD		; check for the CDPC
	xor	dx,di			; modify via the translate code
	in	al,dx

	test	al,bSMRDdactyp		; 16 bit DAC?
        jz      @@vpasplus              ; no, its an 8 bit DAC - PAS plus

        ; neither PAS or PAS plus - assume PAS 16
        mov     [PAS.cardType],pas16
        mov     [PAS.modes],sdStereo or sdMono or sd16bit or sd8bit
        jmp     @@ok

@@vpas:
        ; standard PAS card
        mov     [PAS.cardType],pasNormal
        mov     [PAS.modes],sdStereo or sdMono or sd8bit
        jmp     @@ok

@@vpasplus:
        ; PAS plus
        mov     [PAS.cardType],pasPlus
        mov     [PAS.modes],sdStereo or sdMono or sd8bit
        jmp     @@ok


@@bad:
        mov     ax,errSDFailure         ; bad card - SD hardware failure
        jmp     @@done

@@ok:
        xor     ax,ax

@@done:
        ret
ENDP



;/***************************************************************************\
;*
;* Function:    int pasDetect(int *result);
;*
;* Description: Detects Pro Audio Spectrum soundcard
;*
;* Returns:     MIDAS error code.
;*              1 stored to *result if PAS was detected, 0 if not.
;*
;\***************************************************************************/

PROC    pasDetect       FAR     result : dword
USES    si,di

        ; For some oddball reason this function does not work with
        ; Borland Pascal 7 in Protected mode. Therefore when compiling for
        ; BP7 Protect Mode this function just stores 0 in *result and
        ; exits. Therefore it is important to have a reasonable setup like
        ; midasConfig() where the user can enter the correct values for
        ; Port, IRQ and DMA if they differ from the default ones.

IFDEF __DPMI__
IFDEF __PASCAL__
NOPASDETECT = 1
ENDIF
ENDIF

IFDEF NOPASDETECT

        les     bx,[result]             ; no detection for BP7 protected mode
        mov     [word es:bx],0
        xor     ax,ax
        ret
ELSE

IFDEF __DPMI__
        ; use DPMI to emulate real mode interrupt:

        mov     [regs.rEAX],0BC00h      ; make sure MVSOUND.SYS is loaded
        mov     [regs.rEBX],'??'        ; this is our way of knowing if the
        mov     [regs.rECX],0           ; hardware is actually present.
        mov     [regs.rEDX],0
        mov     [regs.rSP],0
        mov     [regs.rSS],0
        call    dpmiRealModeInt LANG, 2Fh, seg regs offset regs
        test    ax,ax
        jnz     @@err

        mov     bx,[word regs.rEBX]     ; build the result
        xor     bx,[word regs.rECX]
        xor     bx,[word regs.rEDX]
ELSE
        mov     ax,0BC00H               ; make sure MVSOUND.SYS is loaded
	mov	bx,'??'                 ; this is our way of knowing if the
	xor	cx,cx			; hardware is actually present.
	xor	dx,dx
        int     2Fh                     ; get the ID pattern
	xor	bx,cx			; build the result
	xor	bx,dx
ENDIF

        cmp     bx,'MV'                 ; if not here, exit...
        jne     @@nopas

;
; get the MVSOUND.SYS specified DMA and IRQ channel
;
IFDEF __DPMI__
        ; use DPMI to emulate real mode interrupt:
        mov     [regs.rEAX],0BC04h      ; get the DMA and IRQ numbers
        mov     [regs.rSP],0
        mov     [regs.rSS],0
        call    dpmiRealModeInt LANG, 2Fh, seg regs offset regs
        test    ax,ax
        jnz     @@err
        mov     al,[byte regs.rEBX]
        mov     [PAS.DMA],al
        mov     al,[byte regs.rECX]
        mov     [PAS.IRQ],al
ELSE
        mov     ax,0BC04h               ; get the DMA and IRQ numbers
        int     2Fh
	mov	[PAS.DMA],bl		; save the correct DMA & IRQ
	mov	[PAS.IRQ],cl
ENDIF


        ; now search for the hardware port address:


    ; search the default address

	mov	di,DEFAULT_BASE 	; try the first address
	call	pasSearchHW
        cmp     ax,1
        je      @@found

    ; search the first alternate address

	mov	di,ALT_BASE_1		; try the first alternate
	call	pasSearchHW
        cmp     ax,1
        je      @@found

    ; search the second alternate address

	mov	di,ALT_BASE_2		; try the second alternate
	call	pasSearchHW
        cmp     ax,1
        je      @@found

    ; search the third, or user requested alternate address

	mov	di,ALT_BASE_3		; try the third alternate
	call	pasSearchHW		; pass the third A, or user I/O
        cmp     ax,1
        jne     @@nopas

@@found:
        les     bx,[result]             ; point es:bx to result variable
        mov     [word es:bx],1          ; PAS detected succesfully

        cmp     [PAS.cardType],pasAutoType      ; has the card type been set?
        jne     @@verset                        ; if yes, do not detect

        call    pasSetVersion           ; set card version to card structure
        test    ax,ax
        jnz     @@err

@@verset:
        cmp     [PAS.cardType],pasNormal        ; normal PAS or PAS plus?
        je      @@no16modes                     ; if yes, 16-bit modes are
        cmp     [PAS.cardType],pasPlus          ; not available
        je      @@no16modes

        ; All output modes available:
        mov     [PAS.modes],sd8bit or sd16bit or sdMono or sdStereo
        jmp     @@ok

@@no16modes:
        ; No 16-bit modes available:
        mov     [PAS.modes],sd8bit or sdMono or sdStereo
        jmp     @@ok

@@nopas:
        les     bx,[result]
        mov     [word es:bx],0          ; no PAS found

@@ok:
        xor     ax,ax                   ; success
        jmp     @@done

@@err:
        ERROR   ID_pasDetect

@@done:
	ret
ENDIF
ENDP




;/***************************************************************************\
;*
;* Function:    int pasInit(ushort mixRate, ushort mode);
;*
;* Description: Initializes Pro Audio Spectrum
;*
;* Input:	mixRate 	mixing rate
;*		mode		output mode (see enum sdMode)
;*
;* Returns:     MIDAS error code
;*
;\***************************************************************************/

PROC	pasInit 	FAR	mixRate : word, mode : word
USES	si,di

        ; Clear hardware state table:
        mov     ax,ds
        mov     es,ax
        lea     di,[HardwareShadowTable]
        mov     cx,SIZE MVState
        xor     al,al
        cld
        rep     stosb

        mov     di,[PAS.port]           ; PAS I/O port
	call	pasSearchHW		; search for hardware at this port
        cmp     ax,1
        je      @@hwok

        mov     ax,errSDFailure         ; Sound Device hardware failure
        jmp     @@err

@@hwok:
        xor     di,DEFAULT_BASE         ; save port XOR default port
	mov	[_mvTranslateCode],di

        cmp     [PAS.cardType],pasAutoType      ; has card type been set?
        jne     @@typeset

        call    pasSetVersion           ; set card type to card structure


@@typeset:
        mov     [pasMode],0

	test	[mode],sd8bit		; force 8-bit?
	jnz	@@8b
        cmp     [PAS.cardType],pas16    ; is the card type PAS 16?
        jne     @@8b                    ; if not, use 8-bit output
	or	[pasMode],sd16bit	; if not, use 16 bits
	jmp	@@bit
@@8b:	or	[pasMode],sd8bit

@@bit:	test	[mode],sdMono		; force mono?
	jnz	@@mono
	or	[pasMode],sdStereo	; if not, use stereo
	jmp	@@mst
@@mono: or	[pasMode],sdMono

@@mst:

        ; from PAS SDK...

;
; setup a pointer to our local hardware state table
;
	mov	bx,offset HardwareShadowTable
        mov     ax,seg HardwareShadowTable
        mov     es,ax
	mov	[word mvhwShadowPointer+0],bx
        mov     [word mvhwShadowPointer+2],es

        mov     [es:bx+MVState._audiomixr],31h    ; lowest filter setting
        mov     [es:bx+MVState._crosschannel],09h ; cross channel l-2-l, r-2-r

IFDEF __DPMI__
        mov     [descrAllocated],0
ENDIF
;
; find the int 2F interface and if found, use it's state table pointer
;
IFDEF __DPMI__
        ; use DPMI to emulate real mode interrupt:

        mov     [regs.rEAX],0BC00h      ; MVSOUND.SYS ID check
        mov     [regs.rEBX],'??'
        mov     [regs.rECX],0
        mov     [regs.rEDX],0
        mov     [regs.rSP],0
        mov     [regs.rSS],0
        call    dpmiRealModeInt LANG, 2Fh, seg regs offset regs
        test    ax,ax
        jnz     @@err

        mov     bx,[word regs.rEBX]     ; build the result
        xor     bx,[word regs.rECX]
        xor     bx,[word regs.rEDX]
ELSE
        mov     ax,0BC00H               ; MVSOUND.SYS ID check
        mov     bx,'??'
        xor     cx,cx
	xor	dx,dx
        int     2Fh
        xor     bx,cx
	xor	bx,dx
ENDIF
	cmp	bx,'MV'                 ; is the int 2F interface here?
	jnz	@@spdone		; no, exit home

IFDEF __DPMI__
        mov     [regs.rEAX],0BC02h
        mov     [regs.rSS],0
        mov     [regs.rSP],0
        call    dpmiRealModeInt LANG, 2Fh, seg regs offset regs
        test    ax,ax
        jnz     @@err
        cmp     [word regs.rEAX],'MV'
        jnz     @@spdone

        ; regs.rEDX:regs.rEBX now contains a real mode pointer to the PAS
        ; hardware status table. Now we must build a DPMI descriptor to
        ; the data area and store it plus offset 0 to mvhwShadowPointer:

        mov     [word mvhwShadowPointer],0      ; offset 0

        ; Allocate descriptor from DPMI:
        call    dpmiAllocDescriptor LANG, seg mvhwShadowPointer \
                (offset mvhwShadowPointer+2)
        test    ax,ax
        jnz     @@err

        xor     eax,eax
        xor     ebx,ebx
        mov     ax,[word regs.rEDX]     ; eax = linear address of hardware
        shl     eax,4                   ; status table
        mov     bx,[word regs.rEBX]
        add     eax,ebx

        ; Set segment base address:
        call    dpmiSetSegmentBase LANG, [word mvhwShadowPointer+2], eax
        test    ax,ax
        jnz     @@err

        ; Set correct segment limit:
        mov     eax,(SIZE MVState) - 1
        call    dpmiSetSegmentLimit LANG, [word mvhwShadowPointer+2], eax
        test    ax,ax
        jnz     @@err

        mov     [descrAllocated],1      ; descriptor allocated
ELSE
	mov	ax,0BC02H		; get the pointer
	int	2fh
	cmp	ax,'MV'                 ; busy or intercepted
	jnz	@@spdone

	mov	[word mvhwShadowPointer+0],bx
	mov	[word mvhwShadowPointer+2],dx
ENDIF
;
@@spdone:

	mov	dx,INTRCTLRST			; flush any pending PCM irq
	xor	dx,[_mvTranslateCode]		; xlate the board address
	out	dx,al


	; calculate sample rate

	les	di,[mvhwShadowPointer]

	mov	eax,1193180
	xor	edx,edx
	movzx	ebx,[mixRate]
	div	ebx
	mov	[es:di+MVState._samplerate],ax	; save output speed
	mov	[pasSpeed],ax

	test	[pasMode],sdStereo
	jz	@@nostereo
	mov	ax,[pasSpeed]
	shr	ax,1			; multiply output rate with 2 if
	mov	[pasSpeed],ax		; stereo
	mov	[es:di+MVState._samplerate],ax

@@nostereo:
	mov	eax,1193180
	xor	edx,edx
	movzx	ebx,[pasSpeed]		; calculate actual output rate
	div	ebx

	test	[pasMode],sdStereo
	jz	@@nostereo2		; divide with 2 if stereo to get
	shr	eax,1			; actual output rate

@@nostereo2:
	mov	[pasRate],ax

	les	di,[mvhwShadowPointer]
	mov	al,00110110b		; 36h Timer 0 & square wave
	mov	dx,TMRCTLR
	xor	dx,[_mvTranslateCode]	; xlate the board address

	cli

	out	dx,al			; setup the mode, etc
	mov	[es:di+MVState._tmrctlr],al

	mov	ax,[es:di+MVState._samplerate]	; pre-calculated & saved in
	mov	dx,SAMPLERATE			; prior code
	xor	dx,[_mvTranslateCode]	; xlate the board address
	out	dx,al			; output the timer value

	jmp	$+2

	xchg	ah,al
	out	dx,al
	sti

	mov	dx,CROSSCHANNEL
	xor	dx,[_mvTranslateCode]

	mov	al,[es:di+MVState._crosschannel]     ; Stop PAS' DMA transfer
	or	al,bCCdrq
	mov	[es:di+MVState._crosschannel],al
	out	dx,al


        ; Calculate required mixing buffer size: (1/25th of a second)
        mov     ax,[pasRate]            ; ax = number of elements per second
        mov     bx,DMABUFLEN
        xor     dx,dx
        div     bx                      ; ax = number of elements in buffer

        mov     cx,[pasMode]            ; 16-bit output mode?
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
        call    dmaAllocBuffer LANG, ax, seg pasDMABuffer offset pasDMABuffer
        test    ax,ax                   ; buffer allocated succesfully?
        jnz     @@err

        mov     es,[pasDMABuffer.bsegment]
        xor     bx,bx

        ; Initialize Digital Sound Mixer using the correct mixing rate and
        ; output mode, mixing to the newly allocated buffer:
        call    dsmInit LANG, [pasRate], [pasMode], es bx, \
                [pasDMABuffer.blength]
        test    ax,ax                   ; error initializing DSM?
        jnz     @@err

        ; Start playing the DMA buffer:
	movzx	ax,[PAS.DMA]
	mov	bx,1			; use auto-initialization
        call    dmaPlayBuffer LANG, seg pasDMABuffer offset pasDMABuffer, \
                ax, bx
        test    ax,ax
        jnz     @@err


	les	di,[mvhwShadowPointer]
	test	[pasMode],sd16bit
	jz	@@no16bit
	mov	cx,(((NOT(bSC216bit+bSC212bit) AND 0FFh)*256) + bSC216bit)
	mov	dx,SYSCONFIG2
	xor	dx,[_mvTranslateCode]	; xlate the board address
	in	al,dx
	and	al,ch			; clear the bits
	or	al,cl			; set the appropriate bits
	out	dx,al
@@no16bit:
	mov	al,bCCmono		; get the stereo/mono mask bit
	test	[pasMode],sdStereo
	jz	@@nostereox
	sub	al,al
@@nostereox:
	or	al,bCCdac		; get the direction bit mask
	or	al,bCCenapcm		; enable the PCM state machine
	mov	dx,CROSSCHANNEL
	xor	dx,[_mvTranslateCode]	; xlate the board address

	mov	ah,0fh + bCCdrq 	; get a mask to load non PCM bits
	and	ah,[es:di+MVState._crosschannel]
					; grab all but PCM/DRQ/MONO/DIRECTION
	or	al,ah			; merge the two states
	xor	al,bCCenapcm		; disable the PCM bit
	out	dx,al			; send to the hardware
	jmp	$+2
	xor	al,bCCenapcm		; enable the PCM bit
	out	dx,al			; send to the hardware
	mov	[es:di+MVState._crosschannel],al  ; and save the new state
;
; Setup the audio filter sample bits
;
	mov	al,[es:di+MVState._audiofilt]
	or	al,(bFIsrate+bFIsbuff)	; enable the sample count/buff counters
	mov	dx,AUDIOFILT
	xor	dx,[_mvTranslateCode]	; xlate the board address
	out	dx,al
	mov	[es:di+MVState._audiofilt],al

	mov	al,[es:di+MVState._crosschannel] ; get the state
	mov	dx,CROSSCHANNEL
	xor	dx,[_mvTranslateCode]	; xlate the board address
	or	al,bCCdrq		; set the DRQ bit to control it
	out	dx,al
	mov	[es:di+MVState._crosschannel],al ; and save the new state

@@ok:
	mov	[PAS.status],sdOK

        xor     ax,ax                   ; success
        jmp     @@done

@@err:
        ERROR   ID_pasInit

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:    int pasClose()
;*
;* Description: Uninitializes Pro Audio Spectrum
;*
;* Returns:     MIDAS error code
;*
;\***************************************************************************/

PROC	pasClose	FAR

	cmp	[PAS.status],sdOK
	jne	@@err

	les	di,[mvhwShadowPointer]
;
; clear the audio filter sample bits
;
	mov	dx,AUDIOFILT
	xor	dx,[_mvTranslateCode]	  ; xlate the board address
	cli			; drop dead...
	mov	al,[es:di+MVState._audiofilt]	 ; get the state
	and	al,not (bFIsrate+bFIsbuff) ; flush the sample timer bits
	mov	[es:di+MVState._audiofilt],al	 ; save the new state
	out	dx,al

	test	[pasMode],sd16bit
	jz	@@no16bit

;
; disable the 16 bit stuff
;
	mov	dx,SYSCONFIG2
	xor	dx,[_mvTranslateCode]	  ; xlate the board address
	in	al,dx
	and	al,not bSC216bit+bSC212bit ; flush the 16 bit stuff
	out	dx,al
;
@@no16bit:

;
; clear the appropriate Interrupt Control Register bit
;
	mov	ah,bICsampbuff
	and	ah,bICsamprate+bICsampbuff
	not	ah
	mov	dx,INTRCTLR
	xor	dx,[_mvTranslateCode]	  ; xlate the board address
	in	al,dx
	and	al,ah			; kill sample timer interrupts
	out	dx,al
	mov	[es:di+MVState._intrctlr],al

	mov	al,[es:di+MVState._crosschannel] ; get the state
	mov	dx,CROSSCHANNEL
	xor	dx,[_mvTranslateCode]	; xlate the board address
	and	al,not bCCdrq		; clear the DRQ bit
	and	al,not bCCenapcm	; clear the PCM enable bit
	or	al,bCCdac
	out	dx,al

	mov	[es:di+MVState._crosschannel],al ; and save the new state

        ; Stop playing the DMA buffer:
	movzx	ax,[PAS.DMA]
        call    dmaStop LANG, ax
        test    ax,ax
        jnz     @@err

        ; Uninitialize Digital Sound Mixer:
        call    dsmClose LANG
        test    ax,ax
        jnz     @@err

        ; Deallocate DMA buffer:
        call    dmaFreeBuffer LANG, seg pasDMABuffer offset pasDMABuffer
        test    ax,ax
        jnz     @@err

IFDEF __DPMI__
        cmp     [descrAllocated],1      ; has the hardware state table
        jne     @@nodesc                ; descriptor been allocated?

        ; Deallocate descriptor:
        call    dpmiFreeDescriptor LANG, [word mvhwShadowPointer+2]
        test    ax,ax
        jnz     @@err

        mov     [descrAllocated],0

@@nodesc:
ENDIF

	mov	[PAS.status],sdUnInitialized

        xor     ax,ax
        jmp     @@done

@@err:
        ERROR   ID_pasClose

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:     int pasStartPlay(void);
;*
;* Description:  Reads DMA buffer playing position and gives it to DSM. MUST
;*               be called ONCE each time before the calling loop of
;*               SoundDevice.Play() and ModulePlayer.Play();
;*
;* Returns:      MIDAS error code
;*
;\***************************************************************************/

PROC    pasStartPlay    FAR

        ; Get DMA playing position and store it to pasDMAPos:
        call    dmaGetPos LANG, seg pasDMABuffer offset pasDMABuffer, \
                seg pasDMAPos offset pasDMAPos
        test    ax,ax
        jnz     @@err

        ; Give the new DMA playing position to DSM:
        call    dsmSetPlayPos LANG, [pasDMAPos]
        test    ax,ax
        jnz     @@err

        jmp     @@done

@@err:
        ERROR   ID_pasStartPlay

@@done:
        ret
ENDP




END
