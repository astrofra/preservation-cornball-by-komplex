/*      dsmnsnd.c
 *
 * Mixing No Sound Sound Device
 *
 * $Id: dsmnsnd.c,v 1.9.2.1 1997/08/21 22:32:49 pekangas Exp $
 *
 * Copyright 1996,1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

/* None of these functions can fail and practically none of them does
   anything. */

#include "lang.h"

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __DJGPP__
#include <go32.h>
#include <sys/farptr.h>
#endif

#ifdef __UNIX__
#include <sys/time.h>
#include <unistd.h>
#endif

#include "mtypes.h"
#include "errors.h"
#include "sdevice.h"
#include "dsm.h"

RCSID(const char *dsmnsnd_rcsid = "$Id: dsmnsnd.c,v 1.9.2.1 1997/08/21 22:32:49 pekangas Exp $";)

#define MIXRATE 4000
#define MIXMODE dsmMixMono | dsmMix16bit
#define OUTPUTMODE sd16bit | sdMono
#define MAXTIME 2000
#define MINTIME 10

#ifndef __WIN32__
typedef U32 DWORD;
#endif


    /* A lot of functions do not use their arguments: */
//#ifdef __WATCOMC__
//#pragma off (unreferenced)
//#endif




/* Names and stuff: */
static char     *nsndCard = "No Sound";


/* Variables: */
static DWORD    prevPlayTime;
static DWORD    playElemsLeft;
static unsigned nsndMasterVolume;
static unsigned updateMix;              /* number of elements to mix between
                                           two updates */
static unsigned mixLeft;                /* number of elements to mix before
                                           next update */
static int      suspended;


/* Local prototypes: */
int CALLING nsndSetUpdRate(unsigned updRate);


enum nsndFunctIDs
{
    ID_nsndDetect = ID_nsnd,
    ID_nsndInit,
    ID_nsndClose,
    ID_nsndGetMixRate,
    ID_nsndGetMode,
    ID_nsndOpenChans,
    ID_nsndCloseChans,
    ID_nsndClearChans,
    ID_nsndMute,
    ID_nsndPause,
    ID_nsndSetMaster,
    ID_nsndPlaySound,
    ID_nsndStopSound,
    ID_nsndSetRate,
    ID_nsndGetRate,
    ID_nsndSetVol,
    ID_nsndSetInst,
    ID_nsndSetPos,
    ID_nsndGetPos,
    ID_nsndSetPanning,
    ID_nsndGetPanning,
    ID_nsndMuteChannel,
    ID_nsndAddInst,
    ID_nsndRemInst,
    ID_nsndSetUpdRate,
    ID_nsndPlay,
    ID_nsndGetMasterVolume,
    ID_nsndGetVolume,
    ID_nsndGetInstrument,
    ID_nsndStartPlay,
    ID_nsndSetAmplification,
    ID_nsndGetAmplification
};


static DWORD GetTimeMS(void)
{
#ifdef __WIN32__
    return GetTickCount();
#endif
#ifdef __DOS__
#  ifdef __DJGPP__
    return _farpeekl(_dos_ds, 0x46c) * (1000 / 18.2);
#  else
    return (*((U32*) 0x46c)) * (1000 / 18.2);
#  endif
#endif
#ifdef __UNIX__
    struct timeval t;

    gettimeofday(&t, NULL);
    return (t.tv_usec / 1000) + (t.tv_sec * 1000);
#endif
}



static int CALLING nsndDetect(int *result)
{
    /* No Sound is always there */
    *result = 1;
    return OK;
}


static int CALLING nsndInit(unsigned mixRate, unsigned mode)
{
    int         error;

    suspended = 0;

    /* Initialize DSM - we'll be mixing the data: */
    if ( (error = dsmInit(MIXRATE, MIXMODE)) != OK )
        PASSERROR(ID_nsndInit);

    /* Set update rate to 50Hz: */
    if ( (error = nsndSetUpdRate(5000)) != OK )
        PASSERROR(ID_nsndInit);

    /* Set real DSM master volume to 0 to speed up mixing: */
    if ( (error = dsmSetMasterVolume(0)) != OK )
        PASSERROR(ID_nsndInit);

    /* Initialize "previous" playing tick count: */
    prevPlayTime = GetTimeMS();

    return OK;
}


static int CALLING nsndClose(void)
{
    int         error;

    /* Just uninitialize DSM: */
    if ( (error = dsmClose()) != OK )
        PASSERROR(ID_nsndClose);

    return OK;
}


#ifdef M_SD_HAVE_SUSPEND

static int CALLING nsndSuspend(void)
{
    suspended = 1;

    return OK;
}


static int CALLING nsndResume(void)
{
    suspended = 0;

    return OK;
}

#endif

int CALLING nsndGetMode(unsigned *mode)
{
    /* Return our default mode: */
    *mode = OUTPUTMODE;
    return OK;
}


int CALLING nsndSetMasterVolume(unsigned masterVolume)
{
    /* Just store the master volume, don't set it to DSM: */
    nsndMasterVolume = masterVolume;

    return OK;
}


int CALLING nsndGetMasterVolume(unsigned *masterVolume)
{
    /* Return our stored master vol: */
    *masterVolume = nsndMasterVolume;

    return OK;
}


int CALLING nsndSetUpdRate(unsigned updRate)
{
    /* Calculate number of elements to mix between two updates: (even) */
    mixLeft = updateMix = ((unsigned) ((100L * (ulong) MIXRATE) /
        ((ulong) updRate)) + 1) & 0xFFFFFFFE;

    return OK;
}


int CALLING nsndStartPlay(void)
{
    DWORD       time, timeDiff;;

    /* Get the time in milliseconds since last nsndStartPlay(): */
    time = GetTimeMS();
    timeDiff = time - prevPlayTime;
    prevPlayTime = time;

    /* Restrict the time difference to reasonable values: */
    if ( timeDiff > MAXTIME )
        timeDiff = MAXTIME;
    if ( timeDiff < MINTIME )
        timeDiff = 0;

    /* Calculate the number of elements that fits into this time - it is the
       amount we'll mix this polling time: */
    playElemsLeft = (timeDiff * MIXRATE) / 1000;
    
    return OK;
}


int CALLING nsndPlay(int *callMP)
{
    int         error;
    unsigned    dsmBufSize;
    unsigned    numElems, doNow;

    if ( (!playElemsLeft) || (suspended) )
    {
        *callMP = 0;
        return OK;
    }

#ifdef __32__
    dsmBufSize = dsmMixBufferSize >> 2;
#else
    dsmBufSize = dsmMixBufferSize >> 1;
#endif

    if ( playElemsLeft > mixLeft )
        numElems = mixLeft;
    else
        numElems = playElemsLeft;
    mixLeft -= numElems;
    playElemsLeft -= numElems;

    while ( numElems )
    {
        if ( numElems > dsmBufSize )
            doNow = dsmBufSize;
        else
            doNow = numElems;
        numElems -= doNow;

        if ( (error = dsmMixData(doNow, dsmMixBuffer)) != OK )
            PASSERROR(ID_nsndPlay)
    }

    /* Check if the music player should be called: */
    if ( mixLeft == 0 )
    {
        mixLeft = updateMix;
        *callMP = 1;
    }
    else
    {
        *callMP = 0;
    }

    return OK;
}


/* And the struct: */

SoundDevice MixNoSound = {
    0,                                  /* Poll to tempo */
    sdUseDSM,
    0,
    0,
    0,
    1,
    1,
    sdMono | sd16bit,
    "Mixing No Sound Sound Device",
    &nsndCard,
    0,
    NULL,
    &nsndDetect,
    &nsndInit,
    &nsndClose,
#ifdef M_SD_HAVE_SUSPEND
    &nsndSuspend,
    &nsndResume,
#endif
    &dsmGetMixRate,
    &nsndGetMode,
    &dsmOpenChannels,
    &dsmCloseChannels,
    &dsmClearChannels,
    &dsmMute,
    &dsmPause,
    &dsmSetMasterVolume,
    &dsmGetMasterVolume,
    &dsmSetAmplification,
    &dsmGetAmplification,
    &dsmPlaySound,
    &dsmReleaseSound,
    &dsmStopSound,
    &dsmSetRate,
    &dsmGetRate,
    &dsmSetVolume,
    &dsmGetVolume,
    &dsmSetSample,
    &dsmGetSample,
    &dsmSetPosition,
    &dsmGetPosition,
    &dsmGetDirection,
    &dsmSetPanning,
    &dsmGetPanning,
    &dsmMuteChannel,
    &dsmAddSample,
    &dsmRemoveSample,
    &nsndSetUpdRate,
    &nsndStartPlay,
    &nsndPlay
#ifdef SUPPORTSTREAMS
    ,
    &dsmStartStream,
    &dsmStopStream,
    &dsmSetLoopCallback,
    &dsmSetStreamWritePosition,
    &dsmPauseStream
#endif
};


/*
 * $Log: dsmnsnd.c,v $
 * Revision 1.9.2.1  1997/08/21 22:32:49  pekangas
 * The Mixing No Sound Sound Device is now used in the Linux version too
 *
 * Revision 1.9  1997/07/31 16:24:43  pekangas
 * Fixed a typo in Win32 support
 *
 * Revision 1.8  1997/07/31 15:53:54  pekangas
 * Fixed to work in DOS
 *
 * Revision 1.7  1997/07/31 10:56:40  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.6  1997/07/28 18:18:34  pekangas
 * Fixed stream support
 *
 * Revision 1.5  1997/06/11 14:37:46  pekangas
 * Fixed dsmInit() arguments
 *
 * Revision 1.4  1997/05/30 18:26:45  pekangas
 * Fixed to work with new mixing routines
 *
 * Revision 1.3  1997/02/19 20:43:14  pekangas
 * Added Suspend() and Resume() -functions to Win32 sound devices
 *
 * Revision 1.2  1997/01/16 18:41:59  pekangas
 * Changed copyright messages to Housemarque
 *
 * Revision 1.1  1996/07/29 19:35:39  pekangas
 * Initial revision
 *
 * Revision 1.1  1996/05/22 20:49:33  pekangas
 * Initial revision
 *
*/
