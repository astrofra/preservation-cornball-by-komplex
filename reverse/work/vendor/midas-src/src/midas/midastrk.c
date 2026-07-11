/*      midastrk.cc
 *
 * MIDAS tracker API
 *
 * $Id: midastrk.c,v 1.5.2.3 1998/01/01 18:03:35 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#if defined(__NT__) || defined(__WINDOWS__) || defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <string.h>

#include "midas.h"

/* This is a kluge, but necessary as Watcom C sucks: */
#if defined(M_DLL)
#define EXPORT_IN_MIDASDLL_H
#endif

#include "midasdll.h"
#include "midastrk.h"


RCSID(const char *midastrk_rcsid = "$Id: midastrk.c,v 1.5.2.3 1998/01/01 18:03:35 pekangas Exp $";)



/* These should come from a header or something */    

#ifdef M_HAVE_THREADS
#  define M_LOCK LockPollThread();
#  define M_UNLOCK UnlockPollThread();
#else
#  define M_LOCK {}
#  define M_UNLOCK {}
#endif

#define MAXPLAYHANDLES 16

typedef struct _ModulePlayHandle
{
    unsigned     numChannels;
    gmpPlayHandle playHandle;
    unsigned     channels[EMPTYARRAY];
} ModulePlayHandle;


extern ModulePlayHandle *mPlayHandles[MAXPLAYHANDLES];

extern int mLastError;





/****************************************************************************\
*
* Function:     BOOL MTRKdecompressPattern(MTRKmodule *module,
*                   unsigned pattern);
*
* Description:  Decompresses a pattern in place
*
* Input:        MTRKmodule *module      The module
*               unsigned pattern        The pattern number
*
* Returns:      TRUE if successful, FALSE if not
*
\****************************************************************************/

_FUNC(BOOL) MTRKdecompressPattern(MTRKmodule *module, unsigned pattern)
{
    MTRKpatternDataEntry *e;
    static MTRKpattern *patt;
    gmpPattern *gmpp;
    unsigned i;
    unsigned size;
    unsigned chan, row;
    uchar *s, *c;
    uchar compInfo;
    int error;
    
    M_LOCK

    /* Allocate memory for the decompressed pattern: */
    size = sizeof(MTRKpattern) + module->numChannels * 256 *
        sizeof(MTRKpatternDataEntry) + 256;
    if ( (error = memAlloc(size, (void**) &patt))
         != OK )
    {
        mLastError = error;
        return FALSE;
    }

    /* Prepare pattern header: */
    gmpp = (gmpPattern*) module->patterns[pattern];
    patt->length = size;
    patt->rows = gmpp->rows;

    /* Clear the pattern data area: */
    e = (MTRKpatternDataEntry*) &patt->data[0];
    for ( row = 0; row < 256; row++ )
    {
        for ( chan = 0; chan < module->numChannels; chan++ )
        {
            e->compInfo = 0xE0 | chan;
            e->note = 0xFF;
            e->instrument = 0xFF;
            e->volume = 0x50;
            e->command = 0;
            e->infobyte = 0;
            e++;
        }
        /* end-of-row marker: */
        c = (uchar*) e;
        *(c++) = 0;
        e = (MTRKpatternDataEntry*) c;
    }    

    /* Decompress the actual pattern: */
    e = (MTRKpatternDataEntry*) &patt->data[0];
    s = &gmpp->data[0];
    for ( row = 0; row < gmpp->rows; row++ )
    {
        do
        {
            compInfo = *(s++);
            chan = compInfo & 0x1F;

            /* Skip data on channels that do not really exist: */
            if ( chan >= module->numChannels )                
            {
                if ( compInfo & 32 )
                    s += 2;
                if ( compInfo & 64 )
                    s++;
                if ( compInfo & 128 )
                    s += 2;
            }
            else
            {            
                if ( compInfo & 32 )
                {
                    e[chan].note = *(s++);
                    e[chan].instrument = *(s++);
                }
                if ( compInfo & 64 )
                {
                    e[chan].volume = *(s++);
                }
                if ( compInfo & 128 )
                {
                    e[chan].command = *(s++);
                    e[chan].infobyte = *(s++);
                }
            }
        } while ( compInfo != 0 );        
        e += module->numChannels;
        e = (MTRKpatternDataEntry*) (((uchar*) e) + 1);
    }

    /* Deallocate the compressed pattern and set the decompressed one
       in its place in the module: */
    module->patterns[pattern] = patt;
    if ( (error = memFree(gmpp)) != OK )
    {
        mLastError = error;
        return FALSE;
    }
    

    /* Reset the playing pointers for all playing handles that are
       playing this module: */
    for ( i = 0; i < MAXPLAYHANDLES; i++ )
    {        
        if ( (mPlayHandles[i] != NULL) &&
             (mPlayHandles[i]->playHandle->module == (gmpModule*) module) &&
             (mPlayHandles[i]->playHandle->pattern == pattern) )
            mPlayHandles[i]->playHandle->playPtr = NULL;
    }

    M_UNLOCK
    
    return TRUE;
}



/****************************************************************************\
*
* Function:     BOOL MTRKcompressPattern(MTRKmodule *module,
*                   unsigned pattern);
*
* Description:  Re-compresses a pattern decompressed with
*               MTRKdecompressPattern().
*
* Input:        MTRKmodule *module      The module
*               unsigned pattern        The pattern number
*
* Returns:      TRUE if successful, FALSE if not
*
\****************************************************************************/

_FUNC(BOOL) MTRKcompressPattern(MTRKmodule *module, unsigned pattern)
{
    static gmpPattern *patt;
    static gmpPattern *destp;
    static gmpPattern *finalp;
    MTRKpatternDataEntry *e;
    unsigned i;
    unsigned chan, row, dataLen;
    uchar *s, *d;
    uchar compInfo;
    int error;
    
    M_LOCK

    patt = (gmpPattern*) module->patterns[pattern];

    /* Allocate temporary working space for the new pattern: */
    if ( (error = memAlloc(patt->length, (void**) &destp)) != OK )
    {
        mLastError = error;
        return FALSE;
    }

    /* Compress the data: */
    s = patt->data;
    d = destp->data;
    for ( row = 0; row < patt->rows; row++ )
    {
        for ( chan = 0; chan < module->numChannels; chan++ )
        {
            e = (MTRKpatternDataEntry*) s;
            s += sizeof(MTRKpatternDataEntry);
            compInfo = chan;
            
            if ( (e->note != 0xFF) || (e->instrument != 0xFF) )
                compInfo |= 32;
            if ( e->volume != 0x50 )
                compInfo |= 64;
            if ( e->command != 0 )
                compInfo |= 128;

            if ( (compInfo & 0xE0) != 0 )
            {
                *(d++) = compInfo;
                if ( compInfo & 32 )
                {
                    *(d++) = e->note;
                    *(d++) = e->instrument;
                }
                if ( compInfo & 64 )
                    *(d++) = e->volume;
                if ( compInfo & 128 )
                {
                    *(d++) = e->command;
                    *(d++) = e->infobyte;
                }
            }
        }
        *(d++) = 0;
        s++;
    }
    dataLen = d - destp->data;

    /* Allocate space for the final pattern: */
    if ( (error = memAlloc(sizeof(gmpPattern) + dataLen, (void**) &finalp))
         != OK )
    {
        mLastError = error;
        return FALSE;
    }

    /* Fill in the header and copy data: */
    finalp->rows = patt->rows;
    finalp->length = sizeof(gmpPattern) + dataLen;
    memcpy(finalp->data, destp->data, dataLen);

    /* Deallocate the old patterns and set the compressed one in its
       place in the module: */
    module->patterns[pattern] = (MTRKpattern*) finalp;
    if ( (error = memFree(patt)) != OK )
    {
        mLastError = error;
        return FALSE;
    }
    if ( (error = memFree(destp)) != OK )
    {
        mLastError = error;
        return FALSE;
    }   

    /* Reset the playing pointers for all playing handles that are
       playing this module: */
    for ( i = 0; i < MAXPLAYHANDLES; i++ )
    {        
        if ( (mPlayHandles[i] != NULL) &&
             (mPlayHandles[i]->playHandle->module == (gmpModule*) module) &&
             (mPlayHandles[i]->playHandle->pattern == pattern) )
            mPlayHandles[i]->playHandle->playPtr = NULL;
    }

    M_UNLOCK
    
    return TRUE;
}



/****************************************************************************\
*
* Function:     MIDASmodulePlayHandle MTRKplayModuleInstruments(MTRKmodule
*                   *module)
*
* Description:  Starts playing the instruments from a module
*
* Input:        MTRKmodule *module      the module
*
* Returns:      MIDAS module playing handle if successful, 0 if not
*
\****************************************************************************/

_FUNC(MIDASmodulePlayHandle) MTRKplayModuleInstruments(MTRKmodule *module)
{
    int error;

    M_LOCK
    
    /* Create an empty pattern 256 if necessary: */
    if ( module->patterns[256] == NULL )
    {
        if ( (error = memAlloc(sizeof(MTRKpattern) + 1,
                               (void**) &module->patterns[256])) != OK )
        {
            mLastError = error;
            M_UNLOCK
            return 0;
        }
        module->patterns[256]->length = sizeof(MTRKpattern) + 1;
        module->patterns[256]->rows = 1;
        module->patterns[256]->data[0] = 0;
        
        module->songData[257] = 256;
        /* song position 256 is used for playing single patterns */
    }

    M_UNLOCK

    return MIDASplayModuleSection((MIDASmodule) module, 257, 257, 257, TRUE);
}



/****************************************************************************\
*
* Function:     BOOL MTRKplayInstrumentNote(MIDASmodulePlayHandle playHandle,
*                   unsigned channel, unsigned instrument, unsigned note)
*
* Description:  Plays a note with a given instrument from a module
*
* Input:        MIDASmodulePlayHandle playHandle  the module playing handle
*               unsigned channel        channel number
*               unsigned instrument     instrument number (0-based)
*               unsigned note           note to play (upper nybble octave)
*
* Returns:      TRUE if successful, FALSE if not
*
\****************************************************************************/

_FUNC(BOOL) MTRKplayInstrumentNote(MIDASmodulePlayHandle playHandle,
                                   unsigned channel, unsigned instrument,
                                   unsigned note)
{
    unsigned sample, xmNote;
    ModulePlayHandle *h = mPlayHandles[playHandle-1];
    gmpInstrument *inst = h->playHandle->module->instruments[instrument];

    if ( inst == NULL )
        return TRUE;

    xmNote = (note & 0xF) + (((note & 0xF0) >> 4) * 12);
    if ( xmNote > 95 )
        xmNote = 95;
    if ( inst->noteSamples == NULL )
        sample = 0;
    else
        sample = inst->noteSamples[xmNote];

    return MTRKplaySampleNote(playHandle, channel, instrument, sample, note);
}




/****************************************************************************\
*
* Function:     BOOL MTRKplaySampleNote(MIDASmodulePlayHandle playHandle,
*                   unsigned channel, unsigned instrument, unsigned sample,
*                   unsigned note)
*
* Description:  Plays a note with a given instrument from a module
*
* Input:        MIDASmodulePlayHandle playHandle  the module playing handle
*               unsigned channel        channel number
*               unsigned instrument     instrument number (0-based)
*               unsigned sample         sample number for the instrument
*               unsigned note           note to play (upper nybble octave)
*
* Returns:      TRUE if successful, FALSE if not
*
\****************************************************************************/

_FUNC(BOOL) MTRKplaySampleNote(MIDASmodulePlayHandle playHandle,
                               unsigned channel, unsigned instrument,
                               unsigned sample, unsigned note)
{
    int error;
    ModulePlayHandle *h = mPlayHandles[playHandle-1];
    gmpInstrument *inst = h->playHandle->module->instruments[instrument];
    gmpSample *samp;
    gmpChannel *chan = &h->playHandle->channels[channel];

    if ( (inst == NULL) || (sample >= inst->numSamples) )
        return TRUE;

    samp = inst->samples[sample];

    if ( samp == NULL )
        return TRUE;        

    M_LOCK

    gmpChan = chan;
    gmpHandle = h->playHandle;
    gmpCurModule = gmpHandle->module;
    gmpPlayMode = gmpCurModule->playMode;

    chan->volEnv.x = 0;
    chan->volEnv.sustained = 0;
    chan->panEnv.x = 0;
    chan->panEnv.sustained = 0;
    chan->fadeOut = 0x10000;
    chan->autoVibPos = 0;
    chan->autoVibDepth = 0;
    
    /* Key is not released any more: */
    chan->keyOff = 0;

    chan->instrument = instrument;
    chan->sample = sample;

    /* Set instrument volume as current channel volume: */
    gmpSetVolume(samp->volume);

    /* Set instrument panning as current channel panning: */
    gmpSetPanning(samp->panning);

    /* Set new note start offset to zero: */
    chan->startOffset = 0;

    /* Set the sample to the Sound Device: */
    if ( inst->samples[sample]->sdHandle != 0 )
    {
        if ( (error = gmpSD->SetSample(chan->sdChannel,
                                       inst->samples[sample]->sdHandle))
             != OK )
        {
            mLastError = error;
            M_UNLOCK
            return FALSE;
        }
    }

    chan->note = note;
    if ( (error = gmpNewNote()) != OK )
    {
        mLastError = error;
        M_UNLOCK
        return FALSE;
    }
    
    
    M_UNLOCK

    return TRUE;
}




/****************************************************************************\
*
* Function:     BOOL MTRKreleaseNote(MIDASmodulePlayHandle playHandle,
*                   unsigned channel);
*
* Description:  Releases the note that is being played on a channel
*
* Input:        MIDASmodulePlayHandle playHandle   the playing handle
*               unsigned channel        channel number
*
* Returns:      TRUE if successful, FALSE if not
*
\****************************************************************************/

_FUNC(BOOL) MTRKreleaseNote(MIDASmodulePlayHandle playHandle,
                            unsigned channel)
{
    int error;
    ModulePlayHandle *h = mPlayHandles[playHandle-1];
    gmpChannel *chan = &h->playHandle->channels[channel];

    M_LOCK

    gmpChan = chan;
    gmpHandle = h->playHandle;

    chan->note = 0xFE;
    if ( (error = gmpNewNote()) != OK )
    {
        mLastError = error;
        M_UNLOCK
        return FALSE;
    }
    
    
    M_UNLOCK

    return TRUE;
    
}




/****************************************************************************\
*
* Function:     BOOL MTRKstopNote(MIDASmodulePlayHandle playHandle,
*                   unsigned channel);
*
* Description:  Stops the note that is being played on a channel
*
* Input:        MIDASmodulePlayHandle playHandle   the playing handle
*               unsigned channel        channel number
*
* Returns:      TRUE if successful, FALSE if not
*
\****************************************************************************/

_FUNC(BOOL) MTRKstopNote(MIDASmodulePlayHandle playHandle, unsigned channel)
{
    int error;
    ModulePlayHandle *h = mPlayHandles[playHandle-1];
    gmpChannel *chan = &h->playHandle->channels[channel];

    M_LOCK

    if ( (error = gmpSD->StopSound(chan->sdChannel)) != OK )
    {
        mLastError = error;
        M_UNLOCK
        return FALSE;
    }
    
    M_UNLOCK

    return TRUE;
    
}


/****************************************************************************\
*
* Function:     void* MTRKmemAlloc(unsigned numBytes);
*
* Description:  A simple wrapper around MIDAS memAlloc()
*
* Input:        unsigned numBytes       number of bytes to allocate
*
* Returns:      Pointer to allocated memory if successful, NULL if failure
*
\****************************************************************************/

_FUNC(void*) MTRKmemAlloc(unsigned numBytes)
{
    int error;
    void *blk;

    if ( (error = memAlloc(numBytes, &blk)) != OK )
    {
        mLastError = error;
        return NULL;
    }

    return blk;
}




/****************************************************************************\
*
* Function:     BOOL MTRKmemFree(void *block);
*
* Description:  A simple wrapper around MIDAS memFree()
*
* Input:        void *block             block to deallocate
*
* Returns:      TRUE if successful, FALSE if not. Usually doesn't fail.
*
\****************************************************************************/

_FUNC(BOOL) MTRKmemFree(void *block)
{
    int error;

    if ( (error = memFree(block)) != OK )
    {
        mLastError = error;
        return FALSE;
    }

    return TRUE;
}



/****************************************************************************\
*
* Function:     void StopInstrument(MIDASmodulePlayHandle playHandle,
*                   unsigned instrument)
*
* Description:  Stops all notes playing with a given instrument, and removes
*               the instrument from the channels
*
* Input:        MIDASmodulePlayHandle playHandle  playback handle
*               unsigned instrument     the instrument number
*
\****************************************************************************/

void StopInstrument(MIDASmodulePlayHandle playHandle, unsigned instrument)
{
    gmpPlayHandle h = mPlayHandles[playHandle-1]->playHandle;
    gmpChannel *chan;
    unsigned n;

    M_LOCK;

    for ( n = 0; n < h->numChannels; n++ )
    {
        chan = &h->channels[n];
        if ( (unsigned) chan->instrument == instrument )
        {
            gmpSD->StopSound(chan->sdChannel);
            chan->instrument = -1;
            chan->sample = 0xFF;
        }
    }

    M_UNLOCK;
}



/****************************************************************************\
*
* Function:     BOOL MTRKresetSample(MTRKmodule *module, unsigned instrument,
*                   unsigned sample, MIDASmodulePlayHandle playHandle)
*
* Description:  Resets a sample to the Sound Device, flushing any sample data
*               changes, taking new loop information into use etc.
*
*               Note! If you are reallocating the sample data, first allocate
*               the memory for the new sample data (using MTRKmemAlloc()),
*               point MTRKsample.sample to the new data, call
*               MTRKresetSample() and only deallocate the old data after that.
*
* Input:        MTRKmodule *module      the module
*               unsigned instrument     instrument number
*               unsigned sample         sample number in instrument
*               MIDASmodulePlayHandle playHandle  playing handle that is used
*                                       for playing the module, 0 if none
*
* Returns:      TRUE if successful, FALSE if not
*
\****************************************************************************/

_FUNC(BOOL) MTRKresetSample(MTRKmodule *module, unsigned instrument,
                            unsigned sample, MIDASmodulePlayHandle playHandle)
{
    int error;
    sdSample sdSmp;
    MTRKsample *smp;
    
    M_LOCK

    if ( playHandle )
        StopInstrument(playHandle, instrument);

    smp = module->instruments[instrument]->samples[sample];

    sdSmp.sample = smp->sample;
    sdSmp.samplePos = sdSmpConv;
    sdSmp.sampleType = smp->sampleType;
    sdSmp.sampleLength = smp->sampleLength;
    sdSmp.loopMode = smp->loopMode;
    sdSmp.loop1Start = smp->loop1Start;
    sdSmp.loop1End = smp->loop1End;
    sdSmp.loop1Type = smp->loop1Type;
    sdSmp.loop2Start = smp->loop2Start;
    sdSmp.loop2End = smp->loop2End;
    sdSmp.loop2Type = smp->loop2Type;

    if ( smp->sdHandle )
    {
        if ( (error = gmpSD->RemoveSample(smp->sdHandle)) != OK )
        {
            mLastError = error;
            M_UNLOCK
            return FALSE;
        }
    }

    if ( (error = gmpSD->AddSample(&sdSmp, 0, &smp->sdHandle)) != OK )
    {
        mLastError = error;
        M_UNLOCK
        return FALSE;
    }

    M_UNLOCK
    return TRUE;          
}



/****************************************************************************\
*
* Function:     BOOL MTRKremoveSample(MTRKmodule *module, unsigned instrument,
*                   unsigned sample, MIDASmodulePlayHandle playHandle)
*
* Description:  Removes a sample from a Sound Device, deallocates its sample
*               structures, and frees the sample for other use.
*
* Input:        MTRKmodule *module      the module
*               unsigned instrument     the instrument number
*               unsigned sample         the sample number in the instrument
*               MIDASmodulePlayHandle playHandle  playing handle that is used
*                                       for playing the module, 0 if none
*
* Returns:      TRUE if successful, FALSE if not
*
\****************************************************************************/

_FUNC(BOOL) MTRKremoveSample(MTRKmodule *module, unsigned instrument,
                             unsigned sample, MIDASmodulePlayHandle playHandle)
{
    int error;
    MTRKsample *smp;
    MTRKinstrument *inst;
    unsigned i;
    
    M_LOCK

    if ( playHandle )
        StopInstrument(playHandle, instrument);

    inst = module->instruments[instrument];
    if ( (inst == NULL) || (inst->numSamples <= sample) ||
         (inst->samples[sample] == NULL) )
    {
        M_UNLOCK
        return TRUE;
    }

    smp = inst->samples[sample];

    if ( smp->sdHandle != 0 )
    {
        if ( (error = gmpSD->RemoveSample(smp->sdHandle)) != OK )
        {
            mLastError = error;
            M_UNLOCK
            return FALSE;
        }
    }

    for ( i = 0; i < module->numSamples; i++ )
    {
        if ( module->samples[i] == smp )
        {
            module->samples[i] = NULL;
            break;
        }
    }

    if ( smp->sample != NULL )
    {
        if ( (error = memFree(smp->sample)) != OK )
        {
            mLastError = error;
            M_UNLOCK
            return FALSE;
        }
    }

    if ( (error = memFree(smp)) != OK )
    {
        mLastError = error;
        M_UNLOCK
        return FALSE;
    }

    inst->samples[sample] = NULL;

    M_UNLOCK
    return TRUE;          
}


    
/****************************************************************************\
*
* Function:     BOOL MTRKremoveInstrument(MTRKmodule *module,
*                   unsigned instrument, MIDASmodulePlayHandle playHandle)
*
* Description:  Removes an instrument and all its samples from a module.
*
* Input:        MTRKmodule *module      the module
*               unsigned instrument     the instrument number
*               MIDASmodulePlayHandle playHandle  playing handle that is used
*                                       for playing the module, 0 if none
*
* Returns:      TRUE if successful, FALSE if not
*
\****************************************************************************/

_FUNC(BOOL) MTRKremoveInstrument(MTRKmodule *module, unsigned instrument,
                                 MIDASmodulePlayHandle playHandle)
{
    int error;
    MTRKinstrument *inst;
    unsigned i;
    
    M_LOCK

    if ( playHandle )
        StopInstrument(playHandle, instrument);

    inst = module->instruments[instrument];
    if ( inst == NULL )
    {
        M_UNLOCK
        return TRUE;
    }

    for ( i = 0; i < inst->numSamples; i++ )
    {
        if ( !MTRKremoveSample(module, instrument, i, playHandle) )
        {
            M_UNLOCK
            return FALSE;
        }
    }

    if ( (error = memFree(inst)) != OK )
    {
        mLastError = error;
        M_UNLOCK
        return FALSE;
    }

    module->instruments[instrument] = NULL;

    M_UNLOCK
    return TRUE;          
}



/****************************************************************************\
*
* Function:     BOOL MTRKnewInstrument(MTRKmodule *module, unsigned instrument, MIDASmodulePlayHandle playHandle)
*
* Description:  Allocates a new instrument structure and fills it with
*               reasonable default values
*
* Input:        MTRKmodule *module      the module
*               unsigned instrument     instrument number
*               MIDASmodulePlayHandle playHandle  playing handle that is used
*                                       for playing the module, 0 if none
*
* Returns:      TRUE if successful, FALSE if not
*
\****************************************************************************/

_FUNC(BOOL) MTRKnewInstrument(MTRKmodule *module, unsigned instrument,
                              MIDASmodulePlayHandle playHandle)
{
    int error;
    MTRKinstrument *inst;
    
    M_LOCK
        
    if ( playHandle )
        StopInstrument(playHandle, instrument);

    if ( module->instruments[instrument] != NULL )
        MTRKremoveInstrument(module, instrument, playHandle);

    if ( (error = memAlloc(sizeof(MTRKinstrument) + 16 * sizeof(MTRKsample*),
                           (void**) &inst)) != OK )
    {
        mLastError = error;
        M_UNLOCK
        return FALSE;
    }

    memset((void*) inst, 0, sizeof(MTRKinstrument) + 16 * sizeof(MTRKsample*));

    if ( ((error = memAlloc(96, (void**) &inst->noteSamples)) != OK) ||
         ((error = memAlloc(sizeof(MTRKenvelope), (void**) &inst->volEnvelope)) != OK) ||
         ((error = memAlloc(sizeof(MTRKenvelope), (void**) &inst->panEnvelope)) != OK) )
    {
        mLastError = error;
        M_UNLOCK
        return FALSE;
    }

    memset((void*) inst->noteSamples, 0, 96);

    inst->volEnvelope->numPoints = 0;
    inst->volEnvelope->sustain = -1;
    inst->volEnvelope->loopStart = inst->volEnvelope->loopEnd = -1;
    inst->volEnvelope->susLoopStart = inst->volEnvelope->susLoopEnd = -1;
    memset((void*) inst->volEnvelope->points, 0,
           25 * sizeof(MTRKenvelopePoint));

    inst->panEnvelope->numPoints = 0;
    inst->panEnvelope->sustain = -1;
    inst->panEnvelope->loopStart = inst->panEnvelope->loopEnd = -1;
    inst->panEnvelope->susLoopStart = inst->panEnvelope->susLoopEnd = -1;
    memset((void*) inst->panEnvelope->points, 0,
           25 * sizeof(MTRKenvelopePoint));

    inst->volume = 64;
    inst->used = 1;

    module->instruments[instrument] = inst;

    M_UNLOCK
    return TRUE;
}



/****************************************************************************\
*
* Function:     BOOL MTRKnewSample(MTRKmodule *module, unsigned instrument,
*                   unsigned sample, MIDASmodulePlayHandle playHandle)
*
* Description:  Allocates a new sample structure and fills it with
*               reasonable default values
*
* Input:        MTRKmodule *module      the module
*               unsigned instrument     instrument number
*               unsigned sample         sample number in the instrument
*               MIDASmodulePlayHandle playHandle  playing handle that is used
*                                       for playing the module, 0 if none
*
* Returns:      TRUE if successful, FALSE if not
*
\****************************************************************************/

_FUNC(BOOL) MTRKnewSample(MTRKmodule *module, unsigned instrument,
                          unsigned sample, MIDASmodulePlayHandle playHandle)
{
    int error;
    MTRKinstrument *inst;
    MTRKsample *smp;
    
    M_LOCK

    inst = module->instruments[instrument];
    if ( inst == NULL )
    {
        M_UNLOCK
        return TRUE;
    }

    if ( playHandle )
        StopInstrument(playHandle, instrument);

    if ( inst->samples[sample] != NULL )
        MTRKremoveSample(module, instrument, sample, playHandle);

    if ( (error = memAlloc(sizeof(MTRKsample), (void**) &smp)) != OK )
    {
        mLastError = error;
        M_UNLOCK
        return FALSE;
    }

    memset((void*) smp, 0, sizeof(MTRKsample));

    inst->samples[sample] = smp;

    smp->volume = 64;
    smp->gvolume = 64;

    M_UNLOCK
    return TRUE;
}




/****************************************************************************\
*
* Function:     BOOL MTRKsetPlayTempo(MIDASmodulePlayHandle playHandle,
*                   unsigned tempo)
*
* Description:  Sets module playback tempo
*
* Input:        MIDASmodulePlayHandle playHandle  module playback handle
*               unsigned tempo          new tempo
*
* Returns:      TRUE if successful, FALSE if not
*
\****************************************************************************/

_FUNC(BOOL) MTRKsetPlayTempo(MIDASmodulePlayHandle playHandle, unsigned tempo)
{
    int error;

    playHandle = playHandle;

    M_LOCK

    if ( (error = gmpSetTempo(tempo)) != OK )
    {
        mLastError = error;
        M_UNLOCK
        return FALSE;
    }

    M_UNLOCK
    return TRUE;
}


/****************************************************************************\
*
* Function:     BOOL MTRKsetPlaySpeed(MIDASmodulePlayHandle playHandle,
*                   unsigned Speed)
*
* Description:  Sets module playback speed
*
* Input:        MIDASmodulePlayHandle playHandle  module playback handle
*               unsigned speed          new speed
*
* Returns:      TRUE if successful, FALSE if not
*
\****************************************************************************/

_FUNC(BOOL) MTRKsetPlaySpeed(MIDASmodulePlayHandle playHandle, unsigned speed)
{
    gmpPlayHandle h = mPlayHandles[playHandle-1]->playHandle;
    h->speed = speed;
    return TRUE;
}


/****************************************************************************\
*
* Function:     BOOL MTRKgetPlayTempo(MIDASmodulePlayHandle playHandle)
*
* Description:  Gets module playback tempo
*
* Input:        MIDASmodulePlayHandle playHandle  module playback handle
*
* Returns:      Current playback tempo
*
\****************************************************************************/

_FUNC(unsigned) MTRKgetPlayTempo(MIDASmodulePlayHandle playHandle)
{
    return gmpTempo;
}


/****************************************************************************\
*
* Function:     BOOL MTRKgetPlaySpeed(MIDASmodulePlayHandle playHandle)
*
* Description:  Gets module playback speed
*
* Input:        MIDASmodulePlayHandle playHandle  module playback handle
*
* Returns:      Current playback speed
*
\****************************************************************************/

_FUNC(unsigned) MTRKgetPlaySpeed(MIDASmodulePlayHandle playHandle)
{
    return mPlayHandles[playHandle-1]->playHandle->speed;
}





/*
 * $Log: midastrk.c,v $
 * Revision 1.5.2.3  1998/01/01 18:03:35  pekangas
 * MTRKdecompressPattern() now ignores data on channels that don't
 * really exist
 *
 * Revision 1.5.2.2  1997/11/20 20:43:03  pekangas
 * Added speed and tempo functions
 *
 * Revision 1.5.2.1  1997/11/18 19:36:22  pekangas
 * Added functions for changing sample data, and adding and removing samples
 * and instruments
 *
 * Revision 1.5  1997/08/13 18:54:43  pekangas
 * Fixed to compile with Visual C
 *
 * Revision 1.4  1997/08/13 18:27:16  pekangas
 * Fixed to compile to DLL again
 *
 * Revision 1.3  1997/08/13 17:35:33  pekangas
 * Updated for 1.1, added instrument playing code and memAlloc/Free wrappers
 *
 * Revision 1.2  1997/07/31 10:56:50  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.1  1997/05/20 09:39:11  pekangas
 * Initial revision
 *
*/
