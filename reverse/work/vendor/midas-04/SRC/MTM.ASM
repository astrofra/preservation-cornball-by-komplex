;*	MTM.ASM
;*
;* Multitracker Module Player, v1.04
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
INCLUDE "mtm.inc"
INCLUDE "mplayer.inc"
INCLUDE "sdevice.inc"
IFDEF __REALMODE__
INCLUDE "ems.inc"
ENDIF
INCLUDE "mmem.inc"


DATASEG


;*
;* MTM player data structures
;*

module		DD	?		; pointer to module structure
sdevice 	DD	?		; current Sound Device

mtmMemPtr	DD	?		; temporary pointer used by some
					; functions
mtmInfo 	DD	?		; pointer to info structure
mtmChanInfo	DD	?		; pointer to channel info structures
updRateFunct	DD	?		; SetUpdRate() function pointer

position	DW	?		; position in song
row		DW	?		; row in pattern
songLength	DW	?		; song length (number of positions)
numChans	DW	?		; number of channels
firstSDChan	DW	?		; first Sound Device channel number
chan		DW	?		; current channel number
loopRow 	DW	?		; pattern loop row number
skipFlag	DW	?		; 1 if some rows should be skipped
					; next time song is played. Set by
					; pattern loop and break commands.

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

loopFlag	DB	?		; pattern loop flag
loopCount	DB	?		; pattern loop counter
delayCount	DB	?		; pattern delay count
delayFlag	DB	?		; pattern delay flag



channels	mtmChannel  MPCHANNELS DUP (?)	    ; channel structures



IDATASEG



;/***************************************************************************\
;*     Multitracker Module Player structure:
;\***************************************************************************/

IFDEF NOLOADERS

mpMTM ModulePlayer    < \
	far ptr mtmIdentify, \
	far ptr mtmInit, \
	far ptr mtmClose, \
	far ptr EmptyFunct, \
	far ptr EmptyFunct, \
	far ptr mtmPlayModule, \
	far ptr mtmStopModule, \
	far ptr mtmSetUpdRateFunct, \
	far ptr mtmPlay, \
	far ptr mtmSetPosition, \
	far ptr mtmGetInformation,\
	far ptr mtmSetMasterVolume >

ELSE

mpMTM ModulePlayer    < \
	far ptr mtmIdentify, \
	far ptr mtmInit, \
	far ptr mtmClose, \
	far ptr mtmLoadModule, \
	far ptr mtmFreeModule, \
	far ptr mtmPlayModule, \
	far ptr mtmStopModule, \
	far ptr mtmSetUpdRateFunct, \
	far ptr mtmPlay, \
	far ptr mtmSetPosition, \
	far ptr mtmGetInformation,\
	far ptr mtmSetMasterVolume >

ENDIF



	; sine table for vibrato:
vibratoTable	DB	0,24,49,74,97,120,141,161
		DB	180,197,212,224,235,244,250,253
		DB	255,253,250,244,235,224,212,197
		DB	180,161,141,120,97,74,49,24

	; ÷"100%" Multitracker compatible (own calculated)
	; period table:

LABEL	Periods 	WORD
; Tuning 0, Normal
	DW	1616,1524,1440,1356,1280,1208,1140,1076,1016,960,907
	DW	856,808,762,720,678,640,604,570,538,508,480,453
	DW	428,404,381,360,339,320,302,285,269,254,240,226
	DW	214,202,190,180,170,160,151,143,135,127,120,113
	DW	107,101,95,90,85,80,75,71,67,63,60,56
	DW	53,50,48,45

; Tuning 1
	DW	1604,1514,1430,1348,1274,1202,1134,1070,1010,954,900
	DW	850,802,757,715,674,637,601,567,535,505,477,450
	DW	425,401,379,357,337,318,300,284,268,253,239,225
	DW	213,201,189,179,169,159,150,142,134,126,119,113
	DW	106,100,94,89,84,80,75,71,67,63,59,56
	DW	53,50,47,45

; Tuning 2
	DW	1592,1504,1418,1340,1264,1194,1126,1064,1004,948,894
	DW	844,796,752,709,670,632,597,563,532,502,474,447
	DW	422,398,376,355,335,316,298,282,266,251,237,224
	DW	211,199,188,177,167,158,149,141,133,125,118,112
	DW	105,99,94,88,83,79,74,70,66,62,59,56
	DW	53,50,47,44

; Tuning 3
	DW	1582,1492,1408,1330,1256,1184,1118,1056,996,940,888
	DW	838,791,746,704,665,628,592,559,528,498,470,444
	DW	419,395,373,352,332,314,296,280,264,249,235,222
	DW	209,198,187,176,166,157,148,140,132,125,118,111
	DW	104,99,93,88,83,78,74,70,66,62,59,55
	DW	52,49,47,44

; Tuning 4
	DW	1570,1482,1398,1320,1246,1176,1110,1048,990,934,882
	DW	832,785,741,699,660,623,588,555,524,495,467,441
	DW	416,392,370,350,330,312,294,278,262,247,233,220
	DW	208,196,185,175,165,156,147,139,131,124,117,110
	DW	104,98,92,87,82,78,73,69,65,62,58,55
	DW	52,49,46,44

; Tuning 5
	DW	1558,1472,1388,1310,1238,1168,1102,1040,982,926,874
	DW	826,779,736,694,655,619,584,551,520,491,463,437
	DW	413,390,368,347,328,309,292,276,260,245,232,219
	DW	206,195,184,174,164,155,146,138,130,123,116,109
	DW	103,97,92,87,82,77,73,69,65,61,58,54
	DW	52,49,46,43

; Tuning 6
	DW	1548,1460,1378,1302,1228,1160,1094,1032,974,920,868
	DW	820,774,730,689,651,614,580,547,516,487,460,434
	DW	410,387,365,345,325,307,290,274,258,244,230,217
	DW	205,193,183,172,163,154,145,137,129,122,115,109
	DW	102,97,91,86,81,77,72,68,64,61,57,54
	DW	51,48,46,43

; Tuning 7
	DW	1536,1450,1368,1292,1220,1150,1086,1026,968,914,862
	DW	814,768,725,684,646,610,575,543,513,484,457,431
	DW	407,384,363,342,323,305,288,272,256,242,228,216
	DW	204,192,181,171,161,152,144,136,128,121,114,108
	DW	102,96,91,85,81,76,72,68,64,60,57,54
	DW	51,48,45,43

; Tuning -8
	DW	1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,960
	DW	907,856,808,762,720,678,640,604,570,538,508,480
	DW	453,428,404,381,360,339,320,302,285,269,254,240
	DW	226,214,202,190,180,170,160,151,143,135,127,120
	DW	113,107,101,95,90,85,80,75,71,67,63,60
	DW	56,53,50,48

; Tuning -7
	DW	1700,1604,1514,1430,1350,1272,1202,1134,1070,1010,954
	DW	900,850,802,757,715,675,636,601,567,535,505,477
	DW	450,425,401,379,357,337,318,300,284,268,253,238
	DW	225,212,200,189,179,169,159,150,142,134,126,119
	DW	112,106,100,94,89,84,79,75,71,67,63,60
	DW	56,53,50,47

; Tuning -6
	DW	1688,1592,1504,1418,1340,1264,1194,1126,1064,1004,948
	DW	894,844,796,752,709,670,632,597,563,532,502,474
	DW	447,422,398,376,355,335,316,298,282,266,251,237
	DW	223,211,199,188,177,167,158,149,141,133,125,118
	DW	112,105,99,94,89,84,79,75,70,66,63,59
	DW	56,53,50,47

; Tuning -5
	DW	1676,1582,1492,1408,1330,1256,1184,1118,1056,996,940
	DW	887,838,791,746,704,665,628,592,559,528,498,470
	DW	444,419,395,373,352,332,314,296,280,264,249,235
	DW	222,209,198,187,176,166,157,148,140,132,125,118
	DW	111,105,99,93,88,83,78,74,70,66,62,59
	DW	55,52,49,47

; Tuning -4
	DW	1664,1570,1482,1398,1320,1246,1176,1110,1048,988,934
	DW	881,832,785,741,699,660,623,588,555,524,494,467
	DW	441,416,392,370,350,330,312,294,278,262,247,233
	DW	220,208,196,185,175,165,156,147,139,131,123,117
	DW	110,104,98,93,87,82,78,73,69,65,62,58
	DW	55,52,49,46

; Tuning -3
	DW	1652,1558,1472,1388,1310,1238,1168,1102,1040,982,926
	DW	875,826,779,736,694,655,619,584,551,520,491,463
	DW	437,413,390,368,347,328,309,292,276,260,245,232
	DW	219,206,195,184,174,164,155,146,138,130,123,116
	DW	109,103,97,92,87,82,77,73,69,65,61,58
	DW	55,52,49,46

; Tuning -2
	DW	1640,1548,1460,1378,1302,1228,1160,1094,1032,974,920
	DW	868,820,774,730,689,651,614,580,547,516,487,460
	DW	434,410,387,365,345,325,307,290,274,258,244,230
	DW	217,205,193,183,172,163,154,145,137,129,122,115
	DW	108,102,97,91,86,81,77,72,68,64,61,57
	DW	54,51,48,46

; Tuning -1
	DW	1628,1536,1450,1368,1292,1220,1150,1086,1026,968,914
	DW	862,814,768,725,684,646,610,575,543,513,484,457
	DW	431,407,384,363,342,323,305,288,272,256,242,228
	DW	216,203,192,181,171,161,152,144,136,128,121,114
	DW	108,102,96,91,85,81,76,72,68,64,60,57
	DW	54,51,48,45


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
;* Function:	int mtmIdentify(uchar *header, int *recognized);
;*
;* Description: Checks if the header is a Multitracker module header
;*
;* Input:	uchar *headeer		pointer to header, length MPHDRSIZE
;*		int *recognized 	pointer to result variable
;*
;* Returns:	MIDAS error code.
;*		*recognized set to 1 if header is a Multitracker module header,
;*		0 if not
;*
;\***************************************************************************/

PROC	mtmIdentify	FAR	header : dword, recognized : dword

	les	bx,[header]

	mov	eax,[dword es:bx]		; eax = header signature
	and	eax,0ffffffh
	cmp	eax,"MTM"
	je	@@1

	; not a Multitracker module
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
;* Function:	int mtmInit(SoundDevice *SD);
;*
;* Description: Initializes Multitracker Module Player
;*
;* Input:	SoundDevice *SD 	pointer to Sound Device to be used
;*					for playing
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	mtmInit 	FAR	SDev : dword

	mov	eax,[SDev]		; store Sound Device pointer in
	mov	[sdevice],eax		; sdevice

	mov	[updRateFunct],0	; no update rate change function

	xor	ax,ax			; success

	ret
ENDP



;/***************************************************************************\
;*
;* Function:	int mtmClose(void);
;*
;* Description: Uninitializes the Multitracker Module Player
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	mtmClose FAR

	xor	ax,ax			; success
	ret
ENDP



;/***************************************************************************\
;*
;* Function:	int mtmPlayModule(mpModule *module, ushort firstSDChannel,
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

PROC	mtmPlayModule	FAR	mmtm : dword, firstSDChannel : word, \
				numChannels : word, loopStart : word, \
				loopEnd : word
USES	si,di

	mov	eax,[mmtm]		; store module pointer in module
	mov	[module],eax
	les	si,[module]		; point es:si to module structure

	mov	ax,[es:si+mpModule.songLength]	; get song length from module
	mov	[songLength],ax 		; and store it

	mov	ax,[firstSDChannel]	; store first SD channel number
	mov	[firstSDChan],ax
	mov	ax,[numChannels]	; store number of channels
	mov	[numChans],ax

	; initialize player internal variables:
	mov	[position],0
	mov	[row],0
	mov	[masterVolume],64
	mov	[playCount],0
	mov	[speed],6		; initial speed is 6
	mov	[tempo],125		; initial BPM tempo is 125
	mov	[pbFlag],0
	mov	[loopRow],0
	mov	[loopFlag],0
	mov	[delayCount],0
	mov	[delayFlag],0
	mov	[skipFlag],0
	mov	[loopCnt],0

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
	mov	cx,MPCHANNELS * SIZE mtmChannel
	xor	al,al
	cld
	rep	stosb

	mov	di,offset channels
	mov	cx,[numChans]
@@clpp:
	mov	[di+mtmChannel.playoff],2	; skip pattern length word
	add	di,SIZE mtmChannel
	dec	cx
	jnz	@@clpp

	; Allocate memory for info structure for mtmGetInformation():
	call	memAlloc LANG, SIZE mpInformation, seg mtmInfo offset mtmInfo
	test	ax,ax
	jnz	@@done

	; Allocate memory for channel info structures:
	mov	ax,SIZE mpChanInfo
	mul	[numChans]		; ax = total size in bytes
	call	memAlloc LANG, ax, seg mtmChanInfo offset mtmChanInfo

	mov	[ALE],0 			; Turn off Amiga Loop Emulation

	xor	ax,ax			; success
	jmp	@@done

@@err:
	ERROR	ID_mtmPlayModule

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	int mtmStopModule(void);
;*
;* Description: Stops playing a module
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	mtmStopModule	FAR

	mov	[module],0		; point module to NULL for safety

	; deallocate information structure:
	call	memFree LANG, [mtmInfo]
	test	ax,ax
	jnz	@@err

	; deallocate channel information structures:
	call	memFree LANG, [mtmChanInfo]
	test	ax,ax
	jnz	@@err

	xor	ax,ax			; success
	jmp	@@done

@@err:
	ERROR	ID_mtmStopModule

@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	int mtmSetUpdRateFunct(int (*SetUpdRate)(ushort updRate));
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

PROC	mtmSetUpdRateFunct	FAR	SetUpdRate : dword

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
	ERROR	ID_mtmSetUpdRateFunct

@@done:
	ret
ENDP





;/***************************************************************************\
;*
;* Function:	int mtmPlay(void);
;*
;* Description: Plays one "frame" of the module. Usually called from
;*		the timer.
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	mtmPlay 	FAR
USES	di, si

	inc	[playCount]		; increment player counter
	mov	al,[speed]		; if player counter is equal to the
	cmp	[playCount],al		; speed, it's time to play the song
	jne	@@noplay		; data.

	call	mtmPlaySong		; play one row of the song data
	test	ax,ax
	jnz	@@err
	jmp	@@ok

@@noplay:
	; Song data is not played - just process the continuous commands
	call	mtmRunCommands
	test	ax,ax
	jnz	@@err

@@ok:	xor	ax,ax			; success
	jmp	@@done

@@err:
	ERROR	ID_mtmPlay

@@done:
	ret
ENDP



;/***************************************************************************\
;*
;* Function:	mtmRunCommands
;*
;* Description: Processes the continuous commands
;*
;* Returns:	MIDAS error code in ax
;*
;\***************************************************************************/

PROC NOLANGUAGE mtmRunCommands	NEAR

	mov	[chan],0		; set channel number to 0
	mov	di,offset channels	; point ds:di to channel structures
	lgs	si,[sdevice]		; point gs:si to Sound Device

@@chanloop:
	movzx	bx,[di+mtmChannel.cmd]	; bx = command for this channel
	shl	bx,1
	movzx	ax,[di+mtmChannel.info] ; ax = command infobyte
	call	[contCmd+bx]		; process the command
	test	ax,ax			; error?
	jnz	@@done			; if yes, pass it on

	add	di,size mtmChannel	; point ds:di to next channel

	mov	ax,[chan]
	inc	ax			; next channel number
	cmp	ax,[numChans]
	jae	@@no
	mov	[chan],ax
	jmp	@@chanloop

@@no:
	call	mtmUpdBars		; update "fake" volume bars

	; pass possible error code on
@@done:
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	mtmPlaySong
;*
;* Description: Plays one row of song data
;*
;* Returns:	MIDAS error code in ax
;*
;\***************************************************************************/


PROC	mtmPlaySong	NEAR
LOCAL	trackNum : word

	mov	[playCount],0		; reset player counter

	cmp	[delayCount],0
	je	@@nodelay

	; pattern delay counter is non-zero. Decrement it and process
	; continuous commands.

	dec	[delayCount]
	call	mtmRunCommands
	; pass possible error code on
	jmp	@@done


@@nodelay:
	mov	[delayFlag],0		; no pattern delay active
	cmp	[skipFlag],0		; should some rows be skipped?
	je	@@noskip

	call	mtmSkipRows
	test	ax,ax			; error?
	jnz	@@done			; if yes, pass error code on

@@noskip:
	les	si,[module]		; point es:si to module structure
	mov	bx,[position]

	lgs	di,[es:si+mpModule.orders]	; point gs:di to orders
	movzx	bx,[byte gs:di+bx]		; bx = pattern number

	mov	cx,[numChans]		; cx = number of channels
	imul	bx,cx			; bx = first track number
	mov	[trackNum],bx		; store track number

	mov	di,offset channels	; point ds:di to channel structures

	; process pattern data for all channels:

@@dataloop:
	les	si,[module]		; point es:si to module structure

	push	di
	mov	bx,[trackNum]
	lgs	di,[es:si+mpModule.patterns]  ; point gs:di to track pointers
	shl	bx,2
	mov	edx,[gs:di+bx]		; edx = pointer to track memory
	pop	di

	test	edx,edx
	jz	@@zerotrack

IFDEF __REALMODE__
IFNDEF NOEMS
	cmp	[useEMS],1		; pattern in EMS?
	jne	@@noEMS

	; map pattern data to conventional memory:
	push	cx
	call	emsMap LANG, edx, seg mtmMemPtr offset mtmMemPtr
	pop	cx
	test	ax,ax			; error?
	jnz	@@done			; if so, pass it on

	les	si,[mtmMemPtr]		; point es:si to track in
	jmp	@@dataok		; conventional memory


@@noEMS:
ENDIF
ENDIF
	mov	si,dx
	shr	edx,16			; point es:si to track data
	mov	es,dx

@@dataok:
	add	si,[di+mtmChannel.playoff]	; add playing position to si

	; es:si now points to current row of track data

	mov	dl,[es:si]		; dl = first data byte
	mov	al,dl
	shr	al,2
	mov	[di+mtmChannel.note],al
	inc	si

	mov	ah,dl			; ah = first data byte
	mov	al,[es:si]		; al = next data byte
	mov	dl,al
	inc	si
	shr	ax,4
	and	al,3fh
	mov	[di+mtmChannel.inst],al

	and	dl,0fh			; dh = command number
	mov	[di+mtmChannel.cmd],dl

	mov	al,[es:si]
	inc	si			; al = next data byte = command
	mov	[di+mtmChannel.info],al ; infobyte

	add	[di+mtmChannel.playoff],3	; 3 bytes played
	jmp	@@next

@@zerotrack:
	add	[di+mtmChannel.playoff],3	; 3 bytes played
	xor	al,al
	mov	[di+mtmChannel.note],al
	mov	[di+mtmChannel.inst],al
	mov	[di+mtmChannel.cmd],al
	mov	[di+mtmChannel.info],al

@@next: inc	[trackNum]		; next channel
	add	di,SIZE mtmChannel	; point ds:di to next channel
	loop	@@dataloop

	les	si,[module]
	call	mtmSave 		; save values for GetInformation()


	; Process possible new values on all channels:

	mov	[chan],0		; channel number = 0
	mov	di,offset channels	; point ds:di to channel structures
	lgs	si,[sdevice]		; point gs:si to Sound Device


@@chanloop:
	xor	bx,bx
	mov	bl,[di+mtmChannel.inst] 	; check if there is a new
	test	bl,bl				; instrument
	jz	@@nonewinst

	mov	[di+mtmChannel.sample],bl	; set instrument number

	push	si
	les	si,[module]
	les	si,[es:si+mpModule.insts]	; point es:si to instruments
	dec	bx
	imul	bx,bx,SIZE mpInstrument 	; bx = offset in instruments
	add	si,bx				; point es:si to new inst
	mov	al,[es:si+mpInstrument.volume]		; al = volume
	mov	bx,[es:si+mpInstrument.sdInstHandle]	; bx = SD inst handle
	pop	si

	mov	[di+mtmChannel.volume],al	; set new volume
	or	[di+mtmChannel.status],1	; status bit 0 = 1 - new inst
	mov	[di+mtmChannel.coff],0		; current Sample Offset = 0

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

	mov	al,[di+mtmChannel.volume]
	call	SetSDVolume		; set volume to Sound Device
	test	ax,ax
	jnz	@@done


@@nonewinst:
	movzx	dx,[di+mtmChannel.note]
	test	dx,dx			; is there a new note?
	jz	@@nonewnote

	movzx	bx,[di+mtmChannel.sample]	; bx = current instrument
	test	bx,bx
	jz	@@nonewnote

	push	si

	les	si,[module]
	les	si,[es:si+mpModule.insts]
	dec	bx			; point es:si to current instrument
	imul	bx,bx,SIZE mpInstrument
	add	si,bx

	mov	al,[es:si+mpInstrument.finetune]   ; al = instrument finetune
	and	al,0fh
	pop	si

	dec	dx
	add	dx,dx
	mov	bl,126
	mul	bl			; bx = period table offset for
	mov	bx,ax			; new note
	add	bx,dx

	mov	[di+mtmChannel.snote],bx	; store period table offset
	mov	bx,[Periods+bx] 	; bx = period number for this note

	; check if current command is a tone portamento:
	mov	al,[di+mtmChannel.cmd]
	cmp	al,3			; Tone Portamento
	je	@@tport
	cmp	al,5			; Tone Portamento + VSlide
	je	@@tport

	mov	[di+mtmChannel.period],bx	; save period
	or	[di+mtmChannel.status],3	; status bit 1 = 1 - new note

	mov	ah,[di+mtmChannel.cmd]
	mov	al,[di+mtmChannel.info]
	and	ax,0FF0h		; is current command ED (Note Delay)?
	cmp	ax,0ED0h
	je	@@notedone		; if is, do not set note


	movzx	ebx,bx
	mov	eax,3579545		; eax = NTSC clock constant
	cdq				; NTSC clock constant / period
	idiv	ebx			; = playing rate

	mov	ebx,eax 		; ebx = playing rate

	mov	ax,[chan]		; ax = SD channel number
	add	ax,[firstSDChan]

	cmp	[di+mtmChannel.cmd],9	; is command 9 (sample offset)
	je	@@smpoff

	mov	[di+mtmChannel.vibpos],0	; clear vibrato position
	mov	[di+mtmChannel.trepos],0	; clear tremolo position

	cmp	[di+mtmChannel.coff],0	; if current sample offset is != 0,
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

	mov	bh,[di+mtmChannel.info] 	; if command infobyte is 0,
	test	bh,bh				; use previous sample offset
	jnz	@@so1				; value as new offset
	mov	bh,[di+mtmChannel.loff]

@@so1:
	mov	[di+mtmChannel.loff],bh 	; save current sample offset
	add	[di+mtmChannel.coff],bh 	; add infobyte to offset

@@setoffset:
	; set sample offset
	xor	bl,bl			; bx = new sample playing position
	mov	bh,[di+mtmChannel.coff]

	mov	ax,[chan]		; ax = SD channel number
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
	mov	[di+mtmChannel.toperi],bx	; store period as slide dest
	jmp	@@notedone


@@nonewnote:
	; no new note - reset period and volume

	; set period:
	call	SetPeriod
	test	ax,ax
	jnz	@@done

	; set volume:
	mov	al,[di+mtmChannel.volume]
	call	SetSDVolume
	test	ax,ax
	jnz	@@done


@@notedone:
	movzx	bx,[di+mtmChannel.cmd]	; bx = command number
	add	bx,bx
	movzx	ax,[di+mtmChannel.info] ; ax = command infobyte
	call	[commands+bx]		; process command
	test	ax,ax
	jnz	@@done

	add	di,size mtmChannel	; point ds:di to next channel

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
	mov	cx,[numChans]
	mov	di,offset channels
@@l1:	mov	[di+mtmChannel.playoff],2	; skip pattern length word
	mov	[di+mtmChannel.comp],0
	add	di,size mtmChannel
	loop	@@l1

	inc	[position]		; next song position

	mov	bx,[songLength]
	cmp	[position],bx		; did we reach song end?
	jb	@@noend

	mov	[position],0		; restart song
	inc	[loopCnt]		; increase song loop conter

@@noend:
	mov	[pbFlag],0		; clear pattern break flag
	call	mtmUpdBars		; update volume bars

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
	mov	eax,3579545		; eax = NTSC clock constant
	cdq				; NTSC clock constant / period
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
;*	Multitracker command processing:
;\***************************************************************************/


; Command F - Set Speed

PROC	SetSpeed	NEAR

	test	al,al			; skip if zero speed
	jz	@@ok

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
	mov	[di+mtmChannel.volume],al
	call	SetVol

	; pass possible error code on
	ret
ENDP




; Command A - Volume Slide

PROC	VolumeSlide	NEAR

	mov	bl,[di+mtmChannel.volume]	; bl = current volume

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
	or	[di+mtmChannel.status],1
	mov	[di+mtmChannel.volume],bl
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

	add	[di+mtmChannel.period],ax
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
	sub	[di+mtmChannel.period],ax
	call	CheckLimits
	call	SetPeriod
	ret
ENDP



; make sure channel period is within limits

PROC	CheckLimits	NEAR

	cmp	[di+mtmChannel.period],45
	jge	@@ok1
	mov	[di+mtmChannel.period],45
@@ok1:
	cmp	[di+mtmChannel.period],1712
	jle	@@ok2
	mov	[di+mtmChannel.period],1712
@@ok2:
	ret
ENDP




; Command 3 - Tone Portamento

PROC	TonePortamento NEAR

	test	ax,ax			; is infobyte 0?
	jnz	@@1

	movzx	ax,[di+mtmChannel.notepsp]	; if yes, use old speed

@@1:	mov	[di+mtmChannel.notepsp],al	; store portamento speed
	mov	bx,[di+mtmChannel.toperi]
	test	bx,bx				; portamento destination zero?
	jz	@@setperiod			; if yes, skip

	cmp	[di+mtmChannel.period],bx	; should we slide up?
	jg	@@up

	; slide down:
	add	[di+mtmChannel.period],ax	; increase period
	cmp	[di+mtmChannel.period],bx	; past portamento dest?
	jl	@@setperiod
	mov	[di+mtmChannel.period],bx	; if yes, set to porta dest
	mov	[di+mtmChannel.toperi],0	; do not slide anymore
	jmp	@@setperiod

@@up:
	; slide up:
	sub	[di+mtmChannel.period],ax	; decrease period
	cmp	[di+mtmChannel.period],bx	; past portamento dest?
	jg	@@setperiod
	mov	[di+mtmChannel.period],bx	; if yes, set to porta dest
	mov	[di+mtmChannel.toperi],0	; do not slide anymore

@@setperiod:
	call	SetPeriod
	ret
ENDP



; Set period on channel to Sound Device

PROC	SetPeriod	NEAR

	movzx	ebx,[di+mtmChannel.period]

	; Set Sound Device period:
	call	SetSDPeriod

	ret
ENDP



; Command 4 - Vibrato

PROC	Vibrato 	NEAR

	test	al,0Fh			; is new vibrato depth non-zero?
	jnz	@@1

	mov	bl,[di+mtmChannel.vibcmd]	; bl = old vibrato infobyte
	and	bl,0Fh		       ; no, set old vibrato depth
	or	al,bl

@@1:
	test	al,0F0h 		; is new vibrato speed non-zero?
	jnz	@@2

	mov	bl,[di+mtmChannel.vibcmd]	; bl = old vibrato infobyte
	and	bl,0F0h 		 ; no, set old vibrato speed
	or	al,bl

@@2:
	mov	[di+mtmChannel.vibcmd],al	; store new vibrato infobyte

	mov	bl,[di+mtmChannel.vibpos]	; bx = vibrato table position
	and	bx,1Fh
	xor	eax,eax
	mov	al,[vibratoTable+bx]	; eax = vibrato value
	mov	cl,[di+mtmChannel.vibcmd]
	and	cl,0Fh			; multiply with depth
	mul	cl
	shr	ax,7			; divide with 128

	movzx	ebx,[di+mtmChannel.period]	; ebx = channel base period

	test	[di+mtmChannel.vibpos],32      ; is vibrato position >= 32?
	jnz	@@vibneg

	; vibrato position < 32 - positive
	add	ebx,eax
	jmp	@@setperiod

@@vibneg:
	; vibrato position >= 32 - negative
	sub	ebx,eax

@@setperiod:
	mov	al,[di+mtmChannel.vibcmd]	; add vibrato speed  to
	shr	al,4
	add	[di+mtmChannel.vibpos],al	; vibrato position

	; Set period to Sound Device:
	call	SetSDPeriod

	ret
ENDP




; Command 0 - Arpeggio

PROC	Arpeggio	NEAR

	test	ax,ax			; is infobyte zero?
	jz	@@ok			; skip if is

	mov	dx,ax			; save infobyte

	mov	bx,[di+mtmChannel.snote]    ; bx = base index to period table

	movzx	ax,[playCount]
	mov	cl,3			; divide player counter with 3
	div	cl
	test	ah,ah			; is mtmulus zero?
	jz	@@a0			; if yes, use base note
	dec	ah			; is mtmulus one?
	jz	@@a1			; if yes, use infobyte upper nybble

	; modulus is 2 - use infobyte lower nybble
	and	dx,0Fh
	add	dx,dx			; add infobyte lower nybble to note
	add	bx,dx
	jmp	@@a0

@@a1:
	; modulus is 1 - use infobyte upper nybble
	shr	dx,4
	add	dx,dx			; add infobyte upper nybble to note
	add	bx,dx

@@a0:
	movzx	ebx,[Periods+bx]	; get period value

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

	mov	[di+mtmChannel.retrigc],1

	xor	ax,ax
	ret
ENDP



PROC	RetrigNote NEAR

	cmp	[playCount],0		; playing song data?
	jne	@@noset

	call	SetRetrigNote		; yes, just set count
	jmp	@@done

@@noset:
	and	al,0Fh			; should note be retrigged?
	cmp	[di+mtmChannel.retrigc],al	; (retrig count = infobyte)
	jb	@@no

	or	[di+mtmChannel.status],3	; note triggered
	mov	[di+mtmChannel.retrigc],1	; reset retrig count

	mov	ax,[chan]		; ax = Sound Device channel number
	add	ax,[firstSDChan]

	; Start from beginning of sample:
	push	gs
	call	[gs:si+SoundDevice.SetPosition], ax, 0
	pop	gs
	jmp	@@done

@@no:	inc	[di+mtmChannel.retrigc]
	xor	ax,ax

@@done:
	ret
ENDP




; Command 7 - Tremolo

PROC	Tremolo 	NEAR

	test	al,0Fh			; is new tremolo depth non-zero?
	jnz	@@1

	mov	bl,[di+mtmChannel.trecmd]	; bl = old tremolo infobyte
	and	bl,0Fh		       ; no, set old tremolo depth
	or	al,bl

@@1:
	test	al,0F0h 		; is new tremolo speed non-zero?
	jnz	@@2

	mov	bl,[di+mtmChannel.trecmd]	; bl = old tremolo infobyte
	and	bl,0F0h 		 ; no, set old tremolo speed
	or	al,bl

@@2:
	mov	[di+mtmChannel.trecmd],al	; store new tremolo infobyte

	mov	bl,[di+mtmChannel.trepos]	; bx = tremolo table position
	and	bx,1Fh
	xor	eax,eax
	mov	al,[vibratoTable+bx]	; eax = tremolo value
	mov	cl,[di+mtmChannel.trecmd]
	and	cl,0Fh			; multiply with depth
	mul	cl
	shr	ax,7			; divide with 128


	movzx	bx,[di+mtmChannel.volume]	; bx = channel base volume

	test	[di+mtmChannel.trepos],32      ; is position >= 32 ?
	jnz	@@neg

	; Position < 32 - positive
	add	bx,ax
	jmp	@@setvol

@@neg:
	; Position >= 32 - negative
	sub	bx,ax

@@setvol:
	mov	al,[di+mtmChannel.trecmd]
	shr	al,4				; add tremolo speed to
	add	[di+mtmChannel.trepos],al	; vibrato position

	or	[di+mtmChannel.status],1

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
	movzx	dx,[di+mtmChannel.note]
	test	dx,dx			; is there a note?
	jz	@@nonote

       ; ax = finetune

	mov	bl,126				; Period table for one finetune
	mul	bl				; value takes 126 bytes.

	dec	dx
	add	dx,dx
	mov	bx,ax				; new note
	add	bx,dx

	mov	[di+mtmChannel.snote],bx	; store period table offset
	mov	bx,[Periods+bx] 	; bx = period number for this note
	mov	[di+mtmChannel.period],bx	; save period

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

	cmp	al,0			; if infobyte is zero, set loop
	je	@@setloop		; starting row

	cmp	[loopFlag],0		; already looping?
	je	@@setcount

	dec	[loopCount]		; yes, just decrease loop counter
	jnz	@@loop			; counter zero?
	mov	[loopFlag],0		; yes, do not loop anymore
	jmp	@@ok

@@loop:
	mov	ax,[loopRow]
	dec	ax			; loop to saved row
	mov	[row],ax
	mov	[skipFlag],1		; skip to new row next time
	jmp	@@ok

@@setcount:
	; start looping - set loop counter
	mov	[loopCount],al		; loop counter = infobyte
	mov	ax,[loopRow]
	dec	ax			; loop to saved row
	mov	[row],ax
	mov	[loopFlag],1		; looping
	mov	[skipFlag],1		; skip to new row next time
	jmp	@@ok

@@setloop:
	mov	ax,[row]		; save current row as loop destination
	mov	[loopRow],ax
@@ok:
	xor	ax,ax
	ret
ENDP




;/***************************************************************************\
;*
;* Function:	mtmSkipRows
;*
;* Description: Skips to current row in pattern data
;*
;* Returns:	MIDAS error code in ax
;*
;\***************************************************************************/

PROC	mtmSkipRows	NEAR

	mov	di,offset channels	; point ds:di to channel structures
	mov	cx,[numChans]
	mov	ax,[row]
	imul	ax,ax,3 		; Bytes to skip from beginning
	add	ax,2			; skip pattern length word
@@chanloop:
	mov	[di+mtmChannel.playoff],ax ; Set new playing position
	add	di,size mtmChannel
	loop	@@chanloop

	mov	[skipFlag],0		; do not skip data now
	xor	ax,ax
	ret
ENDP



; Command EA - Fine Volume Slide Up

PROC	FineVolumeSlideUp NEAR

	cmp	[playCount],0		; do only when playing song data
	jne	@@ok

	add	[di+mtmChannel.volume],al	; add infobyte to volume
	cmp	[di+mtmChannel.volume],64	; make sure volume is within
	jle	@@set				; limits (<= 64)
	mov	[di+mtmChannel.volume],64

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

	sub	[di+mtmChannel.volume],al	; substract infobyte from vol
	cmp	[di+mtmChannel.volume],0	; make sure volume is positive
	jge	@@set
	mov	[di+mtmChannel.volume],0

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
	mov	[di+mtmChannel.volume],0
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

	movzx	ebx,[di+mtmChannel.period]	; ebx = period
	test	bx,bx				; skip if zero period
	jz	@@ok

	or	[di+mtmChannel.status],3	; note set

	movzx	ebx,bx
	mov	eax,3579545		; eax = NTSC clock constant
	cdq				; NTSC clock constant / period
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

	or	[di+mtmChannel.status],1	; volume set
	mov	al,[di+mtmChannel.volume]	; al = channel volume
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
	DW	offset SlideUp
	DW	offset SlideDown
	DW	offset TonePortamento
	DW	offset Vibrato
	DW	offset TPortVSlide
	DW	offset VibratoVSlide
	DW	offset Tremolo
	DW	offset SetPanning
	DW	offset DoNothing
	DW	offset VolumeSlide
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

	; Multitracker extended E-commands:
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
;* Function:	int mtmSetPosition(ushort pos)
;*
;* Description: Jumps to a specified position in module
;*
;* Input:	ushort	pos		Position to jump to
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	mtmSetPosition	FAR	pos : word
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
	; clear playing offset and compression info from all channels:

	mov	cx,[numChans]
	mov	di,offset channels
@@chanloop:
	mov	[di+mtmChannel.playoff],2	; skip pattern length word
	mov	[di+mtmChannel.comp],0
	add	di,size mtmChannel
	loop	@@chanloop

	mov	[pbFlag],0		; clear pattern break and loop
	mov	[loopFlag],0		; flags

	xor	ax,ax			; success

	ret
ENDP




;/***************************************************************************\
;*
;* Function:	int mtmGetInformation(mpInformation *info);
;*
;* Description: Fills the Module Player information structure
;*
;* Input:	mpInformation *info	information structure to be filled
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	mtmGetInformation	FAR	info : dword
USES	si,di,ds

	les	si,[mtmInfo]		; point es:si to information structure
	mov	di,offset channels	; point ds:di to channel structures

	mov	ax,[setFrame]
	mov	[es:si+mpInformation.setFrame],ax      ; copy set-frame flag
	mov	[setFrame],0			; set set-frame flag to 0

	mov	ax,[rows]
	mov	[es:si+mpInformation.row],ax
	mov	ax,[poss]			; copy saved row, position and
	mov	[es:si+mpInformation.pos],ax	       ; pattern numbers
	mov	ax,[pats]
	mov	[es:si+mpInformation.pattern],ax

	movzx	ax,[speed]
	mov	[es:si+mpInformation.speed],ax	       ; copy speed and tempo values
	movzx	ax,[tempo]
	mov	[es:si+mpInformation.BPM],ax

	movzx	ax,[loopCnt]			; copy song loop counter
	mov	[es:si+mpInformation.loopCnt],ax

	mov	eax,[mtmChanInfo]		; copy channel info pointer
	mov	[es:si+mpInformation.chans],eax

	mov	ax,[numChans]			; copy number of channels
	mov	[es:si+mpInformation.numChannels],ax

	mov	cx,[numChans]
	les	si,[mtmChanInfo]		; point es:si to chan. infos

@@chanloop:
	mov	[es:si+mpChanInfo.flags],0	; clear channel info flags

	mov	ax,[di+mtmChannel.snote]	; ax = current period table
						; index
	mov	bl,128
	div	bl				; convert ax to note number
	mov	al,ah
	xor	ah,ah
	shr	ax,1
	inc	ax				; Starts from D-0...

	mov	bl,12				; divide note number with 12
	div	bl				; al = octave, ah = note

	shl	al,4				; set octave to upper nybble
	or	al,ah				; and note to lower

	mov	[es:si+mpChanInfo.note],al	; store note and octave

	or	[es:si+mpChanInfo.flags],32


	mov	al,[di+mtmChannel.sample]		; copy current
	mov	[es:si+mpChanInfo.instrument],al	; instrument number
	or	[es:si+mpChanInfo.flags],32

	mov	al,[di+mtmChannel.info] 		; copy command
	mov	[es:si+mpChanInfo.infobyte],al		; infobyte

	mov	al,[di+mtmChannel.volume]		; copy volume
	mov	[es:si+mpChanInfo.volume],al

	mov	al,[di+mtmChannel.volbar]		; copy volume bar
	mul	[masterVolume]
	shr	ax,6
	mov	[es:si+mpChanInfo.volumebar],al

	mov	al,[di+mtmChannel.cmd]
	and	al,0Fh				; if command number is
	jnz	@@cmd				; non-zero, or infobyte is
	cmp	[di+mtmChannel.info],0		; non-zero, there is a command
	jne	@@cmd

	; no command - point commandname to empty string:
	mov	[es:si+mpChanInfo.command],0
@@ncmd: mov	[word es:si+mpChanInfo.commandname],offset strNoCmd
	mov	[word es:si+2+mpChanInfo.commandname],seg strNoCmd
	jmp	@@cmdok

@@cmd:
	or	[es:si+mpChanInfo.flags],128	; there is a command

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
	add	di,SIZE mtmChannel
	loop	@@chanloop

	les	di,[info]
	mov	eax,[mtmInfo]		; store information structure pointer
	mov	[es:di],eax		; in *info

	xor	ax,ax			; success
	ret
ENDP



;/***************************************************************************\
;*
;* Function:	int mtmSetMasterVolume(uchar volume)
;*
;* Description: Sets the module player master volume
;*
;* Input:	uchar  volume		  New master volume
;*
;* Returns:	MIDAS error code
;*
;\***************************************************************************/

PROC	mtmSetMasterVolume  FAR     vol : word

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
;* Function:	mtmSave
;*
;* Description: Saves row, position and pattern values for GetInformation()
;*
;\***************************************************************************/

PROC	mtmSave 	NEAR
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
;* Function:	mtmUpdBars
;*
;* Description: Updates "fake" volume bars
;*
;\***************************************************************************/

PROC	mtmUpdBars	NEAR
USES	di

	mov	di,offset channels	; point ds:di to channel structures
	mov	cx,[numChans]

@@chanloop:
	cmp	[di+mtmChannel.volbar],0	; is volume bar zero?
	je	@@1
	dec	[di+mtmChannel.volbar]		; if not, decrement it
@@1:
	test	[di+mtmChannel.status],1	; has volume been changed?
	jz	@@nochange
	mov	al,[di+mtmChannel.volume]
	test	[di+mtmChannel.status],2	; force new volume?
	jnz	@@force
	cmp	[di+mtmChannel.volbar],al	; do not force volume
	jbe	@@nochange			; is bar above volume level?
@@force:
	mov	[di+mtmChannel.volbar],al	; set new volume

@@nochange:
	and	[di+mtmChannel.status],not 3	; clear volume change bits
	add	di,SIZE mtmChannel		; next channel
	loop	@@chanloop

	xor	ax,ax
	ret
ENDP




PROC	EmptyFunct	FAR

	xor	ax,ax
	ret
ENDP




END


; MTM track conversion code: (removed)

; MTM-Data coming

IF 0

@@mtm:	movzx	ax,[es:si]
	shr	ax,2
	mov	[note],ax

	mov	ax,[es:si]
	xchg	ah,al
	and	ax,03f0h
	shr	ax,4
	mov	[inst],ax

	mov	ax,[es:si+1]
	xchg	ah,al
	and	ax,0fffh
	mov	[cmd],ax

	add	si,3
	jmp	@@back
ENDIF

