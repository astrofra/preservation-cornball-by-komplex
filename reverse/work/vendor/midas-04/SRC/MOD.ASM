;*	MOD.ASM
;*
;* Protracker Module Player, v1.36
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
INCLUDE "mglobals.inc"
INCLUDE "mod.inc"
INCLUDE "mplayer.inc"
INCLUDE "sdevice.inc"
IFDEF __REALMODE__
INCLUDE "ems.inc"
ENDIF
INCLUDE "mmem.inc"


DATASEG


;*
;* MOD player data structures
;*

module		DD	?		; pointer to module structure
sdevice 	DD	?		; current Sound Device

clock		DD	?		; Clock constant
					; 3546895 for PAL (Protracker)
					; 3579364 for "NTSC" (Fasttracker)
					; (Real NTSC is 3579545)
pattPtr 	DD	?		; Pattern pointer
modMemPtr	DD	?		; temporary pointer used by some
					; functions
modInfo 	DD	?		; pointer to info structure
modChanInfo	DD	?		; pointer to channel info structures
updRateFunct	DD	?		; SetUpdRate() function pointer
position	DW	?		; position in song
row		DW	?		; row in pattern
songLength	DW	?		; song length (number of positions)
numChans	DW	?		; number of channels
firstSDChan	DW	?		; first Sound Device channel number
chan		DW	?		; current channel number
playOffset	DW	?		; Playing offset in the current pattern
oldOffset	DW	?
skipFlag	DW	?		; 1 if some rows should be skipped
					; next time song is played. Set by
					; pattern loop and break commands.
lowLimit	DW	?		; Lower period limit
highLimit	DW	?		; Higher period limit

setFrame	DW	?		; 1 if "set frame" (song is played),
					; 0 if not
rows		DW	?		; saved row number for GetInformation
poss		DW	?		; saved position
pats		DW	?		; saved pattern number
playCount	DB	?		; player speed counter
speed		DB	?		; playing speed, default is 6
tempo		DB	?		; playing BPM tempo
masterVolume	DB	?		; master volume (0-64)
pbFlag		DB	?		; pattern break flag
loopCnt 	DB	?		; song loop counter

delayCount	DB	?		; pattern delay count
delayFlag	DB	?		; pattern delay flag


channels	modChannel  MPCHANNELS DUP (?)	    ; channel structures



IDATASEG



;/***************************************************************************\
;*     Protracker Module Player structure:
;\***************************************************************************/

IFDEF NOLOADERS

mpMOD ModulePlayer    < \
	far ptr modIdentify, \
	far ptr modInit, \
	far ptr modClose, \
	far ptr EmptyFunct, \
	far ptr EmptyFunct, \
	far ptr modPlayModule, \
	far ptr modStopModule, \
	far ptr modSetUpdRateFunct, \
	far ptr modPlay, \
	far ptr modSetPosition, \
	far ptr modGetInformation,\
	far ptr modSetMasterVolume >

ELSE

mpMOD ModulePlayer    < \
	far ptr modIdentify, \
	far ptr modInit, \
	far ptr modClose, \
	far ptr modLoadModule, \
	far ptr modFreeModule, \
	far ptr modPlayModule, \
	far ptr modStopModule, \
	far ptr modSetUpdRateFunct, \
	far ptr modPlay, \
	far ptr modSetPosition, \
	far ptr modGetInformation,\
	far ptr modSetMasterVolume >

ENDIF



	; sine table for vibrato:
vibratoTable	DB	0,24,49,74,97,120,141,161
		DB	180,197,212,224,235,244,250,253
		DB	255,253,250,244,235,224,212,197
		DB	180,161,141,120,97,74,49,24

	; 101% Protracker compatible period table:
LABEL	Periods 	WORD

; Tuning 0, Normal
	DW	1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,907
	DW	856,808,762,720,678,640,604,570,538,508,480,453
	DW	428,404,381,360,339,320,302,285,269,254,240,226
	DW	214,202,190,180,170,160,151,143,135,127,120,113
	DW	107,101,95,90,85,80,75,71,67,63,60,56
	DW	53,50,47,45,42,40,37,35,33,31,30,28

LABEL	Finetuned	WORD
; Tuning 0, Normal
	DW	1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960,907
; Tuning 1
	DW	1700,1604,1514,1430,1348,1274,1202,1134,1070,1010,954,900
; Tuning 2
	DW	1688,1592,1504,1418,1340,1264,1194,1126,1064,1004,948,894
; Tuning 3
	DW	1676,1582,1492,1408,1330,1256,1184,1118,1056,996,940,888
; Tuning 4
	DW	1664,1570,1482,1398,1320,1246,1176,1110,1048,990,934,882
; Tuning 5
	DW	1652,1558,1472,1388,1310,1238,1168,1102,1040,982,926,874
; Tuning 6
	DW	1640,1548,1460,1378,1302,1228,1160,1094,1032,974,920,868
; Tuning 7
	DW	1628,1536,1450,1368,1292,1220,1150,1086,1026,968,914,862
; Tuning -8
	DW	1814,1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960
; Tuning -7
	DW	1800,1700,1604,1514,1430,1350,1272,1202,1134,1070,1010,954
; Tuning -6
	DW	1788,1688,1592,1504,1418,1340,1264,1194,1126,1064,1004,948
; Tuning -5
	DW	1774,1676,1582,1492,1408,1330,1256,1184,1118,1056,996,940
; Tuning -4
	DW	1762,1664,1570,1482,1398,1320,1246,1176,1110,1048,988,934
; Tuning -3
	DW	1750,1652,1558,1472,1388,1310,1238,1168,1102,1040,982,926
; Tuning -2
	DW	1736,1640,1548,1460,1378,1302,1228,1160,1094,1032,974,920
; Tuning -1
	DW	1724,1628,1536,1450,1368,1292,1220,1150,1086,1026,968,914


IFNDEF	NOCMDNAMES

	; command name pointers:
LABEL	cmdNames	DWORD
	DD	far ptr strArpeggio
	DD	far ptr strSlideUp
	DD	far ptr strSlideDown
	DD	far ptr strTonePortamento
	DD	far ptr strVibrato
	DD	far ptr strTPortVSlide
	DD	far ptr strVibVSlide
	DD	far ptr strTremolo
	DD	far ptr strSetPanning
	DD	far ptr strSampleOffs
	DD	far ptr strVolSlide
	DD	far ptr strPosJump
	DD	far ptr strSetVol
	DD	far ptr strPattBreak
	DD	far ptr strNoCmd
	DD	far ptr strSetSpeed

	; E-command name pointers:
LABEL	ecmdNames	DWORD
	DD	far ptr strSetFilter
	DD	far ptr strFineSldUp
	DD	far ptr strFineSldDown
	DD	far ptr strGlissCtrl
	DD	far ptr strSetVibWform
	DD	far ptr strSetFinetune
	DD	far ptr strPatternLoop
	DD	far ptr strSetTremWform
	DD	far ptr strSetPanning
	DD	far ptr strRetrig
	DD	far ptr strFineVSldUp
	DD	far ptr strFineVSldDown
	DD	far ptr strNoteCut
	DD	far ptr strNoteDelay
	DD	far ptr strPattDelay
	DD	far ptr strInvLoop

strSlideUp	DB	"Slide Up",0
strSlideDown	DB	"Slide Down",0
strTonePortamento DB	"Tone Porta",0
strVibrato	DB	"Vibrato",0
strTPortVSlide	DB	"TPrt+VolSld",0
strVibVSlide	DB	"Vib+VolSld",0
strTremolo	DB	"Tremolo",0
strSetPanning	DB	"Set Panning",0
strSampleOffs	DB	"Sample Offs",0
strArpeggio	DB	"Arpeggio",0
strVolSlide	DB	"VolumeSlide",0
strPosJump	DB	"Pos. Jump",0
strPattBreak	DB	"Patt. Break",0
strSetSpeed	DB	"Set Speed",0
strSetVol	DB	"Set Volume",0

strSetFilter	DB	"Set Filter",0
strFineSldUp	DB	"FineSld Up",0
strFineSldDown	DB	"FineSld Dwn",0
strGlissCtrl	DB	"Gliss. Ctrl",0
strSetVibWform	DB	"Vib.Wavefrm",0
strSetFinetune	DB	"SetFinetune",0
strPatternLoop	DB	"Patt.Loop",0
strSetTremWform DB	"Tre.Wavefrm",0
strRetrig	DB	"Retrig Note",0
strFineVSldUp	DB	"FineVSld Up",0
strFineVSldDown DB	"FineVSldDwn",0
strNoteCut	DB	"Note Cut",0
strNoteDelay	DB	"Note Delay",0
strPattDelay	DB	"Patt.Delay",0
strInvLoop	DB	"Invert Loop",0
ENDIF
strNoCmd	DB	0


CODESEG

;/***************************************************************************\
;*
;* Function:	int modIdentify(uchar *header, int *recognized);
;*
;* Description: Checks if the header is a Protracker module header
;*
;* Input:	uchar *headeer		pointer to header, length MPHDRSIZE
;*		int *recognized 	pointer to result variable
;*
;* Returns:	MIDAS error code.
;*		*recognized set to 1 if header is a Protracker module header,
;*		0 if not
;*
;\***************************************************************************/

PROC	modIdentify	FAR	header : dword, recognized : dword

	les	bx,[header]

	mov	eax,[dword es:bx+modHeader.sign]     ; eax = header signature

	; check if the signature is valid:
	cmp	eax,".K.M"
	je	@@1
	cmp	eax,"!K!M"
	je	@@1
	cmp	eax,"4TLF"
	je	@@1
	cmp	eax,"ATCO"
	je	@@1

	mov	ebx,eax
	and	ebx,0ffffff00h
	cmp	ebx,"NHC" SHL 8                 ; xCHN-sign
	je	@@1

	mov	ebx,eax
	and	ebx,0ffff0000h
	cmp	ebx,"HC" SHL 16                 ; xxCH-sign
	je	@@1

	and	eax,0ffffffh
	cmp	eax,"ZDT"                       ; TDZx-sign
	je	@@1

	; not a Protracker module
	xor	ax,ax
	jmp	@@iddone

@@1:	mov	ax,1

@@iddone:
	les	bx,[recognized] 	; store result in *recognized
	mov	[es:bx],ax

	xor	ax,ax			; always successful
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	int modInit(SoundDevice *SD);
;*
;* Description: Initializes Protracker Module Player
;*
;* Input:	SoundDevice *SD 	pointer to Sound Device to be used
;*					for playing
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	modInit 	FAR	SDev : dword

	mov	eax,[SDev]		; store Sound Device pointer in
	mov	[sdevice],eax		; sdevice

	mov	[updRateFunct],0	; no update rate function

	xor	ax,ax			; success

	ret
ENDP



;/***************************************************************************\
;*
;* Function:	int modClose(void);
;*
;* Description: Uninitializes the Protracker Module Player
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	modClose FAR

	xor	ax,ax			; success
	ret
ENDP



;/***************************************************************************\
;*
;* Function:	int modPlayModule(mpModule *module, ushort firstSDChannel,
;*		    ushort numChannels, ushort loopStart, ushort loopEnd);
;*
;*
;* Description: Starts playing a module
;*
;* Input:	mpModule *module	pointer to the module to be played
;*		ushort firstSDChannel	first Sound Device channel to use
;*		ushort numChannels	number of channels
;*		ushort loopStart	song loop start (0 = beginning)
;*		ushort loopEnd		song loop end (use 65535 for whole
;*					song if length is unknown)
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	modPlayModule	FAR	mmod : dword, firstSDChannel : word, \
				numChannels : word, loopStart : word, \
				loopEnd : word
USES	si,di

	mov	[loopCnt],0		; reset song loop counter

	mov	eax,[mmod]		; store module pointer in module
	mov	[module],eax
	les	si,[module]		; point es:si to module structure

	mov	ax,[es:si+mpModule.songLength]	; get song length from module
	mov	[songLength],ax 		; and store it

	mov	ax,[es:si+mpModule.flags]
	mov	[ALE],0
	test	ax,1 SHL 8			; ALE?
	jz	@@noale
	mov	[ALE],1
@@noale:
	cmp	[extendedOctaves],17
	je	@@noOct
	mov	[extendedOctaves],0
	test	ax,1 SHL 9			; Extended octaves?
	jz	@@noOct
	mov	[extendedOctaves],1
@@noOct:
	mov	ax,[firstSDChannel]	; store first SD channel number
	mov	[firstSDChan],ax
	mov	ax,[numChannels]	; store number of channels
	mov	[numChans],ax

	cmp	ax,4
	jne	@@ft
	mov	[clock],3546895 	; Protracker (PAL) clock constant
	jmp	@@set
@@ft:
	mov	[clock],3579364 	; Fasttracker ("NTSC") clock constant

@@set:	cmp	[extendedOctaves],0
	jne	@@extended

	mov	[lowLimit],856
	mov	[highLimit],113
	jmp	@@set2

@@extended:
	mov	[lowLimit],1712
	mov	[highLimit],28

	; initialize player internal variables:
@@set2: mov	[playOffset],2		; skip pattern length word
	mov	[position],0
	mov	[row],0
	mov	[masterVolume],64
	mov	[playCount],0
	mov	[speed],6		; initial speed is 6
	mov	[tempo],125		; initial BPM tempo is 125
	mov	[pbFlag],0
	mov	[delayCount],0
	mov	[delayFlag],0
	mov	[skipFlag],0

	lgs	di,[sdevice]
	call	[gs:di+SoundDevice.SetUpdRate] LANG, 5000
	test	ax,ax
	jnz	@@err


	; Set default panning values for all channels:

	mov	[chan],0

	; set initial panning values to channels:
@@panloop:
	mov	bx,[chan]
	movsx	ax,[es:si+bx+mpModule.chanSettings]
	add	bx,[firstSDChan]		; bx = Sound Device channel
						; number

	; set Sound Device panning:
	push	es gs
	call	[gs:di+SoundDevice.SetPanning] LANG, bx, ax
	pop	gs es
	test	ax,ax
	jnz	@@err

	inc	[chan]				; next channel
	mov	ax,[chan]
	cmp	ax,[numChans]
	jb	@@panloop

	; clear player channel structures:
	mov	ax,ds
	mov	es,ax
	mov	di,offset channels
	mov	cx,MPCHANNELS * SIZE modChannel
	xor	al,al
	cld
	rep	stosb


	; Allocate memory for info structure for modGetInformation():
	call	memAlloc LANG, SIZE mpInformation, seg modInfo offset modInfo
	test	ax,ax
	jnz	@@done

	; Allocate memory for channel info structures:
	mov	ax,SIZE mpChanInfo
	mul	[numChans]		; ax = total size in bytes
	call	memAlloc LANG, ax, seg modChanInfo offset modChanInfo


	xor	ax,ax			; success
	jmp	@@done

@@err:
	ERROR	ID_modPlayModule

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	int modStopModule(void);
;*
;* Description: Stops playing a module
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	modStopModule	FAR

	mov	[module],0		; point module to NULL for safety

	; deallocate information structure:
	call	memFree LANG, [modInfo]
	test	ax,ax
	jnz	@@err

	; deallocate channel information structures:
	call	memFree LANG, [modChanInfo]
	test	ax,ax
	jnz	@@err

	xor	ax,ax			; success
	jmp	@@done

@@err:
	ERROR	ID_modStopModule

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	int modSetUpdRateFunct(int (*SetUpdRate)(ushort updRate));
;*
;* Description: Changes the function which will be called when the song
;*		update rate (ie. player polling rate and tempo) is changed.
;*		Usually tmrSetUpdRate().
;*
;* Input:	int (*SetUpdRate)(ushort updRate)   Update rate changing
;*						    function. Must have similar
;*						    calling convention as
;*						    tmrSetUpdRate(). If NULL
;*						    no function is called.
;*
;* Returns:	MIDAS error code.
;*
;\***************************************************************************/

PROC	modSetUpdRateFunct	FAR	SetUpdRate : dword

	mov	eax,[SetUpdRate]	; copy update rate changing function
	mov	[updRateFunct],eax

	test	eax,eax
	jz	@@nour

	movzx	ax,[tempo]
	mov	bx,40			; BPM * 40 = playing rate in 100*Hz
	mul	bx
	call	[dword updRateFunct] LANG, ax	; set update rate
	test	ax,ax
	jnz	@@err

@@nour:
	xor	ax,ax			; success
	jmp	@@done

@@err:
	ERROR	ID_modSetUpdRateFunct

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	int modPlay(void);
;*
;* Description: Plays one "frame" of the module. Usually called from
;*		the timer.
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	modPlay 	FAR
USES	di, si

	inc	[playCount]		; increment player counter
	mov	al,[speed]		; if player counter is equal to the
	cmp	[playCount],al		; speed, it's time to play the song
	jne	@@noplay		; data.

	call	modPlaySong		; play one row of the song data
	test	ax,ax
	jnz	@@err
	jmp	@@ok

@@noplay:
	; Song data is not played - just process the continuous commands
	call	modRunCommands
	test	ax,ax
	jnz	@@err

@@ok:	xor	ax,ax			; success
	jmp	@@done

@@err:
	ERROR	ID_modPlay

@@done:
	ret
ENDP



;/***************************************************************************\
;*
;* Function:	modRunCommands
;*
;* Description: Processes the continuous commands
;*
;* Returns:	MIDAS error code in ax
;*
;\***************************************************************************/

PROC NOLANGUAGE modRunCommands	NEAR

	mov	[chan],0		; set channel number to 0
	mov	di,offset channels	; point ds:di to channel structures
	lgs	si,[sdevice]		; point gs:si to Sound Device

@@chanloop:
	test	[di+modChannel.flags],64
	jz	@@domppu
	movzx	bx,[di+modChannel.cmd]	; bx = command for this channel
	shl	bx,1
	movzx	ax,[di+modChannel.info] ; ax = command infobyte
	call	[contCmd+bx]		; process the command
	test	ax,ax			; error?
	jnz	@@done			; if yes, pass it on

@@domppu:
	add	di,size modChannel	; point ds:di to next channel

	mov	ax,[chan]
	inc	ax			; next channel number
	cmp	ax,[numChans]
	jae	@@no
	mov	[chan],ax
	jmp	@@chanloop

@@no:
	call	modUpdBars		; update "fake" volume bars

	; pass possible error code on
@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	modPlaySong
;*
;* Description: Plays one row of song data
;*
;* Returns:	MIDAS error code in ax
;*
;\***************************************************************************/


PROC	modPlaySong	NEAR
LOCAL	trackNum : word

	mov	[playCount],0		; reset player counter

	cmp	[delayCount],0
	je	@@nodelay

	; pattern delay counter is non-zero. Decrement it and process
	; continuous commands.

	dec	[delayCount]
	call	modRunCommands
	; pass possible error code on
	jmp	@@done


@@nodelay:
	mov	[delayFlag],0			; Pattern Delay not active
	cmp	[skipFlag],0
	je	@@noskip

	mov	[skipFlag],0
	call	SkipRows
	test	ax,ax
	jne	@@done

@@noskip:
	; clear flags from all channels:

	mov	di,offset channels
	mov	cx,[numChans]
@@cllp: mov	[di+modChannel.flags],0
	add	di,SIZE modChannel
	loop	@@cllp

	les	si,[module]		; point es:si to module structure

	mov	bx,[position]
	lgs	di,[es:si+mpModule.orders]	; point gs:di to orders
	movzx	cx,[gs:di+bx]			; cx = current pattern number
	shl	cx,2

	lgs	di,[es:si+mpModule.patterns]	; point gs:di to current
	add	di,cx				; pattern pointer

IFDEF __REALMODE__
IFNDEF NOEMS
	cmp	[useEMS],1			; is pattern in EMS?
	jne	@@noEMS

	; map pattern data to conventional memory:
	call	emsMap LANG, [dword gs:di], seg modMemPtr offset modMemPtr
	test	ax,ax
	jnz	@@done

	mov	eax,[modMemPtr] 	; store pattern data pointer
	mov	[pattPtr],eax
	jmp	@@dataok

@@noEMS:
ENDIF
ENDIF
	mov	eax,[gs:di]		; store pattern data pointer
	mov	[pattPtr],eax

@@dataok:
	les	si,[pattPtr]		; point es:si to pattern data
	mov	ax,[playOffset] 	; store current playing offset in
	mov	[oldOffset],ax		; oldOffset
	add	si,ax			; point es:si to current row data

	mov	cx,[numChans]
@@dataloop:
	mov	al,[es:si]		; al = current channel flag byte
	inc	si
	test	al,al			; is flag byte zero?
	jz	@@rowend		; if is, end of row

	xor	bx,bx
	mov	bl,al
	and	bl,31			; bx = current channel number

	mov	di,offset channels
	imul	bx,bx,size modChannel	; point ds:di to current channel
	add	di,bx			; structure

	mov	[di+modChannel.flags],al

	test	al,32			; if bit 5 of flag is 1, there is a
	jz	@@nonote		; note or instrument

	mov	dx,[es:si]			; get note number
	add	si,2

	xchg	dh,dl
	mov	bl,dh
	shr	bl,1
	mov	[di+modChannel.note],bl 	; and store it
	mov	bx,dx
	shr	bx,4
	and	bx,31
	mov	[di+modChannel.inst],bl 	; store instrument

@@nonote:
	test	al,64			; if bit 6 of flag 1 is, there is a
	jz	@@nocmd 		; command

	test	al,32
	jnz	@@note

	mov	bx,[es:si]		; get command
	mov	[di+modChannel.cmd],bl	; and store it
	add	si,2
	mov	[di+modChannel.info],bh ; store info byte
	jmp	@@nocmd

@@note: and	dx,0fh
	mov	[di+modChannel.cmd],dl	; store command
	mov	dl,[es:si]
	inc	si
	mov	[di+modChannel.info],dl ; store info byte

@@nocmd:
	dec	cx			; Max number of datas exceeded?
	jnz	@@dataloop		; get next flag byte and datas

@@rowend:
	sub	si,[word pattPtr]	; play offset = si - pattern start
	mov	[playOffset],si 	; offset

	les	si,[module]
	call	modSave 		; save values for GetInformation()


	; Process possible new values on all channels:

	mov	[chan],0		; channel number = 0
	mov	di,offset channels	; point ds:di to channel structures
	lgs	si,[sdevice]		; point gs:si to Sound Device

@@chanloop:
	test	[di+modChannel.flags],32
	jz	@@nonewnote

	xor	bx,bx
	mov	bl,[di+modChannel.inst] 	; check if there is a new
	test	bl,bl				; instrument
	jz	@@nonewinst

	mov	[di+modChannel.sample],bl	; set instrument number

	push	si
	les	si,[module]
	les	si,[es:si+mpModule.insts]	; point es:si to instruments
	dec	bx
	imul	bx,bx,SIZE mpInstrument 	; bx = offset in instruments
	add	si,bx				; point es:si to new inst
	mov	al,[es:si+mpInstrument.volume]		; al = volume
	mov	bx,[es:si+mpInstrument.sdInstHandle]	; bx = SD inst handle
	pop	si

	mov	[di+modChannel.volume],al	; set new volume
	or	[di+modChannel.status],1	; status bit 0 = 1 - new inst
	mov	[di+modChannel.coff],0		; current Sample Offset = 0

	mov	ax,[chan]		; ax = Sound Device channel number
	add	ax,[firstSDChan]

	; set Sound Device instrument:
	push	gs
	call	[gs:si+SoundDevice.SetInstrument] LANG, ax, bx
	pop	gs
	test	ax,ax			; error?
	jnz	@@done			; if so, pass it on


	cmp	[masterVolume],64	; is master volume 64
	je	@@nonewinst		; if so, current volume is OK

	mov	al,[di+modChannel.volume]
	call	SetSDVolume		; set volume to Sound Device
	test	ax,ax
	jnz	@@done


@@nonewinst:
	movzx	dx,[di+modChannel.note]
	test	dx,dx			; is there a new note?
	jz	@@nonewnote

	movzx	bx,[di+modChannel.sample]	; bx = current instrument
	test	bx,bx
	jz	@@nonewnote

	push	si
	les	si,[module]
	les	si,[es:si+mpModule.insts]
	dec	bx			; point es:si to current instrument
	imul	bx,bx,SIZE mpInstrument
	add	si,bx

	mov	al,[es:si+mpInstrument.finetune]   ; al = instrument finetune
	pop	si

	mov	[di+modChannel.snote],dx

	mov	cl,2*12
	mul	cl

	mov	cl,dl
	shr	cl,4

	and	dx,0fh
	add	dx,dx
	mov	bx,dx
	add	bx,ax

	movzx	ebx,[Finetuned+bx]	; bx = period number for this note
	shr	bx,cl			; Shift by octave number
	jz	@@nonewnote

	; check if current command is a tone portamento:
	test	[di+modChannel.flags],64
	jz	@@nno

	mov	al,[di+modChannel.cmd]
	cmp	al,3			; Tone Portamento
	je	@@tport
	cmp	al,5			; Tone Portamento + VSlide
	je	@@tport

@@nno:	mov	[di+modChannel.period],bx	; save period
	or	[di+modChannel.status],3	; status bit 1 = 1 - new note

	test	[di+modChannel.flags],64
	jz	@@nno2

	mov	ah,[di+modChannel.cmd]
	mov	al,[di+modChannel.info]
	and	ax,0FF0h		; is current command ED (Note Delay)?
	cmp	ax,0ED0h
	je	@@notedone		; if is, do not set note

@@nno2: mov	eax,[clock]		; eax = PAL clock constant
	xor	edx,edx 		; PAL clock constant / period
	idiv	ebx			; = playing rate

	mov	ebx,eax 		; ebx = playing rate

	mov	ax,[chan]		; ax = SD channel number
	add	ax,[firstSDChan]

	test	[di+modChannel.flags],64
	jz	@@koo
	cmp	[di+modChannel.cmd],9	; is command 9 (sample offset)
	je	@@smpoff

@@koo:	mov	[di+modChannel.vibPos],0	; clear vibrato position
	mov	[di+modChannel.trePos],0	; clear tremolo position

	cmp	[di+modChannel.coff],0	; if current sample offset is != 0,
	jne	@@dooffset		; do not set position

	; Start playing sound with rate ebx:
	push	es gs
	call	[gs:si+SoundDevice.PlaySound] LANG, ax, ebx
	pop	gs es
	test	ax,ax
	jnz	@@done

	jmp	@@notedone


@@dooffset:
	; sample offset - only set playing rate
	push	es gs
	call	[gs:si+SoundDevice.SetRate] LANG, ax, ebx
	pop	gs es
	test	ax,ax
	jnz	@@done

	jmp	@@setoffset

@@smpoff:
	; sample offset command
	push	es gs
	call	[gs:si+SoundDevice.SetRate] LANG, ax, ebx
	pop	gs es
	test	ax,ax
	jnz	@@done

	mov	bh,[di+modChannel.info] 	; if command infobyte is 0,
	test	bh,bh				; use previous sample offset
	jnz	@@so1				; value as new offset
	mov	bh,[di+modChannel.loff]
@@so1:
	mov	[di+modChannel.loff],bh 	; save current sample offset
	add	[di+modChannel.coff],bh 	; add infobyte to offset
	mov	al,[di+modChannel.coff]
	add	[di+modChannel.coff],bh 	; PROTRACKER FEATURE!
	mov	bh,al				; Sample offset is added
	xor	bl,bl				; twice, but the first one
	jmp	@@huu				; is used. This is due
						; to the fact that PT
						; calls sample offset routine
						; twice in "set frame"...
@@setoffset:
	; set sample offset
	xor	bl,bl			; bx = new sample playing position
	mov	bh,[di+modChannel.coff]

@@huu:	mov	ax,[chan]		; ax = SD channel number
	add	ax,[firstSDChan]

	; set playing position:
	push	es gs
	call	[gs:si+SoundDevice.SetPosition] LANG, ax, bx
	pop	gs es
	test	ax,ax
	jnz	@@done

	jmp	@@notedone


@@tport:
	; tone portamento
	mov	[di+modChannel.toPeri],bx	; store period as slide dest
	jmp	@@notedone


@@nonewnote:
	; no new note - reset period and volume

	; set period:
	call	CheckLimits
	call	SetPeriod
	test	ax,ax
	jnz	@@done

	; set volume:
	mov	al,[di+modChannel.volume]
	call	SetSDVolume
	test	ax,ax
	jnz	@@done


@@notedone:
	test	[di+modChannel.flags],64
	jz	@@koo3

	movzx	bx,[di+modChannel.cmd]	; bx = command number
	add	bx,bx
	movzx	ax,[di+modChannel.info] ; ax = command infobyte
	call	[commands+bx]		; process command
	test	ax,ax
	jnz	@@done

@@koo3:
	add	di,size modChannel	; point ds:di to next channel

	mov	ax,[chan]
	inc	ax			; next channel number
	cmp	ax,[numChans]
	jae	@@no
	mov	[chan],ax
	jmp	@@chanloop

@@no:	cmp	[pbFlag],0
	jne	@@break

	inc	[row]			; next row
	cmp	[row],64		; did we reach pattern end?
	jb	@@noend

	mov	[row],0

	; pattern end - reset playing offset and compression info on
	; all channels:

@@break:
	inc	[position]		; next song position
	mov	[playOffset],2		; skip pattern length word

	mov	bx,[songLength]
	cmp	[position],bx		; did we reach song end?
	jb	@@noend

	mov	[position],0		; restart song
	inc	[loopCnt]		; increase song loop conter

@@noend:
	mov	[pbFlag],0		; clear pattern break flag
	call	modUpdBars		; update volume bars

	xor	ax,ax			; success
@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	SetSDVolume
;*
;* Description: Sets SD volume to current channel, scaled according to
;*		masterVolume
;*
;* Input:	al		volume
;*
;* Returns:	MIDAS error code in ax
;*
;\***************************************************************************/

PROC	SetSDVolume	NEAR

	mul	[masterVolume]		; bx = volume scaled according to
	shr	ax,6			; master volume
	mov	bx,ax

	mov	ax,[chan]		; ax = Sound Device channel number
	add	ax,[firstSDChan]

	; set Sound Device volume:
	push	gs
	call	[gs:si+SoundDevice.SetVolume] LANG, ax, bx
	pop	gs

	; pass possible error code on

	ret
ENDP



;/***************************************************************************\
;*
;* Function:	SetSDPeriod
;*
;* Description: Sets Sound Device playing period for current channel
;*
;* Input:	ebx		period number
;*
;* Returns:	MIDAS error code in ax
;*
;\***************************************************************************/

PROC	SetSDPeriod	NEAR

	test	ebx,ebx 			; skip if zero period
	jz	@@ok

	movzx	ebx,bx
	mov	eax,[clock]		; eax = PAL clock constant
	xor	edx,edx 		; PAL clock constant / period
	idiv	ebx			; = playing rate

	mov	ebx,eax 		; ebx = playing rate

	mov	ax,[chan]		; ax = SD channel number
	add	ax,[firstSDChan]

	; Set Sound Device playing rate:
	push	gs
	call	[gs:si+SoundDevice.SetRate] LANG, ax, ebx
	pop	gs

	; pass possible error code on
	jmp	@@done

@@ok:
	xor	ax,ax

@@done:
	ret
ENDP





;/***************************************************************************\
;*	Protracker command processing:
;\***************************************************************************/


; Command F - Set Speed

PROC	SetSpeed	NEAR

	test	al,al			; skip if zero speed
	jz	@@ok

	cmp	[ptTempo],1		; are BPM tempos used?
	jne	@@speed 		; if not, value is always speed

	cmp	al,32			; is infobyte >= 32?
	jbe	@@speed

	; infobyte >= 32 - it is the BPM tempo, not speed

	mov	[tempo],al		; store new tempo
	xor	ah,ah

	mov	bx,40
	mul	bx			; ax = update rate in 100*Hz
	mov	bx,ax

	; set Sound Device update rate:
	push	gs bx
	call	[gs:si+SoundDevice.SetUpdRate] LANG, bx
	pop	bx gs
	test	ax,ax
	jnz	@@done

	cmp	[updRateFunct],0	; Is an update rate changing function
	jne	@@setrate		; set? If not, do not set rate

	xor	ax,ax
	jmp	@@done

@@setrate:
	; Set update rate:
	push	gs
	call	[dword updRateFunct] LANG, bx
	pop	gs

	jmp	@@done

@@speed:
	mov	[speed],al		; set speed

@@ok:	xor	ax,ax
@@done:
	ret
ENDP




; Command B - Position Jump

PROC	PositionJump	NEAR

	cmp	[position],ax		; is jump forward?
	jl	@@fwd

	inc	[loopCnt]		; no, increase song loop counter

@@fwd:
	dec	ax
	mov	[position],ax		; set new position
	mov	[pbFlag],1		; break to new pattern
	mov	[row],0 		; start from row 0
	xor	ax,ax			; success
	ret
ENDP



; Command D - Pattern Break

PROC	PatternBreak	NEAR

	mov	ah,al
	and	al,0fh
	shr	ah,4			; ax = new row (infobyte is in
	aad				; BCD)
	cmp	ax,63
	jbe	@@ok
	mov	ax,63
@@ok:	mov	[row],ax		; store new row

	mov	[skipFlag],1		; skip rows next time
	mov	[pbFlag],1		; break pattern flag on
	xor	ax,ax
	ret
ENDP




; Command C - Set Volume

PROC	SetVolume	NEAR

	cmp	al,64
	jbe	@@vok			; make sure volume is <= 64
	mov	al,64

@@vok:
	mov	[di+modChannel.volume],al
	call	SetVol

	; pass possible error code on
	ret
ENDP




; Command A - Volume Slide

PROC	VolumeSlide	NEAR

	mov	bl,[di+modChannel.volume]	; bl = current volume

	test	al,0F0h
	jnz	@@add

	; Upper nybble of infobyte is 0 - substract lower from volume
	sub	bl,al
	jns	@@setv
	xor	bl,bl
	jmp	@@setv

@@add:
	; Upper nybble of infobyte is nonzero - add it to volume
	shr	al,4
	add	bl,al
	cmp	bl,64
	jle	@@setv
	mov	bl,64
@@setv:
	or	[di+modChannel.status],1
	mov	[di+modChannel.volume],bl
	mov	al,bl
	call	SetSDVolume

	; pass possible error value on

	ret
ENDP




; Command E2 - Fine Slide Down

PROC	FineSlideDown	NEAR

	and	ax,0Fh
	cmp	[playCount],0		; no only if playCount is zero
	jne	@@ok

	call	SlideDown
	jmp	@@done

@@ok:
	xor	ax,ax			; success

@@done:
	ret
ENDP




; Command 2 - Slide Down

PROC	SlideDown NEAR

	add	[di+modChannel.period],ax
	call	CheckLimits
	call	SetPeriod
	ret
ENDP



; Command E1 - Fine Slide Up

PROC	FineSlideUp NEAR

	and	ax,0Fh
	cmp	[playCount],0		; no only if playCount is zero
	jne	@@ok

	call	SlideUp
	jmp	@@done

@@ok:
	xor	ax,ax			; success

@@done:
	ret
ENDP




; Command 1 - Slide Up

PROC	SlideUp 	NEAR
	sub	[di+modChannel.period],ax
	call	CheckLimits
	call	SetPeriod
	ret
ENDP



; make sure channel period is within limits

PROC	CheckLimits	NEAR

	mov	ax,[highLimit]
	cmp	[di+modChannel.period],ax
	jge	@@ok1
	mov	[di+modChannel.period],ax
@@ok1:
	mov	ax,[lowLimit]
	cmp	[di+modChannel.period],ax
	jle	@@ok2
	mov	[di+modChannel.period],ax
@@ok2:
	ret
ENDP




; Command 3 - Tone Portamento

PROC	TonePortamento NEAR

	test	ax,ax			; is infobyte 0?
	jnz	@@1

	movzx	ax,[di+modChannel.notePSp]	; if yes, use old speed

@@1:	mov	[di+modChannel.notePSp],al	; store portamento speed
	mov	bx,[di+modChannel.toPeri]
	test	bx,bx				; portamento destination zero?
	jz	@@setperiod			; if yes, skip

	cmp	[di+modChannel.period],bx	; should we slide up?
	jg	@@up

	; slide down:
	add	[di+modChannel.period],ax	; increase period
	cmp	[di+modChannel.period],bx	; past portamento dest?
	jl	@@setperiod
	mov	[di+modChannel.period],bx	; if yes, set to porta dest
	mov	[di+modChannel.toPeri],0	; do not slide anymore
	jmp	@@setperiod

@@up:
	; slide up:
	sub	[di+modChannel.period],ax	; decrease period
	cmp	[di+modChannel.period],bx	; past portamento dest?
	jg	@@setperiod
	mov	[di+modChannel.period],bx	; if yes, set to porta dest
	mov	[di+modChannel.toPeri],0	; do not slide anymore

@@setperiod:
	call	SetPeriod
	ret
ENDP



; Set period on channel to Sound Device

PROC	SetPeriod	NEAR

	movzx	ebx,[di+modChannel.period]

	; Set Sound Device period:
	call	SetSDPeriod

	ret
ENDP



; Command 4 - Vibrato

PROC	Vibrato 	NEAR

	test	al,0Fh			; is new vibrato depth non-zero?
	jnz	@@1

	mov	bl,[di+modChannel.vibCmd]	; bl = old vibrato infobyte
	and	bl,0Fh		       ; no, set old vibrato depth
	or	al,bl

@@1:
	test	al,0F0h 		; is new vibrato speed non-zero?
	jnz	@@2

	mov	bl,[di+modChannel.vibCmd]	; bl = old vibrato infobyte
	and	bl,0F0h 		 ; no, set old vibrato speed
	or	al,bl

@@2:
	mov	[di+modChannel.vibCmd],al	; store new vibrato infobyte

	mov	bl,[di+modChannel.vibPos]	; bx = vibrato table position
	and	bx,1Fh
	xor	eax,eax
	mov	al,[vibratoTable+bx]	; eax = vibrato value
	mov	cl,[di+modChannel.vibCmd]
	and	cl,0Fh			; multiply with depth
	mul	cl
	shr	ax,7			; divide with 128

	movzx	ebx,[di+modChannel.period]	; ebx = channel base period

	test	[di+modChannel.vibPos],32      ; is vibrato position >= 32?
	jnz	@@vibneg

	; vibrato position < 32 - positive
	add	ebx,eax
	jmp	@@setperiod

@@vibneg:
	; vibrato position >= 32 - negative
	sub	ebx,eax

@@setperiod:
	mov	al,[di+modChannel.vibCmd]	; add vibrato speed  to
	shr	al,4
	add	[di+modChannel.vibPos],al	; vibrato position

	; Set period to Sound Device:
	call	SetSDPeriod

	ret
ENDP




; Command 0 - Arpeggio

PROC NOLANGUAGE Arpeggio	NEAR

	test	ax,ax			; is infobyte zero?
	jz	@@ok			; skip if is

	mov	dx,ax			; save infobyte

	mov	bx,[di+modChannel.snote]  ; bx = last set note
	and	bx,0fh

	movzx	ax,[playCount]
	mov	cl,3			; divide player counter with 3
	div	cl
	test	ah,ah			; is modulus zero?
	jz	@@a0			; if yes, use base note
	dec	ah			; is modulus one?
	jz	@@a1			; if yes, use infobyte upper nybble

	; modulus is 2 - use infobyte lower nybble
	and	dx,0Fh
	add	bx,dx
	jmp	@@a0

@@a1:	; modulus is 1 - use infobyte upper nybble
	shr	dx,4
	add	bx,dx

@@a0:
	mov	dx,[di+modChannel.snote]
	shr	dx,4

@@a3:	cmp	bx,12
	jl	@@ok2
	sub	bx,12
	inc	dx
	jmp	@@a3

@@ok2:
	cmp	[extendedOctaves],0	; are extended octaves in use?
	jne	@@extoct

	cmp	dx,3			; is octave number <= 3?
	jbe	@@extoct		; if is, skip

	; New note octave number is > 3 - substract 3 to emulate Protracker
	; arpeggio wrapping effect. For full Protracker compatibility also
	; the finetune value should be increased by 1, but the difference
	; is marginal.

	sub	dx,3

@@extoct:
	movzx	ax,[di+modChannel.sample]	; bx = current instrument
	test	ax,ax
	jz	@@ok

	push	si

	les	si,[module]
	les	si,[es:si+mpModule.insts]
	dec	ax			; point es:si to current instrument
	imul	ax,ax,SIZE mpInstrument
	add	si,ax

	mov	al,[es:si+mpInstrument.finetune]   ; al = instrument finetune

	pop	si

	mov	cl,2*12
	mul	cl

	mov	cx,dx

	add	bx,bx
	add	bx,ax

	movzx	ebx,[Finetuned+bx]	 ; bx = period number for this note
	shr	ebx,cl			 ; Shift by octave number
	jz	@@ok

	; set period to Sound Device:
	call	SetSDPeriod
	jmp	@@done

@@ok:
	xor	ax,ax

@@done:
	ret
ENDP



; Command 5 - Tone Portamento and Volume Slide

PROC	TPortVSlide	NEAR

	call	VolumeSlide		; do volume slide
	test	ax,ax
	jnz	@@done

	xor	ax,ax
	call	TonePortamento		; do tone portamento with 0 infobyte

@@done:
	ret
ENDP




; Command 6 - Vibrato and Volume Slide

PROC	VibratoVSlide	NEAR

	call	VolumeSlide		; do volume slide
	test	ax,ax
	jnz	@@done

	xor	ax,ax
	call	Vibrato 		; do vibrato with 0 infobyte

@@done:
	ret
ENDP



; Command 8 - Set Panning

PROC	SetPanning NEAR

	cmp	[usePanning],0		; should panning command be supported?
	je	@@ok			; skip if not

	cmp	al,0A4h 		; DMP-compatible surround panning
	jne	@@nsurround		; value 0A4h

	mov	ax,panSurround		; set surround panning
	jmp	@@set

@@nsurround:
	cmp	al,128			; skip illegal panning values
	ja	@@ok

	sub	al,40h			; convert DMP panning values to
	cbw				; MIDAS (0-128) to (-64 - 64)

@@set:
	mov	bx,[chan]		; bx = Sound Device channel number
	add	bx,[firstSDChan]

	; Set Sound Device panning:
	push	gs
	call	[gs:si+SoundDevice.SetPanning], bx, ax
	pop	gs
	jmp	@@done

@@ok:
	xor	ax,ax

@@done:
	ret
ENDP


; Command E8 - 16-Step Set Panning

PROC NOLANGUAGE SetPanning16 NEAR

	cmp	[usePanning],0		; should panning command be supported?
	je	@@ok			; skip if not

	sub	ax,8
	js	@@ski
	inc	ax
@@ski:
	sal	ax,3
	mov	bx,[chan]		; bx = Sound Device channel number
	add	bx,[firstSDChan]

	cmp	ax,-8
	jl	@@sk
	cmp	ax,8
	jg	@@sk

	xor	ax,ax			; set values 7 and 8 to middle

@@sk:	; Set Sound Device panning:
	push	gs
	call	[gs:si+SoundDevice.SetPanning], bx, ax
	pop	gs
	jmp	@@done
@@ok:
	xor	ax,ax
@@done:
	ret
ENDP


; Command E9 - Retrig Note

PROC	SetRetrigNote NEAR

	mov	[di+modChannel.retrigC],1

	xor	ax,ax
	ret
ENDP



PROC	RetrigNote NEAR

	cmp	[playCount],0		; playing song data?
	jne	@@noset

	test	[di+modChannel.flags],32	; if no note, no retrig
	jz	@@juuh

	call	SetRetrigNote		; yes, just set count
	jmp	@@done

@@noset:
	and	al,0Fh			; should note be retrigged?
	cmp	[di+modChannel.retrigC],al	; (retrig count = infobyte)
	jb	@@no

	or	[di+modChannel.status],3	; note triggered
	mov	[di+modChannel.retrigC],1	; reset retrig count

	mov	ax,[chan]		; ax = Sound Device channel number
	add	ax,[firstSDChan]

	; Start from beginning of sample:
	push	gs
	call	[gs:si+SoundDevice.SetPosition], ax, 0
	pop	gs
	jmp	@@done

@@no:	inc	[di+modChannel.retrigC]
@@juuh: xor	ax,ax

@@done:
	ret
ENDP




; Command 7 - Tremolo

PROC	Tremolo 	NEAR

	test	al,0Fh			; is new tremolo depth non-zero?
	jnz	@@1

	mov	bl,[di+modChannel.treCmd]	; bl = old tremolo infobyte
	and	bl,0Fh		       ; no, set old tremolo depth
	or	al,bl

@@1:
	test	al,0F0h 		; is new tremolo speed non-zero?
	jnz	@@2

	mov	bl,[di+modChannel.treCmd]	; bl = old tremolo infobyte
	and	bl,0F0h 		 ; no, set old tremolo speed
	or	al,bl

@@2:
	mov	[di+modChannel.treCmd],al	; store new tremolo infobyte

	mov	bl,[di+modChannel.trePos]	; bx = tremolo table position
	and	bx,1Fh
	xor	eax,eax
	mov	al,[vibratoTable+bx]	; eax = tremolo value
	mov	cl,[di+modChannel.treCmd]
	and	cl,0Fh			; multiply with depth
	mul	cl
	shr	ax,7			; divide with 128


	movzx	bx,[di+modChannel.volume]	; bx = channel base volume

	test	[di+modChannel.trePos],32      ; is position >= 32 ?
	jnz	@@neg

	; Position < 32 - positive
	add	bx,ax
	jmp	@@setvol

@@neg:
	; Position >= 32 - negative
	sub	bx,ax

@@setvol:
	mov	al,[di+modChannel.treCmd]
	shr	al,4				; add tremolo speed to
	add	[di+modChannel.trePos],al	; vibrato position

	or	[di+modChannel.status],1

	; Make sure volume is within limits:
	cmp	bx,0
	jge	@@11
	xor	bx,bx
@@11:
	cmp	bx,64
	jle	@@22
	mov	bx,64
@@22:
	mov	al,bl
	; Set volume to Sound Device:
	call	SetSDVolume

	ret
ENDP




; Command E - extended commands. Infobyte upper nybble is command number

PROC	ECommand NEAR

	mov	bl,al
	and	bx,0f0h
	shr	bx,3			; bx = index to offset table
	and	ax,0Fh			; al = infobyte
	call	[ecmds+bx] LANG 	; process command

	ret
ENDP


; Command E5 - Set finetune

PROC	SetFineTune NEAR
	test	[di+modChannel.flags],32	; is there a note?
	jz	@@nonote

       ; ax = finetune

	mov	dx,[di+modChannel.snote]

	mov	cl,2*12
	mul	cl

	mov	cl,dl
	shr	cl,4

	and	dx,0fh
	add	dx,dx
	mov	bx,dx
	add	bx,ax

	mov	bx,[Finetuned+bx]	; bx = period number for this note
	shr	bx,cl			; Shift by octave number
	jz	@@nonote

	mov	[di+modChannel.period],bx	; save period

	call	SetPeriod
	jmp	@@done
@@nonote:
	xor	ax,ax
@@done: ret
ENDP


; Command E6 - Pattern Loop

PROC	PatternLoop NEAR

	cmp	[playCount],0		; do only when playing song data
	jne	@@ok

	test	al,al			; if infobyte is zero, set loop
	jz	@@setloop		; starting row

	cmp	[di+modChannel.loopCnt],0	; already looping?
	je	@@setcount

	dec	[di+modChannel.loopCnt] ; yes, just decrease loop counter
	jz	@@ok			; counter zero?
@@loop:
	xor	ax,ax
	mov	al,[di+modChannel.loopPos]
	dec	ax			; loop to saved row
	mov	[row],ax
	mov	[skipFlag],1		; skip to new row next time
	jmp	@@ok

@@setcount:
	; start looping - set loop counter
	mov	[di+modChannel.loopCnt],al	; loop counter = infobyte
	xor	ax,ax
	mov	al,[di+modChannel.loopPos]
	dec	ax
	mov	[row],ax
	mov	[skipFlag],1		; skip to new row next time
	jmp	@@ok

@@setloop:
	mov	ax,[row]		; save current row as loop destination
	mov	[di+modChannel.loopPos],al
@@ok:
	xor	ax,ax
	ret
ENDP


;/***************************************************************************\
;*
;* Function:	SkipRows
;*
;* Description: Skips rows of song data. Used by Pattern Break and Pattern
;*		Loop commands
;*
;* Input:	cx		number of rows to skip
;*
;* Returns:	MIDAS error code in ax
;*
;\***************************************************************************/

PROC	SkipRows	NEAR
LOCAL	rowCount : word, kpattPtr : dword
USES	es,gs,si,di

	mov	[playOffset],2		; skip pattern length word

	mov	cx,[row]
	test	cx,cx
	je	@@done
	mov	[rowCount],cx		; store row counter

	les	si,[module]		; point es:si to module structure

	mov	bx,[position]
	lgs	di,[es:si+mpModule.orders]	; point gs:di to orders
	movzx	cx,[gs:di+bx]			; cx = current pattern number
	shl	cx,2

	lgs	di,[es:si+mpModule.patterns]	; point gs:di to current
	add	di,cx				; pattern pointer

IFDEF __REALMODE__
IFNDEF NOEMS
	cmp	[useEMS],1			; is pattern in EMS?
	jne	@@noEMS

	; map pattern data to conventional memory:
	call	emsMap LANG, [dword gs:di], seg modMemPtr offset modMemPtr
	test	ax,ax
	jnz	@@done

	mov	eax,[modMemPtr] 	; store pattern data pointer
	mov	[kpattPtr],eax
	jmp	@@dataok

@@noEMS:
ENDIF
ENDIF
	mov	eax,[gs:di]		; store pattern data pointer
	mov	[kpattPtr],eax

@@dataok:
	les	si,[kpattPtr]		; point es:si to pattern data
	add	si,[playOffset]

@@rowloop:
	mov	cx,[numChans]
@@dataloop:
	mov	al,[es:si]		; al = current channel flag byte
	inc	si
	test	al,al			; is flag byte zero?
	jz	@@rowend		; if is, end of row

	test	al,32			; if bit 5 of flag is 1, there is a
	jz	@@nonote		; note or instrument
	add	si,2			; skip note and instrument bytes
	test	al,64			; if combined note and command,
	jz	@@nope			; command takes only one byte
	inc	si
	jmp	@@nope

@@nonote:
	test	al,64			; if bit 6 of flag is 1, there is a
	jz	@@nope			; command
	add	si,2			; skip command

@@nope:
	loop	@@dataloop		; get next flag byte and datas until
					; row end
@@rowend:
	dec	[rowCount]
	jnz	@@rowloop

	sub	si,[word kpattPtr]	; play offset = si - pattern start
	mov	[playOffset],si 	; offset

@@done:
	xor	ax,ax
	ret
ENDP



; Command EA - Fine Volume Slide Up

PROC	FineVolumeSlideUp NEAR

	cmp	[playCount],0		; do only when playing song data
	jne	@@ok

	add	[di+modChannel.volume],al	; add infobyte to volume
	cmp	[di+modChannel.volume],64	; make sure volume is within
	jle	@@set				; limits (<= 64)
	mov	[di+modChannel.volume],64

@@set:
	call	SetVol
	jmp	@@done

@@ok:
	xor	ax,ax

@@done:
	ret
ENDP




; Command EB - Fine Volume Slide Down

PROC	FineVolumeSlideDown	NEAR

	cmp	[playCount],0		; do only when playing song data
	jne	@@ok

	sub	[di+modChannel.volume],al	; substract infobyte from vol
	cmp	[di+modChannel.volume],0	; make sure volume is positive
	jge	@@set
	mov	[di+modChannel.volume],0

@@set:
	call	SetVol
	jmp	@@done

@@ok:
	xor	ax,ax

@@done:
	ret
ENDP




; Command EC - Note Cut

PROC	NoteCut 	NEAR

	cmp	[playCount],al		; cut note when play counter is equal
	jne	@@ok			; to infobyte

	; Cut note by setting colume to zero:
	mov	[di+modChannel.volume],0
	call	SetVol
	jmp	@@done

@@ok:
	xor	ax,ax

@@done:
	ret
ENDP



; Command ED - Note Delay

PROC	NoteDelay	NEAR

	cmp	[playCount],al		; start note when player count is
	jne	@@ok			; equal to infobyte

	test	[di+modChannel.flags],32	; no note, no note delay
	jz	@@ok

	movzx	ebx,[di+modChannel.period]	; ebx = period
	test	bx,bx				; skip if zero period
	jz	@@ok

	or	[di+modChannel.status],3	; note set

	mov	eax,[clock]		; eax = PAL clock constant
	xor	edx,edx 		; PAL clock constant / period
	idiv	ebx			; = playing rate

	mov	ebx,eax 		; ebx = playing rate

	mov	ax,[chan]		; ax = SD channel number
	add	ax,[firstSDChan]

	; Start playing sound:
	push	gs
	call	[gs:si+SoundDevice.PlaySound] LANG, ax, ebx
	pop	gs
	jmp	@@done

@@ok:
	xor	ax,ax

@@done:
	ret
ENDP




; Command EE - Pattern Delay

PROC	PatternDelay	NEAR

	cmp	[delayFlag],0		; do not set delay counter if pattern
	jne	@@ok			; delay is already active

	mov	[delayCount],al 	; pattern delay count = infobyte
	mov	[delayFlag],1		; pattern delay active

@@ok:
	xor	ax,ax

	ret
ENDP




;/***************************************************************************\
;*
;* Function:	SetVol
;*
;* Description: Sets current channel volume to Sound Device
;*
;* Returns:	MIDAS error code in ax
;*
;\***************************************************************************/

PROC	SetVol		NEAR

	or	[di+modChannel.status],1	; volume set
	mov	al,[di+modChannel.volume]	; al = channel volume
	mov	bl,[masterVolume]
	mul	bl
	shr	ax,6				; bx = volume scaled according
	mov	bx,ax				; to master volume

	mov	ax,[chan]		; ax = Sound Device channel number
	add	ax,[firstSDChan]

	; Set Sound Device playing volume:
	push	gs
	call	[gs:si+SoundDevice.SetVolume] LANG, ax, bx
	pop	gs

	ret
ENDP




; Do nothing - just clear ax to mark success

PROC	DoNothing	NEAR

	xor	ax,ax
	ret
ENDP




;/***************************************************************************\
;*     Calling offset tables to commands:
;\***************************************************************************/

	; Commands run when song data is played:
LABEL	commands	WORD
	DW	offset Arpeggio
	DW	offset DoNothing
	DW	offset DoNothing
	DW	offset DoNothing
	DW	offset DoNothing
	DW	offset DoNothing
	DW	offset DoNothing
	DW	offset DoNothing
	DW	offset SetPanning
	DW	offset DoNothing
	DW	offset DoNothing
	DW	offset PositionJump
	DW	offset SetVolume
	DW	offset PatternBreak
	DW	offset ECommand
	DW	offset SetSpeed

	; Continuous commands, run when song data is not played:
LABEL	contCmd 	WORD
	DW	offset Arpeggio
	DW	offset SlideUp
	DW	offset SlideDown
	DW	offset TonePortamento
	DW	offset Vibrato
	DW	offset TPortVSlide
	DW	offset VibratoVSlide
	DW	offset Tremolo
	DW	offset DoNothing
	DW	offset DoNothing
	DW	offset VolumeSlide
	DW	offset DoNothing
	DW	offset DoNothing
	DW	offset DoNothing
	DW	offset ECommand
	DW	offset DoNothing

	; Protracker extended E-commands:
LABEL	ecmds		WORD
	DW	offset DoNothing
	DW	offset FineSlideUp
	DW	offset FineSlideDown
	DW	offset DoNothing
	DW	offset DoNothing
	DW	offset SetFineTune
	DW	offset PatternLoop
	DW	offset DoNothing
	DW	offset SetPanning16
	DW	offset RetrigNote
	DW	offset FineVolumeSlideUp
	DW	offset FineVolumeSlideDown
	DW	offset NoteCut
	DW	offset NoteDelay
	DW	offset PatternDelay
	DW	offset DoNothing

ENDP



;/***************************************************************************\
;*
;* Function:	int modSetPosition(ushort pos)
;*
;* Description: Jumps to a specified position in module
;*
;* Input:	ushort	pos		Position to jump to
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	modSetPosition	FAR	pos : word
USES	di

	mov	ax,[pos]		; ax = new position

	cmp	ax,0			; new position negative?
	jge	@@ok1
	mov	ax,[songLength] 	; if is, set to end of song
	dec	ax

@@ok1:
	mov	[position],ax		; set new position
	mov	[poss],ax

	mov	[row],0 		; start from row 0

	mov	bx,[songLength]
	cmp	[position],bx		; is position past song end?
	jl	@@ok2

	mov	[position],0		; if is, start from the beginning

@@ok2:
	mov	[pbFlag],0		; clear pattern break and delay flags
	mov	[delayCount],0
	mov	[playOffset],2		; skip pattern length word
	xor	ax,ax			; success
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	int modGetInformation(mpInformation **info);
;*
;* Description: Fills the Module Player information structure
;*
;* Input:	mpInformation *info	pointer to pointer to info struct
;*
;* Returns:	MIDAS error code.
;*		Pointer to information structure is stored in *info.
;*
;\***************************************************************************/

PROC	modGetInformation	FAR	info : dword
USES	si,di,ds

	les	si,[modInfo]		; point es:si to information structure
	mov	di,offset channels	; point ds:di to channel structures

	mov	ax,[setFrame]
	mov	[es:si+mpInformation.setFrame],ax      ; copy set-frame flag
	mov	[setFrame],0			; set set-frame flag to 0

	mov	ax,[rows]
	mov	[es:si+mpInformation.row],ax
	mov	ax,[poss]			; copy saved row, position and
	mov	[es:si+mpInformation.pos],ax	; pattern numbers
	mov	ax,[pats]
	mov	[es:si+mpInformation.pattern],ax

	movzx	ax,[speed]
	mov	[es:si+mpInformation.speed],ax	       ; copy speed and tempo values
	movzx	ax,[tempo]
	mov	[es:si+mpInformation.BPM],ax

	movzx	ax,[loopCnt]			; copy song loop counter
	mov	[es:si+mpInformation.loopCnt],ax

	mov	eax,[modChanInfo]		; copy channel info pointer
	mov	[es:si+mpInformation.chans],eax

	mov	ax,[numChans]			; copy number of channels
	mov	[es:si+mpInformation.numChannels],ax

	mov	cx,[numChans]
	les	si,[modChanInfo]		; point es:si to chan. infos

@@chanloop:
	mov	[es:si+mpChanInfo.flags],0	; clear channel info flags

	mov	ax,[di+modChannel.snote]	; ax = last set note
	mov	[es:si+mpChanInfo.note],al	; store note and octave

	test	[di+modChannel.flags],32
	jz	@@nonote
	or	[es:si+mpChanInfo.flags],32

@@nonote:
	mov	al,[di+modChannel.sample]		; copy current
	mov	[es:si+mpChanInfo.instrument],al	; instrument number

	mov	al,[di+modChannel.volume]		; copy volume
	mov	[es:si+mpChanInfo.volume],al

	mov	al,[di+modChannel.volBar]		; copy volume bar
	mul	[masterVolume]
	shr	ax,6
	mov	[es:si+mpChanInfo.volumebar],al

	test	[di+modChannel.flags],64
	jz	@@ncmd

	mov	al,[di+modChannel.info] 		; copy command
	mov	[es:si+mpChanInfo.infobyte],al		; infobyte

	mov	al,[di+modChannel.cmd]
	and	al,0Fh				; if command number is
	jnz	@@cmd				; non-zero, or infobyte is
	cmp	[di+modChannel.info],0		; non-zero, there is a command
	jne	@@cmd

	; no command - point commandname to empty string:
	mov	[es:si+mpChanInfo.command],0
@@ncmd: mov	[word es:si+mpChanInfo.commandname],offset strNoCmd
	mov	[word es:si+2+mpChanInfo.commandname],seg strNoCmd
	jmp	@@cmdok

@@cmd:	or	[es:si+mpChanInfo.flags],128	; there is a command

	movzx	bx,al
	cmp	bx,0Eh				; E-command?
	jne	@@notecmd

	; the command is E-command. Store infobyte upper nybble + 10h as the
	; command number and infobyte lower nybble as infobyte:
	mov	al,[es:si+mpChanInfo.infobyte]
	shr	al,4
	movzx	bx,al
	add	al,10h
	mov	[es:si+mpChanInfo.command],al
	and	[es:si+mpChanInfo.infobyte],0Fh

IFNDEF	NOCMDNAMES
	shl	bx,2
	mov	eax,[ecmdNames+bx]			; eax = command name
	mov	[es:si+mpChanInfo.commandname],eax	; string pointer
	jmp	@@cmdok
ELSE
	jmp	@@ncmd
ENDIF

@@notecmd:
	; normal command
	mov	[es:si+mpChanInfo.command],al

IFNDEF	NOCMDNAMES
	shl	bx,2			; eax = command name string pointer
	mov	eax,[cmdNames+bx]
	mov	[es:si+mpChanInfo.commandname],eax	 ; store pointer
ELSE
	jmp	@@ncmd
ENDIF

@@cmdok:
	add	si,SIZE mpChanInfo	; next channel
	add	di,SIZE modChannel
	loop	@@chanloop

	les	di,[info]
	mov	eax,[modInfo]		; store information structure pointer
	mov	[es:di],eax		; in *info

	xor	ax,ax			; success
	ret
ENDP



;/***************************************************************************\
;*
;* Function:	int modSetMasterVolume(uchar volume)
;*
;* Description: Sets the module player master volume
;*
;* Input:	uchar  volume		  New master volume
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	modSetMasterVolume  FAR     vol : word

	mov	ax,[vol]
	cmp	al,64
	jbe	@@ok
	mov	al,64
@@ok:
	mov	[masterVolume],al
	xor	ax,ax			; success
	ret
ENDP



;/***************************************************************************\
;*
;* Function:	modSave
;*
;* Description: Saves row, position and pattern values for GetInformation()
;*
;\***************************************************************************/

PROC	modSave 	NEAR
USES	di

	mov	[setFrame],1		; set set-frame flag
	mov	ax,[row]
	mov	[rows],ax		; save row and position
	mov	bx,[position]
	mov	[poss],bx

	lgs	di,[es:si+mpModule.orders]
	movzx	bx,[gs:di+bx]		; save pattern number
	mov	[pats],bx

	xor	ax,ax

	ret
ENDP




;/***************************************************************************\
;*
;* Function:	modUpdBars
;*
;* Description: Updates "fake" volume bars
;*
;\***************************************************************************/

PROC	modUpdBars	NEAR
USES	di

	mov	di,offset channels	; point ds:di to channel structures
	mov	cx,[numChans]

@@chanloop:
	cmp	[di+modChannel.volBar],0	; is volume bar zero?
	je	@@1
	dec	[di+modChannel.volBar]		; if not, decrement it
@@1:
	test	[di+modChannel.status],1	; has volume been changed?
	jz	@@nochange
	mov	al,[di+modChannel.volume]
	test	[di+modChannel.status],2	; force new volume?
	jnz	@@force
	cmp	[di+modChannel.volBar],al	; do not force volume
	jbe	@@nochange			; is bar above volume level?
@@force:
	mov	[di+modChannel.volBar],al	; set new volume

@@nochange:
	and	[di+modChannel.status],not 3	; clear volume change bits
	add	di,SIZE modChannel		; next channel
	loop	@@chanloop

	xor	ax,ax
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	int modConvertSample(uchar *sample, ushort length);
;*
;* Description: Converts signed 8-bit sample to unsigned.
;*
;* Input:	cuchar *sample		pointer to sample data
;*		ushort length		sample length in bytes
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	modConvertSample	FAR	sample : dword, slength : word

	les	bx,[sample]		; point es:bx to sample data
	mov	cx,[slength]		; cx = sample length
	test	cx,cx			; skip if zero length
	jz	@@ok

@@lp:	xor	[byte es:bx],80h	; convert sample byte
	inc	bx
	loop	@@lp

@@ok:
	xor	ax,ax

	ret
ENDP





;/***************************************************************************\
;*
;* Function:	int modConvertTrack(void *track, ushort type,
;*			ushort *trackLen);
;*
;* Description: Converts one Protracker format track to internal format
;*
;* Input:	void *track		pointer to track data
;*		ushort type		track type (0 = normal Protracker)
;*		ushort *trackLen	pointer to track length variable
;*
;* Returns:	MIDAS error code.
;*		Converted track length stored in *trackLen.
;*
;\***************************************************************************/

PROC	modConvertPattern FAR	track : dword, chanNum : word, \
				trackLen : dword, flags : dword

LOCAL	period : word, note : word, inst : word, cmd : word, \
	chanCount : word, dataFlag : word, \
	rowCount : byte
USES	si,di

	mov	[rowCount],64		; row counter = 64
	les	si,[track]		; point es:si to track data
	lgs	di,[track]		; point gs:di to conversion dest

@@rowloop:
	mov	[chanCount],0
	mov	[dataFlag],1
@@chanloop:
	; Read period from pattern data:
	mov	ax,[es:si]
	xchg	ah,al
	mov	bh,ah
	and	ax,0FFFh
	mov	[period],ax

	; Read instrument number:
	mov	bl,[es:si+2]
	shr	bl,4
	and	bh,010h
	or	bl,bh
	xor	bh,bh
	mov	[inst],bx

	; Read command and infobyte:
	mov	ax,[es:si+2]
	xchg	ah,al
	and	ax,0FFFh
	mov	[cmd],ax

	; Find note number corresponding to the period:
	mov	[note],0		; no note
	cmp	[period],0		; is period zero?
	jne	@@search		; if yes, there is no note

	cmp	[inst],0
	je	@@nosearch

	push	es di
	les	di,[flags]
	or	[word es:di],1 SHL 8	; Instrument without note --> ALE!
	pop	di es
	jmp	@@nosearch

@@search:
	push	si
	mov	cx,6*12 		; there are 6*12 note numbers
	xor	si,si			; si = period table index
	xor	bx,bx			; current note number = 0

@@findnote:
	mov	ax,[Periods+si] 	; ax = period for this note
	cmp	[period],ax		; equal to period in pattern?
	je	@@found
	ja	@@closest		; passed the exact position, find
					; nearest note
	inc	bx			; no, try next note
	add	si,2
	loop	@@findnote

	; no note found for period - invalid pattern data
	pop	si
	jmp	@@error

@@closest:
	sub	ax,[period]		; Upper delta
	mov	dx,[period]
	sub	dx,[Periods-2+si]	; Lower delta
	cmp	ax,dx
	jae	@@found 		; Upper is closer
	dec	bx			; Lower is closer

@@found:
	; note corresponding to the period found
	push	es di
	les	di,[flags]

	mov	ax,bx
	xor	dx,dx
	mov	bx,12
	div	bx			; AX = Octave, DX = Note
	test	ax,ax
	jnz	@@noext

	or	[word es:di],1 SHL 9	; Octave 0 note!
@@noext:
	cmp	ax,3
	jbe	@@noext2
	or	[word es:di],1 SHL 9	; Over octave 3 note!
@@noext2:
	shl	ax,4
	or	ax,dx
	mov	[note],ax		; AL Upper nybble = Octave
					; AL Lower nybble = Note

@@huu:	cmp	[inst],0
	je	@@go_on
	cmp	[note],0
	je	@@ale
	mov	ax,[cmd]
	cmp	ah,3			; Also tone portamentos affect ALE...
	je	@@ale
	cmp	ah,5
	jne	@@go_on

@@ale:	or	[word es:di],1 SHL 8	; Instrument without note --> ALE!

@@go_on:
	pop	di es si

@@nosearch:
	add	si,4			; point es:si to next data

	mov	ax,[chanCount]
	and	ax,31
	xor	bx,bx

	cmp	[note],0		; is there a note?
	jne	@@note
	cmp	[inst],0		; or instrument
	je	@@nonote

@@note:
	or	al,32			; There is note or instrument
	mov	bx,[note]
	shl	bx,9
	mov	dx,[inst]
	and	dx,31
	shl	dx,4
	or	bx,dx			; BX:	7 MSB = Note
					;	5 NSB = Instrument
					;	4 LSB = Room for command
					;   ------
					;      16
@@nonote:
	cmp	[cmd],0
	je	@@nocmd

	or	al,64

	mov	[gs:di],al
	mov	cx,[cmd]
	and	cx,0fffh
	test	al,32			; Is there a note?
	jz	@@no

	or	bl,ch
	xchg	bh,bl
	mov	[gs:di+1],bx
	mov	[gs:di+3],cl
	add	di,4
	jmp	@@next

@@no:
	xchg	ch,cl
	mov	[gs:di+1],cx
	add	di,3
	jmp	@@next

@@nocmd:
	test	al,32
	jz	@@nothing

	mov	[gs:di],al
	xchg	bh,bl
	mov	[gs:di+1],bx
	add	di,3
	jmp	@@next

@@nothing:
	mov	[dataFlag],0			; There is an empty data!

@@next:
	inc	[chanCount]
	mov	ax,[chanNum]
	cmp	[chanCount],ax
	jne	@@chanloop

	; go to next row:
	cmp	[dataFlag],1
	je	@@nope

	mov	[byte gs:di],0			; Row ends!
	inc	di
@@nope:
	dec	[rowCount]
	jnz	@@rowloop

	sub	di,[word track] 	; calculate compressed length

	les	bx,[trackLen]
	mov	[es:bx],di		; store compressed length in *trackLen

	xor	ax,ax			; success
	jmp	@@done

@@error:
	mov	ax,errInvalidPatt	; invalid pattern data
	ERROR	ID_modConvertPattern

@@done:
	ret
ENDP


PROC	EmptyFunct	FAR

	xor	ax,ax
	ret
ENDP



END

