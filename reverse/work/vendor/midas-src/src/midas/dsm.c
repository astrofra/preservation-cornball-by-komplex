/*      DSM.C
 *
 * Digital Sound Mixer
 *
 * $Id: dsm.c,v 1.41.2.2 1997/09/06 17:16:24 pekangas Exp $
 *
 * Copyright 1996,1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

/* Possibly environment dependent code is marked with *!!* */

#include <assert.h>
#include <string.h>

#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#include "mmem.h"
#include "sdevice.h"
#include "dsm.h"
#include "mutils.h"
#include "mglobals.h"
#include "mulaw.h"

#ifdef __DPMI__
#include "dpmi.h"
#endif

RCSID(const char *dsm_rcsid = "$Id: dsm.c,v 1.41.2.2 1997/09/06 17:16:24 pekangas Exp $";)


    /* Default ramping speed - two volume steps per output sample: */
#define RAMPSPEED 0x20000


#define MIXBUFLEN 40                    /* mixing buffer length 1/40th of a
                                           second */

void *dsmMixBuffer;                     /* DSM mixing buffer. dsmPlay() writes
                                           the mixed data here. Post-
                                           processing is usually necessary. */
unsigned dsmMixBufferSize;              /* DSM mixing buffer size */

/* The following global variables are used internally by different DSM
   functions and should not be accessed by other modules: */

unsigned        dsmMixRate;             /* mixing rate in Hz */
unsigned        dsmMode;                /* output mode (see enum
                                           dsmMixMode) */
#ifdef __16__
unsigned        dsmVolTableSeg;         /* volume table segment */
#endif
int             *dsmVolumeTable;        /* pointer to volume table */
int             *dsmUlawVolumeTable;    /* u-law volume table */
float           *dsmByteFloatTable;     /* sample byte->float table */
float           *dsmUlawFloatTable;     /* sample ulaw->float table */

dsmChannel      *dsmChannels;           /* pointer to channel datas */
dsmSample       *dsmSamples;            /* sample structures */

unsigned        dsmNumChannels;         /* number of open channels */
unsigned        dsmAllocatedChannels;   /* number of allocated channels */

unsigned        dsmAmplification;       /* amplification level */
dsmMixRoutineSet *dsmActMixRoutines;    /* active mixing routine set */
unsigned dsmMasterVolume;               /* master volume */
int      dsmMuted;                      /* 1 if muted, 0 if not */
int      dsmPaused;                     /* 1 if paused, 0 if not */
volatile unsigned dsmChPlay;            /* 1 if data on channels may be
                                           played */
dsmPostProcessor *dsmPostProcList;      /* the post-processor list */
int             dsmAutoFilterType;      /* autodetected filter type */




/* DSM internal variables: */

static int *dsmVolTableMem;             /* pointer to volume table returned
                                           by memAlloc(). Used for
                                           deallocating */
static int *dsmUlawVolTableMem;         /* pointer to u-law volume table
                                           returned by memAlloc(). Used for
                                           deallocating */
static unsigned *dsmMixBufferMem;       /* pointer to mixing buffer returned
                                           by memAlloc(). User for dealloc */
static float *dsmByteFloatTableMem;
static float *dsmUlawFloatTableMem;

static int      mixInteger;
static int      mixInterpolate;
static int      ulawAutoConvert;


static dsmPostProcessor dsmFilters =
    {
        NULL, NULL, /* next, prev */
        NULL,       /* user data */
        &dsmFilterFloatMono,
        &dsmFilterFloatStereo,
        &dsmFilterIntMono,
        &dsmFilterIntStereo
    };



/* Calculate log2(sampleSize) based on sample type */
static int dsmSampleShift(int sampleType)
{
    switch ( sampleType )
    {
        case smp8bitStereo:
        case smpUlawStereo:
        case smp16bitMono:
            return 1;

        case smp16bitStereo:
            return 2;
    }
    return 0;
}


/* Get correct clearing value for sample type */
static uchar dsmSampleClearValue(int sampleType)
{
    switch ( sampleType )
    {
        case smp16bitMono:
        case smp16bitStereo:
            return 0;
    }
    return 128;
}


extern int CALLING mCpuIsPPro(void);


/****************************************************************************\
*
* Function:     int dsmInit(unsigned mixRate, unsigned mode);
*
* Description:  Initializes Digital Sound Mixer
*
* Input:        unsigned mixRate        mixing rate in Hz
*               unsigned mode           mixing mode (see enum dsmMixMode)
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmInit(unsigned mixRate, unsigned mode)
{
    int         error, i;
    static uchar *tempUlaw;
    static short *tempRaw;
    unsigned    n;
    

    if ( !(mode & (dsmMixInteger | dsmMixFloat)) )
    {
        if ( (mPreferredMixingMode == M_MIX_FLOAT) ||
             (mode & dsmMixInterpolation) || (mCpuIsPPro()) )
            mode |= dsmMixFloat;
        else
            mode |= dsmMixInteger;
    }
    if ( !(mode & (dsmMix16bit | dsmMix8bit)) )
        mode |= dsmMix16bit;

    dsmMixRate = mixRate;
    dsmMode = mode;

    if ( mode & dsmMixInteger )
        mixInteger = 1;
    else
        mixInteger = 0;

    /* 16-bit -> u-law -conversion is only beneficial for integer mixing: */
    if ( mixInteger )
        ulawAutoConvert = m16bitUlawAutoConvert;
    else
        ulawAutoConvert = 0;

    dsmNumChannels = 0;                 /* no open channels */
    dsmChPlay = 0;                      /* do not play data in channels */
    dsmChannels = NULL;                 /* channel structures not allocated */
    dsmMuted = 0;                       /* not muted */
    dsmPaused = 0;                      /* not paused */
    dsmMasterVolume = 64;               /* master volume maximum */

    /* Calculate mixing buffer size: */
    if ( mixInteger )
    {
        if ( mode & dsmMixStereo )
            dsmMixBufferSize = 2 * sizeof(unsigned) * mixRate / MIXBUFLEN;
        else
            dsmMixBufferSize = sizeof(unsigned) * mixRate / MIXBUFLEN;
    }
    else
    {
        if ( mode & dsmMixStereo )
            dsmMixBufferSize = 2 * sizeof(float) * mixRate / MIXBUFLEN;
        else
            dsmMixBufferSize = sizeof(float) * mixRate / MIXBUFLEN;
    }
        
    /* Round up mixing buffer size to nearest paragraph: */
    dsmMixBufferSize = (dsmMixBufferSize + 15) & 0xFFFFFFF0;

    if ( mixInteger )
    {
        /* Allocate memory for normal volume table: */
        if ( (error = memAlloc(VOLLEVELS * 256 * sizeof(int) + 1024,
                               (void**) &dsmVolTableMem)) != OK )
            PASSERROR(ID_dsmInit);

        /* Allocate memory for u-law volume table: */
        if ( (error = memAlloc(VOLLEVELS * 256 * sizeof(int) + 1024,
                               (void**) &dsmUlawVolTableMem)) != OK )
            PASSERROR(ID_dsmInit);

        /* Align both volume tables to 1024-byte boundaries: */
        dsmVolumeTable = (int*) ((((unsigned) dsmVolTableMem) + 1023) &
                                 (~1023));
        dsmUlawVolumeTable = (int*) ((((unsigned) dsmUlawVolTableMem)
                                      + 1023) & (~1023));
    }

    /* Allocate the mixing buffer: */
    if ( (error = memAlloc(dsmMixBufferSize + 1024, (void*)&dsmMixBufferMem)) != OK )
        PASSERROR(ID_dsmInit);

    /* Align the mixing buffer to a 1024-byte boundary: */
    dsmMixBuffer = (void*) ((((unsigned) dsmMixBufferMem) + 1023) & (~1023));

    /* Allocate memory for sample structures: */
    if ( (error = memAlloc(MAXSAMPLES * sizeof(dsmSample), (void**)
                           &dsmSamples)) != OK )
        PASSERROR(ID_dsmInit);

    /* Mark all samples unused: */
    for ( i = 0; i < MAXSAMPLES; i++ )
        dsmSamples[i].inUse = 0;

    if ( !mixInteger )
    {
        /* Allocate the sample byte->float conversion table, align it at a
           1024-byte boundary and initialize it: */
        if ( (error = memAlloc(1024 + 256 * sizeof(float), (void**)
                               &dsmByteFloatTableMem)) != OK )
            PASSERROR(ID_dsmInit);

        dsmByteFloatTable = (float*) ((((unsigned) dsmByteFloatTableMem)
                                       + 1023) & (~1023));
        
        for ( i = 0; i < 256; i++ )
            dsmByteFloatTable[i] = (float) (256 * (i-128));

        
        /* Allocate and initialize sample u-law->float conversion table: */
        if ( (error = memAlloc(1024 + 256 * sizeof(float), (void**)
                               &dsmUlawFloatTableMem)) != OK )
            PASSERROR(ID_dsmInit);
        
        dsmUlawFloatTable = (float*) ((((unsigned) dsmUlawFloatTableMem)
                                       + 1023) & (~1023));
        
        if ( (error = memAlloc(256, (void**) &tempUlaw)) != OK )
            PASSERROR(ID_dsmInit);
        if ( (error = memAlloc(256 * sizeof(short), (void**) &tempRaw)) != OK )
            PASSERROR(ID_dsmInit);
        for ( i = 0; i < 256; i++ )
            tempUlaw[i] = i;
        if ( (error = mUlawDecode(tempUlaw, 256, tempRaw)) != OK )
            PASSERROR(ID_dsmInit);
        for ( i = 0; i < 256; i++ )
            dsmUlawFloatTable[i] = (float) tempRaw[i];
        if ( (error = memFree(tempRaw)) != OK )
            PASSERROR(ID_dsmInit);
        if ( (error = memFree(tempUlaw)) != OK )
            PASSERROR(ID_dsmInit);
    }

    /* Pick the correct mixing routines: */
    if ( dsmMode & dsmMixInterpolation )
    {
        mixInterpolate = 1;
        if ( dsmMode & dsmMixMono )      
            dsmActMixRoutines = &dsmMixMonoFloatInterp;
        else
            dsmActMixRoutines = &dsmMixStereoFloatInterp;
    }
    else            
    {
        mixInterpolate = 0;
        if ( dsmMode & dsmMixMono )
        {
            if ( mixInteger )
                dsmActMixRoutines = &dsmMixMonoInteger;
            else
                dsmActMixRoutines = &dsmMixMonoFloat;
        }
        else
        {
            if ( mixInteger )
                dsmActMixRoutines = &dsmMixStereoInteger;
            else
                dsmActMixRoutines = &dsmMixStereoFloat;
        }
    }

    /* Relocate any mixing routines necessary in data area: */
    for ( n = 0; n < sdNumSampleTypes; n++ )
    {
        if ( dsmActMixRoutines->routines[n].mainLoopReloc != NULL )
        {
            if ( (error = dsmRelocMixingRoutine(&dsmActMixRoutines->
                                                routines[n])) != OK )
                PASSERROR(ID_dsmInit);
        }
        if ( dsmActMixRoutines->middleRoutines[n].mainLoopReloc != NULL )
        {
            if ( (error = dsmRelocMixingRoutine(&dsmActMixRoutines->
                                                middleRoutines[n])) != OK )
                PASSERROR(ID_dsmInit);
        }
        if ( dsmActMixRoutines->surroundRoutines[n].mainLoopReloc != NULL )
        {
            if ( (error = dsmRelocMixingRoutine(&dsmActMixRoutines->
                                                surroundRoutines[n])) != OK )
                PASSERROR(ID_dsmInit);
        }
    }

    /* Initialize the DSM post processor list: */
    if ( (error = memAlloc(sizeof(dsmPostProcessor), (void**)
                           &dsmPostProcList)) != OK )
        PASSERROR(ID_dsmInit);
    dsmPostProcList->next = dsmPostProcList->prev = dsmPostProcList;

    /* Autodetect the default filter mode: */
    if ( mixInterpolate )
        dsmAutoFilterType = 0;
    else
        dsmAutoFilterType = 1;

    /* Add the (possible) filters: */
    if ( (error = dsmAddPostProcessor(&dsmFilters, dsmPostProcLast, NULL))
         != OK )
        PASSERROR(ID_dsmInit);

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmClose(void)
*
* Description:  Uninitializes Digital Sound Mixer
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmClose(void)
{
    int         error;

    /* Remove filters from the post-processor list and remove it: */
    if ( (error = dsmRemovePostProcessor(&dsmFilters)) != OK )
        PASSERROR(ID_dsmClose);
    if ( (error = memFree(dsmPostProcList)) != OK )
        PASSERROR(ID_dsmClose);

    if ( !mixInteger )
    {
        if ( (error = memFree(dsmUlawFloatTableMem)) != OK )
            PASSERROR(ID_dsmClose);
        if ( (error = memFree(dsmByteFloatTableMem)) != OK )
            PASSERROR(ID_dsmClose);
    }

    /* Deallocate volume tables: */
    if ( mixInteger )
    {
        if ( (error = memFree(dsmVolTableMem)) != OK )
            PASSERROR(ID_dsmClose);
        if ( (error = memFree(dsmUlawVolTableMem)) != OK )
            PASSERROR(ID_dsmClose);
    }

    /* Deallocate mixing buffer: */
    if ( (error = memFree(dsmMixBufferMem)) != OK )
        PASSERROR(ID_dsmClose);

    /* Deallocate sample structures: */
    if ( (error = memFree(dsmSamples)) != OK )
        PASSERROR(ID_dsmClose);

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmRelocMixingRoutine(dsmMixRoutine *routine)
*
* Description:  Relocates a mixing routine in writable data area
*
* Input:        dsmMixRoutine *routine  the routine
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmRelocMixingRoutine(dsmMixRoutine *routine)
{
    dsmMixLoopRelocInfo *reloc = routine->mainLoopReloc;
    static void *data;
    unsigned i;
    unsigned orgStart, start, offset;
    U32 *relocData;
    int error;

    assert(reloc);
    assert(reloc->numCopyBytes);

    /* Allocate memory for the code and copy it there: */
    if ( (error = memAlloc(reloc->numCopyBytes, &data)) != OK )
        PASSERROR(ID_dsmRelocMixingRoutine);
    mMemCopy(data, (void*) routine->mainMixLoop, reloc->numCopyBytes);

    orgStart = (unsigned) routine->mainMixLoop;
    start = (unsigned) data;

    /* Relocate all absolute addresses in the code: */
    for ( i = 0; i < reloc->numRelocEntries; i++ )
    {
        offset = reloc->relocTable[i] - orgStart;
        relocData = (U32*) (((U8*) data) + offset);
        *relocData = ((*relocData) - orgStart) + start;
    }

    /* Finally, point the mixing loop to the one in data area, and destroy
       all evidence that it ever was elsewhere. This way, if DSM is
       initialized again, the loop won't get copied many times. */
    routine->mainMixLoop = (void (CALLING *)(unsigned, int)) data;
    routine->mainLoopReloc = NULL;

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmGetMixRate(unsigned *mixRate)
*
* Description:  Reads the actual mixing rate
*
* Input:        unsigned *mixRate       pointer to mixing rate variable
*
* Returns:      MIDAS error code.
*               Mixing rate, in Hz, is stored in *mixRate
*
\****************************************************************************/

int CALLING dsmGetMixRate(unsigned *mixRate)
{
    *mixRate = dsmMixRate;
    return OK;
}




/****************************************************************************\
*
* Function:     int dsmOpenChannels(unsigned channels)
*
* Description:  Opens channels for output
*
* Input:        unsigned channels       number of channels to open
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmOpenChannels(unsigned channels)
{
    int         error;

    dsmChPlay = 0;                      /* data on channels may not be
                                           played */

    dsmNumChannels = channels;
    if ( dsmMode & dsmMixInterpolation )
        dsmAllocatedChannels = 2 * dsmNumChannels;
    else
        dsmAllocatedChannels = dsmNumChannels;
    
    dsmNumChannels = channels;
    dsmMuted = 0;                       /* not muted */
    dsmPaused = 0;                      /* not paused */

    /* Allocate memory for channel structures: */
    if ( (error = memAlloc(dsmAllocatedChannels * sizeof(dsmChannel), (void**)
        &dsmChannels)) != OK )
        PASSERROR(ID_dsmOpenChannels);

    
    /* Set default amplification level and calculate volume table: */
    if ( channels < 5 )
    {
        if ( (error = dsmSetAmplification(64)) != OK )
            PASSERROR(ID_dsmOpenChannels);
    }
    else
    {
        if ( (error = dsmSetAmplification(14*channels)) != OK )
            PASSERROR(ID_dsmOpenChannels);
    }
    
    /* Clear all channels: */
    if ( (error = dsmClearChannels()) != OK )
        PASSERROR(ID_dsmOpenChannels);

    dsmChPlay = 1;                      /* data on channels may now be
                                           played */
    return OK;
}




/****************************************************************************\
*
* Function:     int dsmCalcVolTable(unsigned amplification)
*
* Description:  Calculates a new volume table
*
* Input:        unsigned amplification  Amplification level. 64 - normal
*                                       (100%), 32 = 50%, 128 = 200% etc.
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmCalcVolTable(unsigned amplification)
{
    int         error;
    int         volume, value;
    long        temp;
    int         *tablePtr;              /* current volume table position */
    int         *ulawPtr;
    short       *ulawTable;
    uchar       *ulaw;
    unsigned    u;

    assert(mixInteger);

    /* Allocate a temporary u-law table and initialize it: */
    if ( (error = memAlloc(256 * sizeof(short), (void**) &ulawTable)) != OK )
        PASSERROR(ID_dsmCalcVolTable)
    if ( (error = memAlloc(256 * sizeof(uchar), (void**) &ulaw)) != OK )
        PASSERROR(ID_dsmCalcVolTable)
    for ( u = 0; u < 256; u++ )
        ulaw[u] = u;
    if ( (error = mUlawDecode(ulaw, 256, ulawTable)) != OK )
        PASSERROR(ID_dsmCalcVolTable)
    if ( (error = memFree(ulaw)) != OK )
        PASSERROR(ID_dsmCalcVolTable)


    tablePtr = dsmVolumeTable;
    ulawPtr = dsmUlawVolumeTable;

    for ( volume = 0; volume < VOLLEVELS; volume++ )
    {
        for ( value = -128; value < 128; value++ )
        {
            /*!!*/
            temp = (((long) (value * volume)) * ((long) amplification)) / 64L;
            temp = (temp * 256L / ((long)(VOLLEVELS-1)) /
                    ((long) dsmNumChannels));
            *(tablePtr++) = temp;
        }

        for ( value = 0; value < 256; value++ )
        {
            temp = ((((long) ulawTable[value]) * volume) *
                ((long) amplification)) / 64L;
            temp = (temp / ((long)(VOLLEVELS-1)) / ((long) dsmNumChannels));
            *(ulawPtr++) = temp;
        }
    }

    /* Get rid of the u-law table: */
    if ( (error = memFree(ulawTable)) != OK )
        PASSERROR(ID_dsmCalcVolTable)

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmCloseChannels(void)
*
* Description:  Closes open output channels
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmCloseChannels(void)
{
    int         error;

    /* Check that channels have been opened: */
    if ( dsmNumChannels == 0 )
    {
        /* No open channels - return error: */
        ERROR(errNoChannels, ID_dsmCloseChannels);
        return errNoChannels;
    }

    dsmChPlay = 0;                      /* do not play data on channels */

    /* Deallocate channel structures: */
    if ( (error = memFree(dsmChannels)) != OK )
        PASSERROR(ID_dsmCloseChannels)

    dsmNumChannels = 0;

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmClearChannels(void)
*
* Description:  Clears open channels (removes all sounds)
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmClearChannels(void)
{
    unsigned    i;
    dsmChannel  *chan;

    /* Check that channels have been opened: */
    if ( dsmNumChannels == 0 )
    {
        /* No open channels - return error: */
        ERROR(errNoChannels, ID_dsmCloseChannels);
        return errNoChannels;
    }

    /* Remove sounds from channels: */
    for ( i = 0; i < dsmAllocatedChannels; i++ )
    {
        chan = &dsmChannels[i];
        chan->status = dsmChanStopped;  /* playing is stopped */
        chan->sampleHandle = 0;         /* no sample selected */
        chan->sampleChanged = 0;        /* sample not changed */
        chan->sampleType = smpNone;     /* no sample */
        chan->origSampleType = smpNone; /* no sample */
        chan->samplePos = sdSmpNone;    /* no sample */
        chan->rate = 0;                 /* no playing rate set */
        chan->direction = 1;            /* forward direction */
        chan->panning = panMiddle;      /* channel at middle */
        chan->muted = 0;                /* channel not muted */
        chan->LoopCallback = NULL;      /* no loop callback */
        chan->inTranslation = 0;        /* not in translation */
        chan->leftVol = 0;
        chan->rightVol = 0;
        chan->leftVolTarget = 0;
        chan->rightVolTarget = 0;
        chan->leftRampSpeed = 0;
        chan->rightRampSpeed = 0;
        chan->volumeRamping = 0;
    }

    return OK;
}



/****************************************************************************\
*
* Function:     void dsmSetChannelVolume(dsmChannel *chan, unsigned rampSpeed)
*
* Description:  Sets the channel volume and starts volume ramp
*
* Input:        dsmChannel *chan        the channel
*               unsigned rampSpeed      volume ramping speed
*
\****************************************************************************/

static void dsmSetChannelVolume(dsmChannel *chan, unsigned rampSpeed)
{
    unsigned leftVol, rightVol;

    if ( chan->muted || dsmMuted )
    {
        leftVol = rightVol = 0;
    }
    else
    {
        /* First, handle panning (results in 64 * volume): */
        if ( (chan->panning == panSurround) || (chan->panning == panMiddle) ||
             (dsmMode & dsmMixMono) )
        {
            leftVol = 64 * chan->volume;
            rightVol = 64 * chan->volume;
        }
        else
        {
            if ( chan->panning < 0 )
            {
                leftVol = 64 * chan->volume;
                rightVol = (64 + chan->panning) * chan->volume;
            }
            else
            {
                leftVol = (64 - chan->panning) * chan->volume;
                rightVol = 64 * chan->volume;
            }
        }

        /* Then, take master volume into account: (4096*volume) */
        leftVol = leftVol * dsmMasterVolume;
        rightVol = rightVol * dsmMasterVolume;

        /* And just scale: (65536*volume -> 16.16 fixed point) */
        leftVol = 16 * leftVol;
        rightVol = 16 * rightVol;
    }

    /* Start volume ramping if necessary: */
    if ( mixInterpolate && (!mDisableVolumeRamping) )
    {
        chan->volumeRamping = 0;
        if ( (chan->leftVol != leftVol) || (chan->leftVolTarget != leftVol) )
        {
            chan->leftRampSpeed = rampSpeed;
            chan->leftVolTarget = leftVol;
            chan->volumeRamping = 1;
        }
        if ( (chan->rightVol != rightVol) ||
             (chan->rightVolTarget != rightVol) )
        {
            chan->rightRampSpeed = rampSpeed;
            chan->rightVolTarget = rightVol;
            chan->volumeRamping = 1;
        }
    }
    else
    {
        chan->volumeRamping = 0;
        chan->leftVol = chan->leftVolTarget = leftVol;
        chan->rightVol = chan->rightVolTarget = rightVol;
    }               
}



/****************************************************************************\
*
* Function:     void dsmResetVolumes(void)
*
* Description:  Resets volumes on all channels
*
\****************************************************************************/

static void dsmResetVolumes(void)
{
    unsigned i;

    for ( i = 0; i < dsmNumChannels; i++ )
        dsmSetChannelVolume(&dsmChannels[i], RAMPSPEED);
}





/****************************************************************************\
*
* Function:     int dsmMute(int mute)
*
* Description:  Mutes all channels
*
* Input:        int mute                1 = mute, 0 = un-mute
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmMute(int mute)
{
    dsmMuted = mute;
    dsmResetVolumes();
    return OK;
}




/****************************************************************************\
*
* Function:     int dsmPause(int pause)
*
* Description:  Pauses or resumes playing
*
* Input:        int pause               1 = pause, 0 = resume
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmPause(int pause)
{
    dsmPaused = pause;
    return OK;
}




/****************************************************************************\
*
* Function:     int dsmSetMasterVolume(unsigned masterVolume)
*
* Description:  Sets the master volume
*
* Input:        unsigned masterVolume   master volume (0 - 64)
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetMasterVolume(unsigned masterVolume)
{
    dsmMasterVolume = masterVolume;
    dsmResetVolumes();
    return OK;
}




/****************************************************************************\
*
* Function:     int dsmGetMasterVolume(unsigned *masterVolume)
*
* Description:  Reads the master volume
*
* Input:        unsigned *masterVolume  pointer to master volume
*
* Returns:      MIDAS error code. Master volume is written to *masterVolume.
*
\****************************************************************************/

int CALLING dsmGetMasterVolume(unsigned *masterVolume)
{
    *masterVolume = dsmMasterVolume;
    return OK;
}




/****************************************************************************\
*
* Function:     int dsmSetAmplification(unsigned amplification)
*
* Description:  Sets amplification level and calculates new volume table.
*
* Input:        unsigned amplification  amplification level, 64 = normal
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetAmplification(unsigned amplification)
{
    int         error;

    dsmAmplification = amplification;

    if ( mixInteger )
    {
        if ( (error = dsmCalcVolTable(amplification)) != OK )
            PASSERROR(ID_dsmSetAmplification)
    }

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmGetAmplification(unsigned *amplification)
*
* Description:  Reads the amplification level
*
* Input:        unsigned *amplification   pointer to amplification level
*
* Returns:      MIDAS error code. Amplification level is written to
*               *amplification.
*
\****************************************************************************/

int CALLING dsmGetAmplification(unsigned *amplification)
{
    *amplification = dsmAmplification;
    return OK;
}



void BeginTranslation(unsigned channel)
{
    dsmChannel *chan = &dsmChannels[channel];

    if ( ((chan->status == dsmChanPlaying) ||
          (chan->status == dsmChanReleased)) &&
         (chan->sampleType != smpNone) &&
         (chan->sampleLength > 2) &&
         (chan->sample != NULL) &&
         (chan->playPos < (chan->sampleLength-1)) &&
         (!chan->inTranslation) )
    {
        mMemCopy(chan->transBuffer, chan->sample +
                 (chan->playPos << dsmSampleShift(chan->sampleType)),
                 2 << dsmSampleShift(chan->sampleType));
        chan->numTransSamples = 1;
        chan->inTranslation = 1;
        chan->transPos = 0;
        chan->transSampleType = chan->sampleType;
    }
}


void EndTranslation(unsigned channel, unsigned pos)
{
    dsmChannel *chan = &dsmChannels[channel];
    unsigned s;

    if ( chan->sampleType != chan->transSampleType )
    {
        chan->inTranslation = 0;
        return;
    }

    if ( ((chan->status == dsmChanPlaying) ||
          (chan->status == dsmChanReleased)) &&
         (chan->sampleType != smpNone) &&
         (chan->sampleLength > 2) &&
         (chan->sample != NULL) &&
         (pos < (chan->sampleLength-1)) &&
         (chan->inTranslation) )
    {
        s = dsmSampleShift(chan->sampleType);
        mMemCopy(chan->transBuffer + (2 << s), chan->sample + (pos << s),
                 1 << s);
        chan->numTransSamples = 2;
    }
}




void StopTranslation(unsigned channel)
{
    dsmChannel *chan = &dsmChannels[channel];
    unsigned s, i;
    uchar v, *b;

    if ( (chan->sampleType != smpNone) &&
         (chan->inTranslation) )
    {
        s = dsmSampleShift(chan->sampleType);
        b = chan->transBuffer + (2 << s);
        v = dsmSampleClearValue(chan->sampleType);
        for ( i = 0; i < (((unsigned) 1) << s); i++ )
            *b++ = v;
        chan->numTransSamples = 2;
    }
}




/****************************************************************************\
*
* Function:     int dsmFadeOut(unsigned channel)
*
* Description:  Starts fading out a channel if fadeout channels have been
*               allocated and the channel has data being played
*
* Input:        unsigned channel        channel number
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmFadeOut(unsigned channel)
{
    dsmChannel *chan, *destChan;
    int oldStatus;

    if ( mDisableVolumeRamping )
        return OK;

    if ( (dsmNumChannels + channel) >= dsmAllocatedChannels )
        return OK;

    chan = &dsmChannels[channel];
    destChan = &dsmChannels[dsmNumChannels + channel];

    if ( (chan->status == dsmChanStopped) || (chan->status == dsmChanEnd) || 
         ((chan->leftVol == 0) && (chan->rightVol == 0)) )
        return OK;

    if ( destChan->status == dsmChanFadeOut )
        return OK;
    
    destChan->status = dsmChanFadeOut;

    /* Copy the channel information, and make sure half-ready channel data
       won't get used: */
    oldStatus = chan->status;
    chan->status = dsmChanStopped;
    mMemCopy(destChan, chan, sizeof(dsmChannel));
    chan->status = oldStatus;

    /* Start ramping: */
    destChan->volume = 0;
    destChan->status = dsmChanFadeOut;
    dsmSetChannelVolume(destChan, RAMPSPEED);
    destChan->volumeRamping = 1;
    
    return OK;
}



/****************************************************************************\
*
* Function:     int dsmFadeIn(unsigned channel)
*
* Description:  Starts fading in a channel if fadeout channels have been
*               allocated
*
* Input:        unsigned channel        channel number
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmFadeIn(unsigned channel)
{
    dsmChannel *chan, *fadeChan;

    if ( mDisableVolumeRamping )
        return OK;

    /* Don't do anything if we don't have fadeout channels */
    if ( (dsmNumChannels + channel) >= dsmAllocatedChannels )
        return OK;
    
    chan = &dsmChannels[channel];
    fadeChan = &dsmChannels[dsmNumChannels + channel];

/*    if ( fadeChan->status != dsmChanFadeOut )
        return OK;*/

    /* Ramp back up from zero: */
    chan->leftVol = 0;
    chan->rightVol = 0;
    dsmSetChannelVolume(chan, RAMPSPEED);
    
    return OK;
}




/****************************************************************************\
*
* Function:     int dsmPlaySound(unsigned channel, ulong rate)
*
* Description:  Starts playing a sound
*
* Input:        unsigned channel        channel number
*               ulong rate              playing rate in Hz
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmPlaySound(unsigned channel, ulong rate)
{
    int         error;

    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmPlaySound);
        return errInvalidChanNumber;
    }

    if ( mixInterpolate )
    {
        if ( (error = dsmFadeOut(channel)) != OK )
            PASSERROR(ID_dsmPlaySound);
    }

    /* Not playing sound yet, but SetPosition will start it: */
    dsmChannels[channel].status = dsmChanEnd;

    /* Set playing rate: */
    if ( (error = dsmSetRate(channel, rate)) != OK )
        PASSERROR(ID_dsmPlaySound)

    /* Set playing position to the beginning of the sample: */
    if ( (error = dsmSetPosition(channel, 0)) != OK )
        PASSERROR(ID_dsmPlaySound)

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmReleaseSound(unsigned channel)
*
* Description:  Releases the current sound from the channel. If sdLoop1Rel or
*               sdLoop2 looping modes are used, playing will be continued from
*               the release part of the current sample (data after the end
*               of the first loop) after the end of the first loop is reached
*               next time, otherwise the sound will be stopped.
*
* Input:        unsigned channel        channel number
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmReleaseSound(unsigned channel)
{
    dsmChannel  *chan;

    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmReleaseSound);
        return errInvalidChanNumber;
    }

    chan = &dsmChannels[channel];

    /* If no sound is being played in the channel do nothing: */
    if ( chan->status == dsmChanPlaying )
    {
        if ( (chan->loopMode == sdLoop1Rel) || (chan->loopMode == sdLoop2) )
        {
            /* Release sound - continue from release portion of the sample: */
            chan->status = dsmChanReleased;
        }
        else
        {
            /* One loop only or no looping - let playback continue normally */
        }
    }

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmStopSound(unsigned channel)
*
* Description:  Stops playing a sound
*
* Input:        unsigned channel        channel number
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmStopSound(unsigned channel)
{
    int error;
    
    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmStopSound);
        return errInvalidChanNumber;
    }

    if ( mixInterpolate )
    {
        if ( (error = dsmFadeOut(channel)) != OK )
            PASSERROR(ID_dsmStopSound);
    }

    /* Stop sound: */
    dsmChannels[channel].status = dsmChanStopped;

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmSetRate(unsigned channel, ulong rate)
*
* Description:  Sets the playing rate
*
* Input:        unsigned channel        channel number
*               ulong rate              playing rate in Hz
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetRate(unsigned channel, ulong rate)
{
    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmSetRate);
        return errInvalidChanNumber;
    }

    /* Set the playing rate: */
    dsmChannels[channel].rate = rate;

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmGetRate(unsigned channel, ulong *rate)
*
* Description:  Reads the playing rate on a channel
*
* Input:        unsigned channel        channel number
*               ulong *rate             pointer to playing rate
*
* Returns:      MIDAS error code. Playing rate is written to *rate, 0 if
*               no sound is being played.
*
\****************************************************************************/

int CALLING dsmGetRate(unsigned channel, ulong *rate)
{
    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmGetRate);
        return errInvalidChanNumber;
    }

    if ( (dsmChannels[channel].status == dsmChanStopped) ||
        (dsmChannels[channel].status == dsmChanEnd) )
    {
        /* Nothing is being played - write 0 to *rate: */
        *rate = 0;
    }
    else
    {
        /* Write the playing rate: */
        *rate = dsmChannels[channel].rate;
    }

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmSetVolume(unsigned channel, unsigned volume)
*
* Description:  Sets the playing volume
*
* Input:        unsigned channel        channel number
*               unsigned volume         playing volume (0-64)
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetVolume(unsigned channel, unsigned volume)
{
    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmSetVolume);
        return errInvalidChanNumber;
    }

    /* Set the volume: */
    dsmChannels[channel].volume = volume;

    dsmSetChannelVolume(&dsmChannels[channel], RAMPSPEED);

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmGetVolume(unsigned channel, unsigned *volume)
*
* Description:  Reads the playing volume
*
* Input:        unsigned channel        channel number
*               unsigned *volume        pointer to volume
*
* Returns:      MIDAS error code. Playing volume is written to *volume.
*
\****************************************************************************/

int CALLING dsmGetVolume(unsigned channel, unsigned *volume)
{
    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmGetVolume);
        return errInvalidChanNumber;
    }

    /* Get the volume: */
    *volume = dsmChannels[channel].volume;

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmSetSample(unsigned channel, unsigned smpHandle)
*
* Description:  Sets the sample number on a channel
*
* Input:        unsigned channel        channel number
*               unsigned smpHandle      sample handle returned by
*                                       dsmAddSample()
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetSample(unsigned channel, unsigned smpHandle)
{
    dsmChannel  *chan;
    dsmSample   *sample;
    int         error;

    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmSetSample);
        return errInvalidChanNumber;
    }

    /* Check that the sample handle is valid and the sample is in use: */
    if ( (smpHandle > MAXSAMPLES) || (dsmSamples[smpHandle-1].inUse == 0) )
    {
        ERROR(errInvalidSampleHandle, ID_dsmSetSample);
        return errInvalidSampleHandle;
    }

    chan = &dsmChannels[channel];
    sample = &dsmSamples[smpHandle-1];

    /* Set new sample number to channel: */
    chan->sampleHandle = smpHandle;

    /* Sample has been changed: */
    chan->sampleChanged = 1;

    /* If the new sample has one Amiga-compatible loop and playing has ended
       (not released or stopped), set the new sample and start playing from
       loop start: */
    if ( (sample->loopMode == sdLoopAmiga) && (chan->status == dsmChanEnd) )
    {
        /* Set sample and start playing: */
        if ( !mixInterpolate )
        {
            chan->status = dsmChanPlaying;
            if ( (error = dsmSetPosition(channel, sample->loop1Start)) != OK )
                PASSERROR(ID_dsmSetSample);
            /* (We won't do this with interpolating mixing, as this tends to
               cause clicks) */
        }
    }

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmGetSample(unsigned channel, unsigned *smpHandle)
*
* Description:  Reads current sample handle
*
* Input:        unsigned channel        channel number
*               unsigned *smpHandle     pointer to sample handle
*
* Returns:      MIDAS error code. Sample handle is written to *smpHandle;
*
\****************************************************************************/

int CALLING dsmGetSample(unsigned channel, unsigned *smpHandle)
{
    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmGetSample);
        return errInvalidChanNumber;
    }

    /* Write sample handle to *smpHandle: */
    *smpHandle = dsmChannels[channel].sampleHandle;

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmChangeSample(unsigned channel)
*
* Description:  Changes the sample used in a channel to the one specified
*               by the channel's sample handle. Used only internally by
*               other DSM functions, does no error checking.
*
* Input:        unsigned channel        channel number
*
* Returns:      MIDAS error code (does not fail)
*
\****************************************************************************/

int CALLING dsmChangeSample(unsigned channel)
{
    dsmChannel  *chan = &dsmChannels[channel];
    dsmSample   *sample = &dsmSamples[chan->sampleHandle-1];

    /* Start using the sample specified by chan->sampleHandle: */

    chan->sample = sample->sample;
    chan->sampleType = sample->sampleType;
    chan->origSampleType = sample->sampleType;
    chan->samplePos = sample->samplePos;
    chan->sampleLength = sample->sampleLength;
    chan->loopMode = sample->loopMode;
    chan->loop1Start = sample->loop1Start;
    chan->loop1End = sample->loop1End;
    chan->loop1Type = sample->loop1Type;
    chan->loop2Start = sample->loop2Start;
    chan->loop2End = sample->loop2End;
    chan->loop2Type = sample->loop2Type;
    chan->sampleChanged = 0;

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmSetPosition(unsigned channel, unsigned position)
*
* Description:  Sets the playing position from the beginning of the sample
*
* Input:        unsigned channel        channel number
*               unsigned position       new playing position
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetPosition(unsigned channel, unsigned position)
{
    dsmChannel  *chan;
    int         error;

    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmSetPosition);
        return errInvalidChanNumber;
    }

    chan = &dsmChannels[channel];

    if ( mixInterpolate )
    {
        if ( (error = dsmFadeOut(channel)) != OK )
            PASSERROR(ID_dsmSetPosition);
    }

    /* Convert position from bytes to samples: */
    position = position >> dsmSampleShift(chan->origSampleType);
    /* (we might have converted the sample to another format - the user will
       give the position as we hadn't) */

    /* Check if sample has been changed, and if so, set the values to the
       channel structure: */
    if ( chan->sampleChanged )
    {
        if ( (error = dsmChangeSample(channel)) != OK )
            PASSERROR(ID_dsmSetPosition)

        /* If channel status is released and the new sample does not have
           two loops, end the sample: */
        if ( (chan->loopMode != sdLoop1Rel) && (chan->loopMode != sdLoop2) &&
            (chan->status == dsmChanReleased) )
        {
            chan->status = dsmChanEnd;
            chan->inTranslation = 0;
            return OK;
        }
    }

    /* Check that sample and playing rate have been set on the channel: */
    if ( (chan->sampleHandle != 0) && (chan->rate != 0) )
    {
        switch ( chan->status )
        {
            case dsmChanEnd:
            case dsmChanPlaying:
                /* Either playing sample before releasing or playing has
                   ended - check the first loop type: */
                chan->loopNum = 1;
                switch ( chan->loop1Type )
                {
                    case loopNone:
                        /* No looping - if position is below sample end, set
                           it and start playing there: */
                        if ( position < chan->sampleLength )
                        {
                            chan->playPos = position;
                            if ( !chan->inTranslation )
                                chan->playPosLow = 0;
                            chan->status = dsmChanPlaying;
                            chan->direction = dsmPlayForward;
                        }
                        else
                            chan->status = dsmChanEnd;
                        break;

                    case loopUnidir:
                        /* Unidirectional looping - if position is below
                           loop end, set it, otherwise set loop start as the
                           new position. Start playing in any case: */
                        if ( position < chan->loop1End )
                            chan->playPos = position;
                        else
                            chan->playPos = chan->loop1Start;
                        if ( !chan->inTranslation )
                            chan->playPosLow = 0;
                        chan->status = dsmChanPlaying;
                        chan->direction = dsmPlayForward;
                        break;

                    case loopBidir:
                        /* Bidirectional looping - if position is below loop
                           end, set it and start playing forward, otherwise
                           set loop end as the new position and start playing
                           backwards: */
                        if ( position < chan->loop1End )
                        {
                            chan->playPos = position;
                            chan->direction = dsmPlayForward;
                        }
                        else
                        {
                            chan->playPos = chan->loop1End;
                            chan->direction = dsmPlayBackwards;
                        }
                        if ( !chan->inTranslation )
                            chan->playPosLow = 0;
                        chan->status = dsmChanPlaying;
                }
                break;

            case dsmChanReleased:
                /* Playing after sample has been released - check second loop
                   type: */
                chan->loopNum = 2;
                switch ( chan->loop2Type )
                {
                    case loopNone:
                        /* No looping - if position is below sample end, set
                           it and start playing there: */
                        if ( position < chan->sampleLength )
                        {
                            chan->playPos = position;
                            if ( !chan->inTranslation )
                                chan->playPosLow = 0;
                            chan->status = dsmChanPlaying;
                            chan->direction = dsmPlayForward;
                        }
                        else
                            chan->status = dsmChanEnd;
                        break;

                    case loopUnidir:
                        /* Unidirectional looping - if position is below
                           loop end, set it, otherwise set loop start as the
                           new position. Start playing in any case: */
                        if ( position < chan->loop2End )
                            chan->playPos = position;
                        else
                            chan->playPos = chan->loop2Start;
                        if ( !chan->inTranslation )
                            chan->playPosLow = 0;
                        chan->status = dsmChanPlaying;
                        chan->direction = dsmPlayForward;
                        break;

                    case loopBidir:
                        /* Bidirectional looping - if position is below loop
                           end, set it and start playing forward, otherwise
                           set loop end as the new position and start playing
                           backwards: */
                        if ( position < chan->loop2End )
                        {
                            chan->playPos = position;
                            chan->direction = dsmPlayForward;
                        }
                        else
                        {
                            chan->playPos = chan->loop2End;
                            chan->direction = dsmPlayBackwards;
                        }
                        if ( !chan->inTranslation )
                            chan->playPosLow = 0;
                        chan->status = dsmChanPlaying;
                }
                break;

            case dsmChanStopped:
            default:
                /* If sound has been stopped do nothing: */
                break;
        }
    }

    if ( mixInterpolate )
    {
        if ( (error = dsmFadeIn(channel)) != OK )
            PASSERROR(ID_dsmSetPosition);
    }

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmGetPosition(unsigned channel, unsigned *position)
*
* Description:  Reads the current playing position
*
* Input:        unsigned channel        channel number
*               unsigned *position      pointer to playing position
*
* Returns:      MIDAS error code. Playing position is written to *position.
*
\****************************************************************************/

int CALLING dsmGetPosition(unsigned channel, unsigned *position)
{
    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmGetPosition);
        return errInvalidChanNumber;
    }

    /* Write position to *position and convert to bytes: */
    *position = dsmChannels[channel].playPos << dsmSampleShift(
        dsmChannels[channel].origSampleType);

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmGetDirection(unsigned channel, int *direction)
*
* Description:  Reads current playing direction
*
* Input:        unsigned channel        channel number
*               int *direction          pointer to playing direction. 1 is
*                                       forward, -1 backwards
*
* Returns:      MIDAS error code. Playing direction is written to *direction.
*
\****************************************************************************/

int CALLING dsmGetDirection(unsigned channel, int *direction)
{
    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmGetDirection);
        return errInvalidChanNumber;
    }

    /* Write position to *position: */
    *direction = dsmChannels[channel].direction;

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmSetPanning(unsigned channel, int panning)
*
* Description:  Sets the panning position of a channel
*
* Input:        unsigned channel        channel number
*               int panning             panning position (see enum sdPanning)
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetPanning(unsigned channel, int panning)
{
    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmSetPanning);
        return errInvalidChanNumber;
    }

    /* Set panning position to channel: */
    dsmChannels[channel].panning = panning;

    dsmSetChannelVolume(&dsmChannels[channel], RAMPSPEED);    

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmGetPanning(unsigned channel, int *panning)
*
* Description:  Reads the panning position of a channel
*
* Input:        unsigned channel        channel number
*               int *panning            pointer to panning position
*
* Returns:      MIDAS error code. Panning position is written to *panning.
*
\****************************************************************************/

int CALLING dsmGetPanning(unsigned channel, int *panning)
{
    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmGetPanning);
        return errInvalidChanNumber;
    }

    /* Write panning position to *panning: */
    *panning = dsmChannels[channel].panning;

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmMuteChannel(unsigned channel, int mute)
*
* Description:  Mutes/un-mutes a channel
*
* Input:        unsigned channel        channel number
*               int mute                muting status - 1 = mute, 0 = un-mute
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmMuteChannel(unsigned channel, int mute)
{
    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmMuteChannel);
        return errInvalidChanNumber;
    }

    /* Set muting status: */
    dsmChannels[channel].muted = mute;

    dsmSetChannelVolume(&dsmChannels[channel], RAMPSPEED);
    
    return OK;
}




/****************************************************************************\
*
* Function:     int dsmAddSample(sdSample *sample, int copySample,
*                   unsigned *smpHandle);
*
* Description:  Adds a new sample to the DSM sample list and prepares it for
*               DSM use
*
* Input:        sdSample *sample        pointer to sample information
*                                           structure
*               int copySample          copy sample data to a new place in
*                                       memory? 1 = yes, 0 = no
*               unsigned *smpHandle     pointer to sample handle
*
* Returns:      MIDAS error code. Sample handle for the new sample is written
*               to *smpHandle
*
* Notes:        If copySample = 1, sample data must not be in EMS memory.
*               If copySample = 0 and sample is 16-bit, the sample data WILL
*               be modified by this function.
*
\****************************************************************************/

int CALLING dsmAddSample(sdSample *sample, int copySample, unsigned
    *smpHandle)
{
    int         i, handle, error;
    static void *copyDest;
    dsmSample   *dsmSmp;
    unsigned    destLength;             /* destination sample length */
    int         smpShift;
    int         ulawConvert;

    /* Find first unused sample handle: */
    handle = 0;
    for ( i = 0; i < MAXSAMPLES; i++ )
    {
        if ( dsmSamples[i].inUse == 0 )
        {
            handle = i+1;
            break;
        }
    }

    /* Check if an empty handle was found. If not, return an error: */
    if ( handle == 0 )
    {
        ERROR(errNoSampleHandles, ID_dsmAddSample);
        return errNoSampleHandles;
    }

    /* Point dsmSmp to new sample: */
    dsmSmp = &dsmSamples[handle-1];

    /* Mark sample used: */
    dsmSmp->inUse = 1;

    smpShift = dsmSampleShift(sample->sampleType);

    /* Copy sample information: */
    dsmSmp->sampleType = sample->sampleType;
    dsmSmp->origSampleType = sample->sampleType;
    dsmSmp->sampleLength = sample->sampleLength >> smpShift;
    dsmSmp->loopMode = sample->loopMode;
    dsmSmp->loop1Start = sample->loop1Start >> smpShift;
    dsmSmp->loop1End = sample->loop1End >> smpShift;
    dsmSmp->loop1Type = sample->loop1Type;
    dsmSmp->loop2Start = sample->loop2Start >> smpShift;
    dsmSmp->loop2End = sample->loop2End >> smpShift;
    dsmSmp->loop2Type = sample->loop2Type;

    /* Get rid of any 0-byte loops: */
    if ( dsmSmp->loop1Start == dsmSmp->loop1End )
        dsmSmp->loop1Type = loopNone;
    if ( dsmSmp->loop2Start == dsmSmp->loop2End )
    {
        dsmSmp->loop2Type = loopNone;
    }

    if ( (sample->sampleType == smpNone) || (sample->sampleLength == 0) ||
        (sample->sample == NULL) || (sample->samplePos == sdSmpNone) )
    {
        /* There is no sample - set up DSM sample structure accordingly: */
        dsmSmp->sampleType = smpNone;
        dsmSmp->sampleLength = 0;
        dsmSmp->sample = NULL;
        dsmSmp->copied = 0;
        dsmSmp->samplePos = sdSmpNone;
    }
    else
    {
        /* Check if we should convert sample data from 16-bit to u-law: */
        if ( ((sample->sampleType == smp16bitMono) ||
              (sample->sampleType == smp16bitStereo)) && ulawAutoConvert &&
             copySample )
        {
            ulawConvert = 1;
            if ( sample->sampleType == smp16bitMono )
                dsmSmp->sampleType = smpUlawMono;
            else
                dsmSmp->sampleType = smpUlawStereo;
        }
        else
        {
            ulawConvert = 0;
        }
        
        if ( copySample )
        {
            /* Sample data should be copied elsewhere in memory */
            destLength = sample->sampleLength;

            if ( ulawConvert )
                destLength = destLength / 2;

            /* Allocate memory for sample: */
            if ( (error = memAlloc(destLength, (void**) &dsmSmp->sample))
                 != OK )                    
                PASSERROR(ID_dsmAddSample);
                        

            copyDest = dsmSmp->sample;

            /* Sample is in conventional memory: */
            dsmSmp->samplePos = sdSmpConv;

            /* Copy sample data or convert it to correct memory area: */
            if ( ulawConvert )
            {
                if ( (error = mUlawEncode((short*) sample->sample,
                                          sample->sampleLength, copyDest))
                     != OK )
                    PASSERROR(ID_dsmAddSample);
            }
            else
            {
                mMemCopy(copyDest, sample->sample, destLength);
            }

            /* Sample is copied and should be deallocated when removed: */
            dsmSmp->copied = 1;
        }
        else
        {
            /* There is sample data, but it should not be copied - copy sample
               pointer and position: */
            dsmSmp->sample = sample->sample;
            dsmSmp->samplePos = sample->samplePos;
            dsmSmp->copied = 0;
        }
    }

    /* Write sample handle to *smpHandle: */
    *smpHandle = handle;

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmRemoveSample(unsigned smpHandle)
*
* Description:  Removes a sample from the sample list and deallocates it if
*               necessary.
*
* Input:        unsigned smpHandle      sample handle returned by
*                                       dsmAddSample()
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmRemoveSample(unsigned smpHandle)
{
    dsmSample   *sample;
    int         error;

    /* Check that the sample handle is valid and the sample is in use: */
    if ( (smpHandle > MAXSAMPLES) || (dsmSamples[smpHandle-1].inUse == 0) )
    {
        ERROR(errInvalidSampleHandle, ID_dsmRemoveSample);
        return errInvalidSampleHandle;
    }

    sample = &dsmSamples[smpHandle-1];

    /* Mark sample unused: */
    sample->inUse = 0;

    /* Check if sample data should be deallocated: */
    if ( sample->copied )
    {
        if ( (error = memFree(sample->sample)) != OK )
            PASSERROR(ID_dsmRemoveSample)
    }

    return OK;
}




#ifdef SUPPORTSTREAMS


/****************************************************************************\
*
* Function:     int dsmStartStream(unsigned channel, uchar *buffer,
*                   unsigned bufferLength, int sampleType);
*
* Description:  Starts playing a digital audio stream on a channel
*
* Input:        unsigned channel        channel number
*               uchar *buffer           pointer to stream buffer
*               unsigned bufferLength   buffer length in bytes
*               int sampleType          stream sample type
*               ulong rate              stream playing rate (in Hz)
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmStartStream(unsigned channel, uchar *buffer, unsigned
    bufferLength, int sampleType, ulong rate)
{
    dsmChannel  *chan;

    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmStartStream);
        return errInvalidChanNumber;
    }

    chan = &dsmChannels[channel];

    /* Set up channel for playing the stream: */
    chan->sample = buffer;
    chan->sampleType = sampleType;
    chan->origSampleType = sampleType;
    chan->samplePos = sdSmpConv;
    chan->sampleLength = bufferLength >> dsmSampleShift(sampleType);
    chan->loopMode = sdLoop1;
    chan->loop1Start = 0;
    chan->loop1End = bufferLength >> dsmSampleShift(sampleType);
    chan->loop1Type = loopUnidir;
    chan->loop2Start = chan->loop2End = 0;
    chan->loop2Type = sdLoopNone;
    chan->playPos = chan->playPosLow = 0;
    chan->rate = rate;
    chan->direction = dsmPlayForward;
    chan->sampleHandle = DSM_SMP_STREAM;        /* magic */
    chan->sampleChanged = 0;
    chan->panning = panMiddle;
    chan->volume = 64;
    chan->muted = 0;
    chan->loopNum = 1;
    chan->status = dsmChanPlaying;
    chan->streamWritePos = 0;

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmStopStream(unsigned channel);
*
* Description:  Stops playing digital audio stream on a channel
*
* Input:        unsigned channel        channel number
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmStopStream(unsigned channel)
{
    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmStopSound);
        return errInvalidChanNumber;
    }

    /* Stop sound: */
    dsmChannels[channel].status = dsmChanStopped;

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmSetLoopCallback(unsigned channel,
*                   void (CALLING *callback)(unsigned channel));
*
* Description:  Sets sample looping callback to a channel
*
* Input:        unsigned channel        channel number
*               [..] *callback          pointer to callback function, NULL to
*                                       disable callback
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetLoopCallback(unsigned channel,
    void (CALLING *callback)(unsigned channel))
{
    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmStopSound);
        return errInvalidChanNumber;
    }

    /* Set callback: */
    dsmChannels[channel].LoopCallback = callback;

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmSetStreamWritePosition(unsigned channel,
*                   unsigned position)
*
* Description:  Sets the stream write position on a channel
*
* Input:        unsigned channel        channel number
*               unsigned position       new stream write position
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetStreamWritePosition(unsigned channel, unsigned position)
{
    dsmChannel  *chan;

    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmSetStreamWritePosition);
        return errInvalidChanNumber;
    }

    chan = &dsmChannels[channel];

    chan->streamWritePos = position >> dsmSampleShift(chan->sampleType);

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmPauseStream(unsigned channel, int pause)
*
* Description:  Pauses/resumes playing a stream on a channel
*
* Input:        unsigned channel        the channel the stream is being played
*                                       on
*               int pause               1 to pause, 0 to resume
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmPauseStream(unsigned channel, int pause)
{
    /* Check that the channel number is legal and channels are open: */
    if ( channel >= dsmNumChannels )
    {
        ERROR(errInvalidChanNumber, ID_dsmPauseStream);
        return errInvalidChanNumber;
    }

    if ( pause )
        dsmChannels[channel].status = dsmChanStopped;
    else
        dsmChannels[channel].status = dsmChanPlaying;

    return OK;
}




#endif /* #ifdef SUPPORTSTREAMS */



/****************************************************************************\
*
* Function:     int dsmAddPostProcessor(dsmPostProcessor *postProc,
*                   unsigned procPos);
*
* Description:  Adds a post processor to the sound output
*
* Input:        dsmPostProcessor *postProc  the post-processor structure
*               unsigned procPos            post-processing position, see
*                                           enum dsmPostProcPosition
*               void *userData              user data argument for the
*                                           post-processing functions
*
* Returns:      MIDAS error code.
*
* Notes:        If any of the functions pointer is NULL, that function is
*               ignored.
*
\****************************************************************************/

int CALLING dsmAddPostProcessor(dsmPostProcessor *postProc, unsigned procPos,
                                void *userData)
{
    assert(dsmPostProcList);
    assert((procPos == dsmPostProcFirst) ||
           (procPos == dsmPostProcLast));

    postProc->userData = userData;

    switch ( procPos )
    {
        case dsmPostProcFirst:          
            postProc->prev = dsmPostProcList;
            postProc->next = dsmPostProcList->next;
            dsmPostProcList->next->prev = postProc;
            dsmPostProcList->next = postProc;
            break;

        case dsmPostProcLast:
            postProc->next = dsmPostProcList;
            postProc->prev = dsmPostProcList->prev;
            dsmPostProcList->prev->next = postProc;
            dsmPostProcList->prev = postProc;
            break;
    }

    return OK;
}




/****************************************************************************\
*
* Function:     int dsmRemovePostProcessor(dsmPostProcessor *postProc)
*
* Description:  Removes a post processor from sound output
*
* Input:        dsmPostProcessor *postProc  the post-processor structure
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmRemovePostProcessor(dsmPostProcessor *postProc)
{
    postProc->prev->next = postProc->next;
    postProc->next->prev = postProc->prev;

    return OK;
}



/*
 * $Log: dsm.c,v $
 * Revision 1.41.2.2  1997/09/06 17:16:24  pekangas
 * Floating point mixing is now default for PPro-class CPUs
 *
 * Revision 1.41.2.1  1997/08/22 14:31:53  pekangas
 * Fixed a problem with extremely short loops (length less than the mixing
 * step), and fixed a potential problem with zero-length loops.
 *
 * Revision 1.41  1997/07/31 10:56:37  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.40  1997/07/30 19:04:02  pekangas
 * Added optimized mono u-law mixing routines and enabled u-law autoconversion
 * by default
 *
 * Revision 1.39  1997/07/28 13:20:24  pekangas
 * Fixed to compile to DOS with Watcom C
 *
 * Revision 1.38  1997/07/25 13:47:54  pekangas
 * Added automatic filtering and made it the default
 *
 * Revision 1.37  1997/07/25 09:58:09  pekangas
 * Added integer filters
 *
 * Revision 1.36  1997/07/24 17:09:49  pekangas
 * Added surround support and support for separate surround and middle mixing
 * routines. Fixed interpolating mixing.
 *
 * Revision 1.35  1997/07/24 14:54:10  pekangas
 * Added support for integer stereo mixing
 *
 * Revision 1.34  1997/07/23 17:27:56  pekangas
 * Added profiling support
 *
 * Revision 1.33  1997/07/17 13:31:05  pekangas
 * Added support for copying and relocating mixing loops in data segment,
 * thus allowing self-modifying code.
 *
 * Revision 1.32  1997/07/17 09:55:45  pekangas
 * Added support for integer mono mixing
 *
 * Revision 1.31  1997/07/14 17:44:02  pekangas
 * Fixed yet another clicking problem.
 * Doubled volume ramping speed.
 *
 * Revision 1.30  1997/07/11 12:53:25  pekangas
 * Fixed filters
 *
 * Revision 1.29  1997/07/10 18:40:22  pekangas
 * Added echo effect support
 *
 * Revision 1.28  1997/07/05 14:21:08  pekangas
 * Fixed the clicks for good
 *
 * Revision 1.27  1997/07/03 15:03:45  pekangas
 * Interpolating mixing no longer indexes past the real sample or loop end,
 * thus replicating data for it is not necessary
 *
 * Revision 1.26  1997/06/20 10:09:39  pekangas
 * Added translation to sound stop
 * Added volume ramping
 *
 * Revision 1.25  1997/06/18 15:56:55  pekangas
 * Changed sample endings for interpolating mixing - reduces clicks?
 *
 * Revision 1.24  1997/06/11 15:37:09  pekangas
 * Added interpolating stereo mixing routines
 *
 * Revision 1.23  1997/06/11 15:15:22  pekangas
 * Fixed new sound starts with interpolation
 *
 * Revision 1.22  1997/06/11 14:37:06  pekangas
 * Added support for post-processing routines.
 * Added automatic amplification for dsmOpenChannel().
 * Removed unused outputBits -argument from dsmInit().
 *
 * Revision 1.21  1997/06/05 20:18:46  pekangas
 * Added preliminary support for interpolating mixing (mono only at the
 * moment)
 *
 * Revision 1.20  1997/06/04 15:21:27  pekangas
 * Aligned byte->float and ulaw->float tables at 1024-byte boundaries
 *
 * Revision 1.19  1997/06/01 18:09:13  jpaana
 * Fixed a warning
 *
 * Revision 1.18  1997/05/30 18:25:22  pekangas
 * Completely new mixing routines
 *
 * Revision 1.17  1997/05/27 10:02:23  pekangas
 * Merged changes from no-ulaw branch, in preparation for new mixing routines
 *
 * Revision 1.16  1997/03/29 15:28:07  jpaana
 * Removed some debug printf's
 *
 * Revision 1.15  1997/03/22 12:01:00  pekangas
 * Changed buffer alignment
 *
 * Revision 1.14  1997/03/11 18:57:33  pekangas
 * Aligned the mixing buffer to a 128-byte boundary
 *
 * Revision 1.13  1997/03/05 19:35:17  pekangas
 * Added 16-bit -> u-law autoconversion
 *
 * Revision 1.12  1997/02/20 19:48:51  pekangas
 * Added support for u-law sample data
 *
 * Revision 1.11.2.4  1997/05/07 17:05:24  pekangas
 * Brought mono filter 2 up to date with stereo
 *
 * Revision 1.11.2.3  1997/05/04 18:51:32  pekangas
 * Tweaked filter #2 a bit to sound better
 *
 * Revision 1.11.2.2  1997/05/03 17:53:25  pekangas
 * Added some optional simple output filtering
 *
 * Revision 1.11.2.1  1997/04/07 21:07:14  pekangas
 * Added dsmPauseStream
 *
 * Revision 1.11  1997/01/16 18:41:59  pekangas
 * Changed copyright messages to Housemarque
 *
 * Revision 1.10  1997/01/16 18:19:10  pekangas
 * Added support for setting the stream write position.
 * Stream data is no longer played past the write position
 *
 * Revision 1.9  1996/10/09 15:54:22  pekangas
 * Fixed dsmReleaseSound() to work as specified
 *
 * Revision 1.8  1996/07/13 19:44:22  pekangas
 * Eliminated Visual C warnings
 *
 * Revision 1.7  1996/07/13 18:40:48  pekangas
 * Fixed to compile with Visual C
 *
 * Revision 1.6  1996/06/26 19:14:55  pekangas
 * Added sample loop callbacks
 *
 * Revision 1.5  1996/05/30 21:10:27  pekangas
 * Fixed a small bug in looping other samples than 8-bit mono
 *
 * Revision 1.4  1996/05/28 20:31:11  pekangas
 * Added support for 8-bit stereo and 16-bit mono and stereo samples
 *
 * Revision 1.3  1996/05/26 20:55:39  pekangas
 * Implemented digital audio stream support
 *
 * Revision 1.2  1996/05/24 16:19:39  jpaana
 * Misc fixes for Linux
 *
 * Revision 1.1  1996/05/22 20:49:33  pekangas
 * Initial revision
 *
*/
