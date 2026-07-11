/*      WinWave.c
 *
 * Windows Wave Sound Device
 *
 * $Id: winwave.c,v 1.19.2.3 1997/09/06 17:18:29 pekangas Exp $
 *
 * Copyright 1996,1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

/*#define DEBUGMESSAGES*/

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#include "mglobals.h"
#include "sdevice.h"
#include "mmem.h"
#include "dsm.h"

RCSID(char const *winwave_rcsid = "$Id: winwave.c,v 1.19.2.3 1997/09/06 17:18:29 pekangas Exp $";)

//#define DUMPBUFFER

#ifdef DEBUGMESSAGES
#  define DEBUGMSG(x) x
#else
#  define DEBUGMSG(x)
#endif    

#define WINWVERSION 1.11
#define WINWVERSTR "1.11"


/* Maximum number of buffer blocks: (FIXME) */
#define MAXBUFBLOCKS 64


/* Sound Device information */

    /* Sound Card names: */
static char     *winwCardName = "Windows Wave output";


/* Critical section object to prevent thread synchronization problems: */
static CRITICAL_SECTION deviceUsed;

/* Sound Device internal static variables */
static unsigned mixRate, outputMode, mixMode;
static unsigned bufferLen, numBlocks, blockLen;
static unsigned mixElemSize;

static unsigned updateMix;              /* number of elements to mix between
                                           two updates */
static unsigned mixLeft;                /* number of elements to mix before
                                           next update */


static HWAVEOUT waveHandle;
static HANDLE   blockHandles[MAXBUFBLOCKS];
static uchar    *blocks[MAXBUFBLOCKS];
static HANDLE   blockHeaderHandles[MAXBUFBLOCKS];
static WAVEHDR  *blockHeaders[MAXBUFBLOCKS];
static int      blockPrepared[MAXBUFBLOCKS];

static unsigned blockNum;               /* current mixing block number */
static unsigned blockPos;               /* mixing position inside block */

static int      suspended;              /* is playback suspended? */

static unsigned blockCount;             /* current playback block count */

#ifdef DUMPBUFFER
static FILE     *buff;
#endif


/****************************************************************************\
*       enum winwFunctIDs
*       -----------------
* Description:  ID numbers for Windows Wave Sound Device functions
\****************************************************************************/

enum winwFunctIDs
{
    ID_winwDetect = ID_winw,
    ID_winwInit,
    ID_winwClose,
    ID_winwGetMode,
    ID_winwOpenChannels,
    ID_winwSetAmplification,
    ID_winwGetAmplification,
    ID_winwSetUpdRate,
    ID_winwStartPlay,
    ID_winwPlay,
    ID_winwInitDevice,
    ID_winwCloseDevice,
    ID_winwSuspend,
    ID_winwResume
};



/* Local prototypes: */
int CALLING winwSetUpdRate(unsigned updRate);





/****************************************************************************\
*
* Function:     static int winwError(MMRESULT error)
*
* Description:  Converts a Windows multimedia system error code to MIDAS
*               error code
*
* Input:        MMRESULT error          Windows multimedia system error code
*
* Returns:      MIDAS error code
*
\****************************************************************************/

static int winwError(MMRESULT error)
{
    switch ( error )
    {
        case MMSYSERR_NOERROR:
            return OK;
        case MMSYSERR_ERROR:
            return errUndefined;
        case MMSYSERR_BADDEVICEID:
        case MMSYSERR_INVALHANDLE:
        case MMSYSERR_NOTENABLED:
            return errInvalidDevice;
        case MMSYSERR_ALLOCATED:
        case MMSYSERR_HANDLEBUSY:
            return errDeviceBusy;
        case MMSYSERR_NODRIVER:
            return errDeviceNotAvailable;
        case MMSYSERR_NOMEM:
            return errOutOfMemory;
        case MMSYSERR_NOTSUPPORTED:
            return errUnsupported;
        case MMSYSERR_INVALFLAG:
        case MMSYSERR_INVALPARAM:
            return errInvalidArguments;
        case WAVERR_BADFORMAT:
            return errBadMode;
        case WAVERR_STILLPLAYING:
            return errDeviceBusy;
        case WAVERR_UNPREPARED:
            return errInvalidArguments;
        case WAVERR_SYNC:
            return errInvalidDevice;
    }

    return errUndefined;
}

    /* Error code passing macros for multimedia system errors - similar to
       PASSERROR in errors.h: */

#ifdef DEBUG
    #define PASSWINERR(error_, functID) { error = winwError(error_); \
        errAdd(winwError(error_), functID); return error; }
#else
    #define PASSWINERR(error_, functID) return winwError(error_);
#endif




/****************************************************************************\
*
* Function:     int winwDetect(int *result)
*
* Description:  Detects a Windows Wave Sound Device
*
* Input:        int *result             pointer to detection result
*
* Returns:      MIDAS error code. Detection result (1 if detected, 0 if not)
*               is written to *result.
*
\****************************************************************************/

int CALLING winwDetect(int *result)
{
    /* Check that we have at least one wave output device: */
    if ( waveOutGetNumDevs() < 1 )
        *result = 0;
    else
        *result = 1;

    return OK;
}




/****************************************************************************\
*
* Function:     int winwInitDevice(void)
*
* Description:  Initializes the Win32 wave output device and allocates
*               output blocks.
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int winwInitDevice(void)
{
    unsigned    i;
    MMRESULT    mmError;
    WAVEHDR     *header;
    WAVEFORMATEX format;
#ifdef DEBUG
    int         error;
#endif

    /* Calculate the appropriate number of blocks: (one block is approximately
       15 milliseconds */
    numBlocks = mBufferLength / 15;
    if ( numBlocks > MAXBUFBLOCKS )
        numBlocks = MAXBUFBLOCKS;

    /* Calculate required buffer block length: (must be a multiple of
       16 bytes) */
    blockLen = mixRate * mixElemSize * mBufferLength / 1000 / numBlocks;
    blockLen = (blockLen + 15) & (~15);

    bufferLen = numBlocks * blockLen;

    blockNum = blockPos = 0;

    /* Set up wave output format structure: */
    format.wFormatTag = WAVE_FORMAT_PCM;
    if ( outputMode & sdStereo )
        format.nChannels = 2;
    else
        format.nChannels = 1;
    format.nSamplesPerSec = mixRate;
    format.nAvgBytesPerSec = mixElemSize * mixRate;
    format.nBlockAlign = mixElemSize;
    if ( outputMode & sd16bit )
        format.wBitsPerSample = 16;
    else
        format.wBitsPerSample = 8;
    format.cbSize = 0;

    /* Open wave output device using the format just set up: */
    if ( (mmError = waveOutOpen(&waveHandle, WAVE_MAPPER, &format, 0, 0, 0))
        != 0 )
        PASSWINERR(mmError, ID_winwInitDevice);

    /* Allocate and lock memory for all mixing blocks: */
    for ( i = 0; i < numBlocks; i++ )
    {
        /* Allocate global memory for mixing block: */
        if ( (blockHandles[i] = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
            blockLen)) == NULL )
        {
            ERROR(errOutOfMemory, ID_winwInitDevice);
            return errOutOfMemory;
        }
        /* Lock mixing block memory: */
        if ( (blocks[i] = GlobalLock(blockHandles[i])) == NULL )
        {
            ERROR(errUnableToLock, ID_winwInitDevice);
            return errUnableToLock;
        }
    }

    /* Allocate and lock memory for all mixing block headers: */
    for ( i = 0; i < numBlocks; i++ )
    {
        /* Allocate global memory for mixing block: */
        if ( (blockHeaderHandles[i] = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
            sizeof(WAVEHDR))) == NULL )
        {
            ERROR(errOutOfMemory, ID_winwInitDevice);
            return errOutOfMemory;
        }

        /* Lock mixing block memory: */
        if ( (header = blockHeaders[i] = GlobalLock(blockHeaderHandles[i]))
            == NULL )
        {
            ERROR(errUnableToLock, ID_winwInitDevice);
            return errUnableToLock;
        }

        /* Reset wave header fields: */
        header->lpData = blocks[i];
        header->dwBufferLength = blockLen;
        header->dwFlags = WHDR_DONE;        /* mark the block is done */
        header->dwLoops = 0;

        /* Block header is not prepared: */
        blockPrepared[i] = 0;
    }

    return OK;
}




/****************************************************************************\
*
* Function:     int winwCloseDevice(void)
*
* Description:  Closes the Win32 wave output device and deallocates output
*               buffer blocks
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int winwCloseDevice(void)
{
    MMRESULT    mmError;
    unsigned    i;
    int         allDone;
    unsigned    doneTimeout;
#ifdef DEBUG
    int         error;
#endif

    /* Reset wave output device, stop playback, and mark all blocks done: */
    if ( (mmError = waveOutReset(waveHandle)) != 0 )
        PASSWINERR(mmError, ID_winwCloseDevice);

    /* Make sure all blocks are indeed done: */
    doneTimeout = 50;
    while ( 1 )
    {
        allDone = 1;
        for ( i = 0; i < numBlocks; i++ )
        {
            if ( (blockHeaders[i]->dwFlags & WHDR_DONE) == 0 )
                allDone = 0;
        }
        if ( allDone || (doneTimeout == 0) )
            break;
        doneTimeout--;
        Sleep(20);
    }

    /* Unprepare all mixing blocks: */
    for ( i = 0; i < numBlocks; i++ )
    {
        if ( blockPrepared[i] )
        {
            if ( (mmError = waveOutUnprepareHeader(waveHandle, blockHeaders[i],
                sizeof(WAVEHDR))) != 0 )
                PASSWINERR(mmError, ID_winwCloseDevice);
        }
    }

    /* Close wave output device: */
    if ( (mmError = waveOutClose(waveHandle)) != 0 )
        PASSWINERR(mmError, ID_winwCloseDevice);

    /* Unlock and deallocate all mixing blocks: */
    for ( i = 0; i < numBlocks; i++ )
    {
        GlobalUnlock(blockHandles[i]);
        GlobalFree(blockHandles[i]);
    }

    /* Unlock and deallocate all mixing block headers: */
    for ( i = 0; i < numBlocks; i++ )
    {
        GlobalUnlock(blockHeaderHandles[i]);
        GlobalFree(blockHeaderHandles[i]);
    }

    return OK;
}



/****************************************************************************\
*
* Function:     int winwTryMode(unsigned mode, unsigned rate)
*
* Description:  Checks if a given mode/rate combination is supported
*
* Input:        unsigned mode           output mode to try
*               unsigned rate           rate to try
*
* Returns:      1 if supported, 0 if not
*
\****************************************************************************/

static int winwTryMode(unsigned mode, unsigned rate)
{
    MMRESULT    mmError;
    WAVEFORMATEX format;
    unsigned sampleSize;

    /* Check the sample size for the given mode: */
    if ( mode & sd8bit )
        sampleSize = 1;
    else
        sampleSize = 2;
    if ( mode & sdStereo )
        sampleSize *= 2;

    /* Set up wave output format structure for the mode to try: */
    format.wFormatTag = WAVE_FORMAT_PCM;
    if ( mode & sdStereo )
        format.nChannels = 2;
    else
        format.nChannels = 1;
    format.nSamplesPerSec = rate;
    format.nAvgBytesPerSec = sampleSize * rate;
    format.nBlockAlign = sampleSize;
    if ( outputMode & sd16bit )
        format.wBitsPerSample = 16;
    else
        format.wBitsPerSample = 8;
    format.cbSize = 0;

    /* Check if the given format is supported. If not, return 0. If we
       got some other error, return 1 - the error will be handled in
       initialization phase, and it is no use trying other modes */
    mmError = waveOutOpen(NULL, WAVE_MAPPER, &format, 0, 0, WAVE_FORMAT_QUERY);
    if ( mmError == WAVERR_BADFORMAT )
        return 0;
    return 1;
}




/****************************************************************************\
*
* Function:     void winwFindSupportedMode(void)
*
* Description:  Finds a supported output mode as close as possible to the
*               one requested
*
\****************************************************************************/

static void winwFindSupportedMode(void)
{
    /* Good output rates to try: (WSS supports these, most other cards
       support anything) */
#define NUMRATES 7
    static unsigned rates[NUMRATES] = {
        8000, 11025, 16000, 22050, 32000, 37800, 44100 };

    /* Output modes to try: */
#define NUMMODES 4
    static unsigned modes[NUMMODES] = {
        sd8bit | sdMono, sd8bit | sdStereo, sd16bit | sdMono,
        sd16bit | sdStereo };

    int mode, rate, i, initMode, initRate;

    /* First, try the current settings, hopefully they are OK: */
    if ( winwTryMode(outputMode, mixRate) )
        return;

    /* OK, not so lucky, find the closest matches from the tables above: */
    rate = NUMRATES-1;
    mode = NUMMODES-1;

    for ( i = 0; i < NUMRATES; i++ )
        if ( rates[i] >= mixRate )
            rate = i;
    for ( i = 0; i < NUMMODES; i++ )
        if ( modes[i] == outputMode )
            mode = i;

    /* Then just try all modes and rates from the selected ones downwards,
       until a match is found. We'll favor a better mode over a higher
       rate, although all cards probably support either everything or
       nothing in a given mode: */
    initMode = mode; initRate = rate;
    for ( mode = initMode; mode >= 0; mode-- )
    {
        for ( rate = initRate; rate >= 0; rate-- )
        {
            if ( winwTryMode(modes[mode], rates[rate]) )
            {
                /* phew */
                outputMode = modes[mode];
                mixRate = rates[rate];
                return;
            }
        }
    }

    /* Oh well, nothing found, just return the default values: */
    return;
}




/****************************************************************************\
*
* Function:     int winwInit(unsigned mixRate, unsigned mode)
*
* Description:  Initializes Windows Wave Sound Device
*
* Input:        unsigned mixRate        mixing rate in Hz
*               unsigned mode           output mode
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING winwInit(unsigned _mixRate, unsigned mode)
{
    int         error;

    mixRate = _mixRate;
    suspended = 0;

    /* Initialize our Device Used -critical section and enter it: */
    InitializeCriticalSection(&deviceUsed);
    EnterCriticalSection(&deviceUsed);

    /* Determine a good output mode to try: */
    if ( mode & sdMono )
        outputMode = sdMono;
    else
        outputMode = sdStereo;
    if ( mode & sd8bit )
        outputMode |= sd8bit;
    else
        outputMode |= sd16bit;

    winwFindSupportedMode();

    /* Now check what we actually got, and initialize mixing mode
       accordingly: */
    switch ( outputMode )
    {
        case sdMono | sd8bit:
            mixMode = dsmMixMono | dsmMix8bit;
            mixElemSize = 1;
            break;

        case sdStereo | sd8bit:
            mixMode = dsmMixStereo | dsmMix8bit;
            mixElemSize = 2;
            break;

        case sdMono | sd16bit:
            mixMode = dsmMixMono | dsmMix16bit;
            mixElemSize = 2;
            break;
            
        case sdStereo | sd16bit:
        default:
            mixMode = dsmMixStereo | dsmMix16bit;
            mixElemSize = 4;
            break;
    }

    if ( mOversampling )
        mixMode |= dsmMixInterpolation;    

    /* Initialize the output device: */
    if ( (error = winwInitDevice()) != OK )
    {
        LeaveCriticalSection(&deviceUsed);
        PASSERROR(ID_winwInit);
    }

    /* Initialize Digital Sound Mixer: */
    if ( (error = dsmInit(mixRate, mixMode)) != OK )
    {
        LeaveCriticalSection(&deviceUsed);
        PASSERROR(ID_winwInit);
    }

    /* Set update rate to 50Hz: */
    if ( (error = winwSetUpdRate(5000)) != OK )
    {
        LeaveCriticalSection(&deviceUsed);
        PASSERROR(ID_winwInit);
    }

#ifdef DUMPBUFFER
    buff = fopen("buffer.raw", "wb");
#endif

    LeaveCriticalSection(&deviceUsed);

    return OK;
}




/****************************************************************************\
*
* Function:     winwClose(void)
*
* Description:  Uninitializes Windows Wave Sound Device
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING winwClose(void)
{
    int         error;

    DEBUGMSG(puts("winwClose: Entering"));

    EnterCriticalSection(&deviceUsed);

#ifdef DUMPBUFFER
    fclose(buff);
#endif

    DEBUGMSG(puts("winwClose: dsmClose"));

    /* Uninitialize Digital Sound Mixer: */
    if ( (error = dsmClose()) != OK )
    {
        LeaveCriticalSection(&deviceUsed);
        PASSERROR(ID_winwClose)
    }

    DEBUGMSG(puts("winwClose: winwCloseDevice"));

    if ( !suspended )
    {
        if ( (error = winwCloseDevice()) != OK )
        {
            LeaveCriticalSection(&deviceUsed);
            PASSERROR(ID_winwClose)
        }
    }

    DEBUGMSG(puts("winwClose: Done."));

    LeaveCriticalSection(&deviceUsed);
    DeleteCriticalSection(&deviceUsed);

    return OK;
}




/****************************************************************************\
*
* Function:     int winwSuspend(void)
*
* Description:  Suspends all sound playback and releases the output device
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING winwSuspend(void)
{
    int         error;

    EnterCriticalSection(&deviceUsed);

    suspended = 1;

    if ( (error = winwCloseDevice()) != OK )
    {
        LeaveCriticalSection(&deviceUsed);
        PASSERROR(ID_winwSuspend)
    }

    LeaveCriticalSection(&deviceUsed);

    return OK;
}




/****************************************************************************\
*
* Function:     int winwResume(void)
*
* Description:  Resumes sound playback after winwSuspend()
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING winwResume(void)
{
    int         error;

    EnterCriticalSection(&deviceUsed);

    if ( (error = winwInitDevice()) != OK )
    {
        LeaveCriticalSection(&deviceUsed);
        PASSERROR(ID_winwResume)
    }

    suspended = 0;

    LeaveCriticalSection(&deviceUsed);

    return OK;
}




/****************************************************************************\
*
* Function:     int winwGetMode(unsigned *mode)
*
* Description:  Reads the current output mode
*
* Input:        unsigned *mode          pointer to output mode
*
* Returns:      MIDAS error code. Output mode is written to *mode.
*
\****************************************************************************/

int CALLING winwGetMode(unsigned *mode)
{
    *mode = outputMode;

    return OK;
}




/****************************************************************************\
*
* Function:     int winwSetUpdRate(unsigned updRate);
*
* Description:  Sets the channel value update rate (depends on song tempo)
*
* Input:        unsigned updRate        update rate in 100*Hz (eg. 50Hz
*                                       becomes 5000).
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING winwSetUpdRate(unsigned updRate)
{
    /* Calculate number of elements to mix between two updates: (even) */
    mixLeft = updateMix = ((unsigned) ((100L * (ulong) mixRate) /
        ((ulong) updRate)) + 1) & 0xFFFFFFFE;

    return OK;
}




/****************************************************************************\
*
* Function:     int winwStartPlay(void)
*
* Description:  Prepares for playing - doesn't actually do anything here...
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING winwStartPlay(void)
{
    blockCount = 0;
    return OK;
}




/****************************************************************************\
*
* Function:     int winwPlay(int *callMP);
*
* Description:  Plays the sound - mixes the correct amount of data with DSM
*               and copies it to wave output buffer with post-processing.
*               Also takes care of sending fully mixed blocks to the wave
*               output device.
*
* Input:        int *callMP             pointer to music player calling flag
*
* Returns:      MIDAS error code. If enough data was mixed for one updating
*               round and music player should be called, 1 is written to
*               *callMP, otherwise 0 is written there. Note that if music
*               player can be called, winwPlay() should be called again
*               with a new check for music playing to ensure the mixing buffer
*               gets filled with new data.
*
\****************************************************************************/

int CALLING winwPlay(int *callMP)
{
    int         error;
    MMRESULT    mmError;
    unsigned    blockLeft, numElems;
    unsigned    dsmBufSize;
    unsigned    oldPos;

    if ( suspended )
    {
        *callMP = 0;
        return OK;
    }

    EnterCriticalSection(&deviceUsed);

    /* Calculate DSM mixing buffer size in elements: (FIXME) */
    dsmBufSize = dsmMixBufferSize;
#ifdef __32__
    dsmBufSize >>= 2;
#else
    dsmBufSize >>= 1;
#endif
    if ( outputMode & sdStereo )
        dsmBufSize >>= 1;

    /* Repeat while we have unused blocks left: */

    while ( blockHeaders[blockNum]->dwFlags & WHDR_DONE )
    {
        /* Mix only a maximum of "numBlocks" blocks between two calls
           to winwStartPlay(). This is done to make sure that MIDAS
           won't eat quite all CPU time when it runs out of CPU. */
        if ( blockCount > numBlocks )
        {
            *callMP = 0;
            LeaveCriticalSection(&deviceUsed);            
            return OK;
        }
        
        /* Check if the block is prepared - if so, unprepare it: */
        if ( blockPrepared[blockNum] )
        {
            if ( (mmError = waveOutUnprepareHeader(waveHandle,
                blockHeaders[blockNum], sizeof(WAVEHDR))) != 0 )
            {
                LeaveCriticalSection(&deviceUsed);
                PASSWINERR(mmError, ID_winwPlay);
            }
            blockPrepared[blockNum] = 0;
        }

        /* Calculate number of bytes of block left: */
        blockLeft = blockLen - blockPos;

        /* Calculate number of mixing elements left: */
        numElems = blockLeft / mixElemSize;

        /* Check that we won't mix more data than there is to the next
           update: */
        if ( numElems > mixLeft )
            numElems = mixLeft;

        /* Decrease number of elements before next update: */
        mixLeft -= numElems;

        /* Mix the data: */
        if ( (error = dsmMixData(numElems, &blocks[blockNum][blockPos]))
             != OK )
        {
            LeaveCriticalSection(&deviceUsed);
            PASSERROR(ID_winwPlay)
        }

        oldPos = blockPos;
        blockPos += numElems * mixElemSize;

#ifdef DUMPBUFFER
        fwrite(&blocks[blockNum][oldPos], numElems * mixElemSize, 1, buff);
#endif

        /* Check if the block is full - if so, write it to the wave output
           device and move to the next one: */
        if ( blockPos >= blockLen )
        {
            blockHeaders[blockNum]->dwFlags = 0;
            /* Reset wave header fields: */
            blockHeaders[blockNum]->lpData = blocks[blockNum];
            blockHeaders[blockNum]->dwBufferLength = blockLen;
            blockHeaders[blockNum]->dwFlags = 0;
            blockHeaders[blockNum]->dwLoops = 0;

            /* Prepare block header: */
            if ( (mmError = waveOutPrepareHeader(waveHandle,
                blockHeaders[blockNum], sizeof(WAVEHDR))) != 0 )
            {
                LeaveCriticalSection(&deviceUsed);
                PASSWINERR(mmError, ID_winwPlay);
            }
            blockPrepared[blockNum] = 1;

            if ( (mmError = waveOutWrite(waveHandle, blockHeaders[blockNum],
                sizeof(WAVEHDR))) != 0 )
            {
                LeaveCriticalSection(&deviceUsed);
                PASSWINERR(mmError, ID_winwPlay);
            }

            //blockNum = (blockNum++) % numBlocks;
            blockPos = 0;
            blockNum++;
            if ( blockNum >= numBlocks )
                blockNum = 0;

            blockCount++;
        }

        /* Check if the music player should be called: */
        if ( mixLeft == 0 )
        {
            mixLeft = updateMix;
            *callMP = 1;
            LeaveCriticalSection(&deviceUsed);
            return OK;
        }
    }

    /* No more data fits to the mixing blocks - just return without
       update: */
    *callMP = 0;

    LeaveCriticalSection(&deviceUsed);

    return OK;
}






    /* WinWave Sound Device structure: */

SoundDevice     WinWave = {
    0,                                  /* tempoPoll = 0 */
    sdUseMixRate | sdUseOutputMode | sdUseDSM,  /* configBits */
    0,                                  /* port */
    0,                                  /* IRQ */
    0,                                  /* DMA */
    1,                                  /* cardType */
    1,                                  /* numCardTypes */
    sdMono | sdStereo | sd8bit | sd16bit,       /* modes */

    "Windows Wave Sound Device " WINWVERSTR,     /* name */
    &winwCardName,                              /* cardNames */
    0,                                          /* numPortAddresses */
    NULL,                                       /* portAddresses */

    &winwDetect,
    &winwInit,
    &winwClose,
#ifdef M_SD_HAVE_SUSPEND
    &winwSuspend,
    &winwResume,
#endif
    &dsmGetMixRate,
    &winwGetMode,
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
    &winwSetUpdRate,
    &winwStartPlay,
    &winwPlay
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
 * $Log: winwave.c,v $
 * Revision 1.19.2.3  1997/09/06 17:18:29  pekangas
 * Fixed a lockup problem
 *
 * Revision 1.19.2.2  1997/08/25 18:58:53  pekangas
 * Fixed a Visual C warning
 *
 * Revision 1.19.2.1  1997/08/22 14:31:23  pekangas
 * Attempted to reduce CPU use when MIDAS runs out of CPU
 *
 * Revision 1.19  1997/07/31 18:31:12  pekangas
 * Removed some unnecessary tests from winwCloseDevice() that were causing
 * false error messages
 *
 * Revision 1.18  1997/07/31 16:17:07  pekangas
 * Fixed 16-bit mono output
 *
 * Revision 1.17  1997/07/31 14:31:19  pekangas
 * Added support for sound cards that do not implement all output modes or
 * rates
 *
 * Revision 1.16  1997/07/31 10:56:57  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.15  1997/07/08 19:16:44  pekangas
 * Added Win32 setup functions, save/restore setup from registry, and
 * fixed WinWave to ignore buffer blocks -setting to be compatible with the
 * new setup.
 *
 * Revision 1.14  1997/06/11 14:37:35  pekangas
 * Major cleanup for new mixing routines
 *
 * Revision 1.13  1997/06/05 20:18:49  pekangas
 * Added preliminary support for interpolating mixing (mono only at the
 * moment)
 *
 * Revision 1.12  1997/05/30 18:26:45  pekangas
 * Fixed to work with new mixing routines
 *
 * Revision 1.11  1997/04/07 21:07:52  pekangas
 * Added the ability to pause/resume streams.
 * Added functions to query the number of stream bytes buffered
 *
 * Revision 1.10  1997/02/19 21:43:45  pekangas
 * Fixed retail build warnings
 *
 * Revision 1.9  1997/02/19 20:43:14  pekangas
 * Added Suspend() and Resume() -functions to Win32 sound devices
 *
 * Revision 1.8  1997/01/16 18:41:59  pekangas
 * Changed copyright messages to Housemarque
 *
 * Revision 1.7  1997/01/16 18:28:40  pekangas
 * Added pointer to dsmSetStreamWritePosition
 *
 * Revision 1.6  1996/07/29 19:33:10  pekangas
 * Added a proper detection function
 *
 * Revision 1.5  1996/07/13 19:56:40  pekangas
 * Eliminated Visual C warnings
 *
 * Revision 1.4  1996/07/08 19:40:32  pekangas
 * Fixed winwClose() calling convention
 *
 * Revision 1.3  1996/05/26 20:57:00  pekangas
 * Added StartStream and EndStream to WinWave Sound Device structure
 *
 * Revision 1.2  1996/05/25 09:32:47  pekangas
 * Changed to use mBufferLength and mBufferBlocks
 *
 * Revision 1.1  1996/05/22 20:49:33  pekangas
 * Initial revision
 *
*/
