{*      MODP.PAS
 *
 * ProTracker Module Player, v1.36
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*}


unit MODP;


interface



{****************************************************************************\
*       struct modChannel
*       -----------------
* Description:  Protracker module player channel data structure
\****************************************************************************}

type
    modChannel = Record
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

        volBar : byte;

        loopPos : byte;
        loopCnt : byte;

        playoff : word;
    end;



    Pinteger = ^integer;
    Pword = ^word;
    Ppointer = ^pointer;


{$IFNDEF NOLOADERS}


{****************************************************************************\
*
* Function:     function modLoadModule(fileName : Pchar; SD : pointer;
*                   SaveSampleInfo : pointer; module : Ppointer) : integer;
*
* Description:  Loads a Protracker module into memory
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

function modLoadModule(fileName : Pchar; SD : pointer;
    SaveSampleInfo : pointer; module : Ppointer) : integer;




{****************************************************************************\
*
* Function:     modFreeModule(module : pointer; SD : pointer) : integer;
*
* Description:  Deallocates a Protracker module
*
* Input:        module : pointer        pointer to module to be deallocated
*               SD : pointer            Sound Device that has stored the
*                                       samples
*
* Returns:      MIDAS error code
*
\****************************************************************************}

function modFreeModule(module : pointer; SD : pointer) : integer;

{$ENDIF}



function modConvertSample(sample : pointer; length : word) : integer;
function modConvertPattern(track : pointer; ttype : word; trackLen : Pword)
    : integer;

function modIdentify(header : pointer; recognized : Pinteger) : integer;
function modInit(SD : pointer) : integer;
function modClose : integer;
function modPlayModule(module : pointer; firstSDChannel, numSDChannels,
    loopStart, loopEnd : word) : integer;
function modStopModule : integer;
function modSetUpdRateFunct(setUpdRate : pointer) : integer;
function modPlay : integer;
function modSetPosition(pos : word) : integer;
function modGetInformation(info : Ppointer) : integer;


procedure mpMOD;                        { Protracker Module Player structure }



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

function modLoadModule(fileName : Pchar; SD : pointer;
    SaveSampleInfo : pointer; module : Ppointer) : integer; external;
function modFreeModule(module : pointer; SD : pointer) : integer; external;
{$L MODLOAD.OBJ}

{$ENDIF}


function modConvertSample(sample : pointer; length : word) : integer;
    external;
function modConvertPattern(track : pointer; ttype : word; trackLen : Pword)
    : integer; external;

function modIdentify(header : pointer; recognized : Pinteger) : integer;
    external;
function modInit(SD : pointer) : integer; external;
function modClose : integer; external;
function modPlayModule(module : pointer; firstSDChannel, numSDChannels,
    loopStart, loopEnd : word) : integer; external;
function modStopModule : integer; external;
function modSetUpdRateFunct(setUpdRate : pointer) : integer; external;
function modPlay : integer; external;
function modSetPosition(pos : word) : integer; external;
function modGetInformation(info : Ppointer) : integer; external;
procedure mpMOD; external;
{$L MOD.OBJ}


END.
