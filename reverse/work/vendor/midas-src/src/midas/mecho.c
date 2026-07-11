/*      mecho.c
 *
 * MIDAS echo effect engine
 *
 * $Id: mecho.c,v 1.7 1997/07/31 10:56:45 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include <assert.h>
#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#include "mmem.h"
#include "sdevice.h"
#include "dsm.h"
#include "mecho.h"


#define MAXRESULT 10435760.0f

typedef struct
{
    unsigned leftOffset, rightOffset;
    float leftPrev, rightPrev;
    float gain;
    float prevGain;
} mEchoFloatEcho;


typedef struct
{
    float feedback;
    float gain;
    unsigned numEchoes;
    float *buffer;
    unsigned bufferLengthMask;
    unsigned position;
    mEchoFloatEcho echoes[EMPTYARRAY];
} mEchoFloatEchoSet;



typedef struct _mEchoEffect
{
    struct _mEchoEffect *next, *prev;
    dsmPostProcessor postProc;
    union
    {
        mEchoFloatEchoSet floatSet;
    } echoSet;
} mEchoEffect;


static int mEchoInitialized=0;
static unsigned mEchoRate;
static int mEchoStereo;
static mEchoEffect *mEchoEffectList;



/****************************************************************************\
*
* Function:     int mEchoInit(void)
*
* Description:  Initializes the echo effect engine. MUST be called AFTER
*               dsmInit().
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mEchoInit(void)
{
    int error;

    /* The echo processing will be done in floating point, even if the
       mixing buffer is integer */

    mEchoRate = dsmMixRate;

    if ( dsmMode & dsmMixStereo )
        mEchoStereo = 1;
    else
        mEchoStereo = 0;

    /* Initialize the echo effect list: */
    if ( (error = memAlloc(sizeof(mEchoEffect),
                           (void**) &mEchoEffectList)) != OK )
        PASSERROR(ID_mEchoInit);
    mEchoEffectList->next = mEchoEffectList->prev = mEchoEffectList;

    mEchoInitialized = 1;
    
    return OK;
}



/****************************************************************************\
*
* Function:     int mEchoClose(void)
*
* Description:  Uninitializes the echo effect engine
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mEchoClose(void)
{
    int error;
    mEchoEffect *effect, *e;

    if ( !mEchoInitialized )
        return OK;

    mEchoInitialized = 0;
    
    /* Remove all remaining effects: */
    effect = mEchoEffectList->next;
    while ( effect != mEchoEffectList )
    {
        assert(effect != effect->next);
        e = effect->next;
        if ( (error = mEchoRemoveEffect((mEchoHandle) effect)) != OK )
            PASSERROR(ID_mEchoClose);
        effect = e;
    }

    if ( (error = memFree(mEchoEffectList)) != OK )
        PASSERROR(ID_mEchoClose);

    return OK;
}


/* A more efficient float->int conversion: */

#ifdef __WC32__

static int tempInt;

void StoreFloatInt(float f, int *dest);
#pragma aux StoreFloatInt = \
    "fistp tempInt" \
    "mov eax,tempInt" \
    "mov [edi],eax" \
    parm [8087] [edi] \
    modify exact [8087 eax];

#else
#ifdef __VC32__

static int tempInt;

__inline void StoreFloatInt(float f, int *dest)
{
    __asm
        {
            mov eax,dest
            fld f
            fistp tempInt
            mov edx,tempInt
            mov [eax],edx
        }
}

#else

/* FIXME: GCC version! */

void StoreFloatInt(float f, int *dest)
{
    *dest = (int) f;
}

#endif
#endif



void CALLING mEchoIntMono(void *data, unsigned numSamples, void *user)
{
    mEchoEffect *effect = (mEchoEffect*) user;
    int *d = (int*) data;
    mEchoFloatEchoSet *floatSet = &effect->echoSet.floatSet;
    float *buffer = floatSet->buffer;
    unsigned position = floatSet->position;
    unsigned bufferMask = floatSet->bufferLengthMask;
    float feedback = floatSet->feedback;
    float gain = floatSet->gain;
    float sample;
    float s;
    mEchoFloatEcho *echo;
    unsigned n;

    while ( numSamples )
    {
        sample = (float) *d;

        buffer[position] = sample;

        echo = floatSet->echoes;
        n = floatSet->numEchoes;

        while ( n )
        {
            s = buffer[(position + echo->leftOffset) & bufferMask];
            sample += echo->gain * s + echo->prevGain * echo->leftPrev;
            echo->leftPrev = s;
            
            n--;
            echo++;
        }

        if ( sample > MAXRESULT )
            sample = MAXRESULT;
        if ( sample < -MAXRESULT )
            sample = -MAXRESULT;

        StoreFloatInt((gain * sample), d++);

        buffer[position] += feedback * sample;

        position = (position + 1) & bufferMask;

        numSamples--;
    }

    floatSet->position = position;
}



void CALLING mEchoIntStereo(void *data, unsigned numSamples, void *user)
{
    mEchoEffect *effect = (mEchoEffect*) user;
    int *d = (int*) data;
    mEchoFloatEchoSet *floatSet = &effect->echoSet.floatSet;
    float *buffer = floatSet->buffer;
    unsigned position = floatSet->position;
    unsigned bufferMask = floatSet->bufferLengthMask;
    float feedback = floatSet->feedback;
    float gain = floatSet->gain;
    float lsample, rsample;
    float s;
    mEchoFloatEcho *echo;
    unsigned n;

    while ( numSamples )
    {
        lsample = (float) *d;
        rsample = (float) *(d+1);

        buffer[position] = lsample;
        buffer[position+1] = rsample;

        echo = floatSet->echoes;
        n = floatSet->numEchoes;

        while ( n )
        {
            s = buffer[(position + echo->leftOffset) & bufferMask];
            lsample += echo->gain * s + echo->prevGain * echo->leftPrev;
            echo->leftPrev = s;
            
            s = buffer[(position + echo->rightOffset) & bufferMask];
            rsample += echo->gain * s + echo->prevGain * echo->rightPrev;
            echo->rightPrev = s;
            
            n--;
            echo++;
        }

        if ( lsample > MAXRESULT )
            lsample = MAXRESULT;
        if ( lsample < -MAXRESULT )
            lsample = -MAXRESULT;
        if ( rsample > MAXRESULT )
            rsample = MAXRESULT;
        if ( rsample < -MAXRESULT )
            rsample = -MAXRESULT;                

        StoreFloatInt((gain * lsample), d++);
        StoreFloatInt((gain * rsample), d++);

        buffer[position] += feedback * lsample;
        buffer[position+1] += feedback * rsample;

        position = (position + 2) & bufferMask;

        numSamples--;
    }

    floatSet->position = position;
}



void CALLING mEchoFloatMono(void *data, unsigned numSamples, void *user)
{
    mEchoEffect *effect = (mEchoEffect*) user;
    float *d = (float*) data;
    mEchoFloatEchoSet *floatSet = &effect->echoSet.floatSet;
    float *buffer = floatSet->buffer;
    unsigned position = floatSet->position;
    unsigned bufferMask = floatSet->bufferLengthMask;
    float feedback = floatSet->feedback;
    float gain = floatSet->gain;
    float sample;
    float s;
    mEchoFloatEcho *echo;
    unsigned n;

    while ( numSamples )
    {
        sample = *d;

        buffer[position] = sample;

        echo = floatSet->echoes;
        n = floatSet->numEchoes;

        while ( n )
        {
            s = buffer[(position + echo->leftOffset) & bufferMask];
            sample += echo->gain * s + echo->prevGain * echo->leftPrev;
            echo->leftPrev = s;
            
            n--;
            echo++;
        }

        if ( sample > MAXRESULT )
            sample = MAXRESULT;
        if ( sample < -MAXRESULT )
            sample = -MAXRESULT;
        
        *d++ = gain * sample;

        buffer[position] += feedback * sample;

        position = (position + 1) & bufferMask;

        numSamples--;
    }

    floatSet->position = position;
}



void CALLING mEchoFloatStereo(void *data, unsigned numSamples, void *user)
{
    mEchoEffect *effect = (mEchoEffect*) user;
    float *d = (float*) data;
    mEchoFloatEchoSet *floatSet = &effect->echoSet.floatSet;
    float *buffer = floatSet->buffer;
    unsigned position = floatSet->position;
    unsigned bufferMask = floatSet->bufferLengthMask;
    float feedback = floatSet->feedback;
    float gain = floatSet->gain;
    float lsample, rsample;
    float s;
    mEchoFloatEcho *echo;
    unsigned n;

    while ( numSamples )
    {
        lsample = *d;
        rsample = *(d+1);

        buffer[position] = lsample;
        buffer[position+1] = rsample;

        echo = floatSet->echoes;
        n = floatSet->numEchoes;

        while ( n )
        {
            s = buffer[(position + echo->leftOffset) & bufferMask];
            lsample += echo->gain * s + echo->prevGain * echo->leftPrev;
            echo->leftPrev = s;
            
            s = buffer[(position + echo->rightOffset) & bufferMask];
            rsample += echo->gain * s + echo->prevGain * echo->rightPrev;
            echo->rightPrev = s;
            
            n--;
            echo++;
        }

        if ( lsample > MAXRESULT )
            lsample = MAXRESULT;
        if ( lsample < -MAXRESULT )
            lsample = -MAXRESULT;
        if ( rsample > MAXRESULT )
            rsample = MAXRESULT;
        if ( rsample < -MAXRESULT )
            rsample = -MAXRESULT;                

        *d++ = gain * lsample;
        *d++ = gain * rsample;

        buffer[position] += feedback * lsample;
        buffer[position+1] += feedback * rsample;

        position = (position + 2) & bufferMask;

        numSamples--;
    }

    floatSet->position = position;
}



/****************************************************************************\
*
* Function:     int mEchoAddEffect(mEchoSet *echoSet, mEchoHandle *handle)
*
* Description:  Adds an echo effect to the output, to the beginning of
*               the post-processor chain.
*
* Input:        mEchoSet *echoSet       echo set that describes the effect
*               mEchoHandle *handle     pointer to destination echo handle
*                                       for the effect
*
* Returns:      MIDAS error code. An echo handle for the effect is written
*               to *handle.
*
\****************************************************************************/

int CALLING mEchoAddEffect(mEchoSet *echoSet, mEchoHandle *handle)
{
    unsigned samples2n;
    int error;
    mEchoEffect *effect;
    mEchoFloatEchoSet *floatSet;
    unsigned maxDelay;
    unsigned i, n;
    unsigned sampleDelay;
    float gain, prevGain;
    
    if ( !mEchoInitialized )
        return OK;

    /* Allocate memory for the echo effect structure: */
    if ( (error = memAlloc(sizeof(mEchoEffect) +
                           echoSet->numEchoes * sizeof(mEchoFloatEcho),
                           (void**) &effect)) != OK )
         PASSERROR(ID_mEchoAddEffect);
    
    maxDelay = 0;

    /* Convert the echo set information: */
    floatSet = &effect->echoSet.floatSet;
    floatSet->feedback = ((float) echoSet->feedback) / 65536.0f;
    floatSet->gain = ((float) echoSet->gain) / 65536.0f;
    floatSet->numEchoes = echoSet->numEchoes;
    for ( i = 0; i < floatSet->numEchoes; i++ )
    {
        sampleDelay = (echoSet->echoes[i].delay/1000) * mEchoRate / 65536;
        if ( maxDelay < sampleDelay )
            maxDelay = sampleDelay;
        
        if ( mEchoStereo )
        {
            if ( echoSet->echoes[i].reverseChannels )
            {
                floatSet->echoes[i].leftOffset = 2*sampleDelay + 1;
                floatSet->echoes[i].rightOffset = 2*sampleDelay;
            }
            else
            {
                floatSet->echoes[i].leftOffset = 2*sampleDelay;
                floatSet->echoes[i].rightOffset = 2*sampleDelay + 1;
            }
            floatSet->echoes[i].leftPrev = 0.0f;
            floatSet->echoes[i].rightPrev = 0.0f;
        }
        else
        {         
            floatSet->echoes[i].leftOffset = sampleDelay;
            floatSet->echoes[i].leftPrev = 0.0f;
        }

        switch ( echoSet->echoes[i].filterType )
        {
            case mEchoFilterLowPass:
                gain = ((float) echoSet->echoes[i].gain) / 65536.0f / 2.0f;
                prevGain = gain;
                break;

            case mEchoFilterHighPass:
                gain = ((float) echoSet->echoes[i].gain) / 65536.0f / 2.0f;
                prevGain = -gain;
                break;

            default:
                gain = ((float) echoSet->echoes[i].gain) / 65536.0f;
                prevGain = 0.0f;
                break;
        }

        floatSet->echoes[i].gain = gain;
        floatSet->echoes[i].prevGain = prevGain;
    }

    if ( mEchoStereo )
        maxDelay = maxDelay * 2;
    maxDelay = maxDelay + 16;
    
    /* Calculate the next nearest number of samples == 2^n that fits the
       maximum delay: */
    samples2n = 1;
    while ( samples2n < maxDelay )
        samples2n <<= 1;

    floatSet->bufferLengthMask = samples2n - 1;

    /* Allocate and clear the buffer: */
    if ( (error = memAlloc(samples2n * sizeof(float),
                           (void**) &floatSet->buffer)) != OK )
        PASSERROR(ID_mEchoInit);
    for ( n = 0; n < samples2n; n++ )
        floatSet->buffer[n] = 0.0f;

    floatSet->position = 0;

    /* Add the effect to the effect list: */
    effect->prev = mEchoEffectList;
    effect->next = mEchoEffectList->next;
    mEchoEffectList->next->prev = effect;
    mEchoEffectList->next = effect;

    /* And add it as a DSM post-processor: */
    effect->postProc.floatMono = &mEchoFloatMono;
    effect->postProc.floatStereo = &mEchoFloatStereo;
    effect->postProc.intMono = &mEchoIntMono;
    effect->postProc.intStereo = &mEchoIntStereo;
    if ( (error = dsmAddPostProcessor(&effect->postProc, dsmPostProcFirst,
                                      (void*) effect)) != OK )
        PASSERROR(ID_mEchoInit);

    /* We're done! */
    *handle = (mEchoHandle) effect;

    return OK;
}




/****************************************************************************\
*
* Function:     int mEchoRemoveEffect(mEchoHandle handle)
*
* Description:  Removes an echo effect from the output
*
* Input:        mEchoHandle handle      echo handle for the effect
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mEchoRemoveEffect(mEchoHandle *handle)
{
    int error;
    mEchoEffect *effect = (mEchoEffect*) handle;

    if ( !mEchoInitialized )
        return OK;

    /* Remove the effect from DSM: */
    if ( (error = dsmRemovePostProcessor(&effect->postProc)) != OK )
        PASSERROR(ID_mEchoRemoveEffect);

    /* Remove it from the effect list: */
    effect->prev->next = effect->next;
    effect->next->prev = effect->prev;

    /* Dellocate the buffer and the effect structure: */
    if ( (error = memFree(effect->echoSet.floatSet.buffer)) != OK )
        PASSERROR(ID_mEchoRemoveEffect);
    if ( (error = memFree(effect)) != OK )
        PASSERROR(ID_mEchoRemoveEffect);

    return OK;
}






/*
 * $Log: mecho.c,v $
 * Revision 1.7  1997/07/31 10:56:45  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.6  1997/07/26 14:13:11  pekangas
 * Added support for echoing integer output
 *
 * Revision 1.5  1997/07/15 12:18:55  pekangas
 * Fixed to compile and link with Visual C.
 * All API functions are again exported properly
 *
 * Revision 1.4  1997/07/12 11:38:52  pekangas
 * Added clipping to prevent overflows and extreme slowdown
 *
 * Revision 1.3  1997/07/11 13:35:16  pekangas
 * Added mono echo routines
 *
 * Revision 1.2  1997/07/11 11:05:53  pekangas
 * Added new options to the echoes: Each echo now has its own filter setting
 * (low/high-pass or nothing) and the whole echo set has a common gain.
 *
 * Revision 1.1  1997/07/10 18:39:58  pekangas
 * Initial revision
 *
*/
