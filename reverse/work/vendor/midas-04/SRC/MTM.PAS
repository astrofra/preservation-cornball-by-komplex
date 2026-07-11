{*	MTM.PAS
 *
 * Multitracker Module Player, v1.04
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*}


unit MTM;


interface



{****************************************************************************\
*	struct mtmInstHdr
*	-----------------
* Description:	Multitracker module instrument header
\****************************************************************************}

type
    mtmInstHdr = Record
        iName : array[0..21] of char;
        sLength : longint;
        loopStart : longint;
        loopEnd : longint;
        fineTune : byte;
        volume : byte;
        attributes : word;
    end;



{****************************************************************************\
*	struct mtmHdr
*	-------------
* Description:	Multitracker module file header
\****************************************************************************}

type
    mtmHdr = Record
        sign : array[0..2] of char;
        version : byte;
        sName : array[0..19] of char;
        numTracks : word;
        lastPattern : byte;
        lastOrder : byte;
        lenComment : word;
        numInsts : byte;
        attribute : byte;
        beatsPerTrack : byte;
        amountTracks : byte;
        panPositions : array[0..31] of byte;
        { instruments : array[0..xxx] of mtmInstHdr; }
    end;



{****************************************************************************\
*	struct mtmChannel
*	-----------------
* Description:	Multitracker Module Player internal channel structure
\****************************************************************************}

type
    mtmChannel = Record
        note : byte;
        inst : byte;
        cmd : byte;
        info : byte;
        compr : byte;

        sample : byte;
        volume : byte;

        period : word;
        snote : word;
        loff : byte;
        coff : byte;
        toperi : word;
        notepsp : byte;
        retrigc : byte;

        status : byte;

        vibcmd : byte;
        vibpos : byte;

        trecmd : byte;
        trepos : byte;

        volbar : byte;
        playoff : word;
    end;

    Pinteger = ^integer;
    Pword = ^word;
    Ppointer = ^pointer;


{$IFNDEF NOLOADERS}



{****************************************************************************\
*
* Function:     function mtmLoadModule(fileName : Pchar; SD : pointer;
*                   SaveSampleInfo : pointer; module : Ppointer) : integer;
*
* Description:  Loads a Multitracker module into memory
*
* Input:        char *fileName          name of module file to be loaded
*               SoundDevice *SD         Sound Device which will store the
*                                       samples
*               int (*SaveSampleInfo)() Pointer to sample information saving
*                                       function. Must have identical calling
*                                       convention with vuPrepare. NIL if no
*                                       such function is used.
*               mpModule **module       pointer to variable which will store
*                                       the module pointer.
*
* Returns:      MIDAS error code.
*               Pointer to module structure is stored in *module.
*
* Notes:        The only practical use at this point for SaveSampleInfo() are
*               the real VU-meters. To load a module and add the prepare the
*               VU meter information point SaveSampleInfo to vuPrepare().
*
\****************************************************************************}

function mtmLoadModule(fileName : Pchar; SD : pointer;
    SaveSampleInfo : pointer; module : Ppointer) : integer;




{****************************************************************************\
*
* Function:     mtmFreeModule(module : pointer; SD : pointer) : integer;
*
* Description:	Deallocates a Multitracker module
*
* Input:        module : pointer        pointer to module to be deallocated
*               SD : pointer            Sound Device that has stored the
*                                       samples
*
* Returns:      MIDAS error code
*
\****************************************************************************}

function mtmFreeModule(module : pointer; SD : pointer) : integer;


{$ENDIF}


function mtmIdentify(header : pointer; recognized : Pinteger) : integer;
function mtmInit(SD : pointer) : integer;
function mtmClose : integer;
function mtmPlayModule(module : pointer; firstSDChannel, numSDChannels,
    loopStart, loopEnd : word) : integer;
function mtmStopModule : integer;
function mtmSetUpdRateFunct(setUpdRate : pointer) : integer;
function mtmPlay : integer;
function mtmSetPosition(pos : word) : integer;
function mtmGetInformation(info : Ppointer) : integer;


procedure mpMTM;		      { Multitracker Module Player structure }



implementation


{$IFNDEF __BPREAL__}
{$DEFINE NOEMS}
{$ENDIF}



USES  Errors, mGlobals, mMem, MPlayer, SDevice, mFile, mUtils
{$IFNDEF NOEMS}
    ,EMS
{$ENDIF}
    ;


{$IFNDEF NOLOADERS}

function mtmLoadModule(fileName : Pchar; SD : pointer;
    SaveSampleInfo : pointer; module : Ppointer) : integer; external;
function mtmFreeModule(module : pointer; SD : pointer) : integer; external;
{$L MTMLOAD.OBJ}

{$ENDIF}


function mtmIdentify(header : pointer; recognized : Pinteger) : integer;
    external;
function mtmInit(SD : pointer) : integer; external;
function mtmClose : integer; external;
function mtmPlayModule(module : pointer; firstSDChannel, numSDChannels,
    loopStart, loopEnd : word) : integer; external;
function mtmStopModule : integer; external;
function mtmSetUpdRateFunct(setUpdRate : pointer) : integer; external;
function mtmPlay : integer; external;
function mtmSetPosition(pos : word) : integer; external;
function mtmGetInformation(info : Ppointer) : integer; external;
procedure mpMTM; external;
{$L MTM.OBJ}


END.
