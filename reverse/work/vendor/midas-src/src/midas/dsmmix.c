/*      dsmmix.c
 *
 * Digital Sound Mixer, mixing routine framework
 *
 * $Id: dsmmix.c,v 1.20.2.3 1998/02/01 14:43:42 pekangas Exp $
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
#include <stdlib.h>
#include <string.h>

#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#include "sdevice.h"
#include "dsm.h"

RCSID(const char *dsmmix_rcsid = "$Id: dsmmix.c,v 1.20.2.3 1998/02/01 14:43:42 pekangas Exp $";)

/* Global variables for mixing routine state information: */
int dsmMixSrcPos;
uchar *dsmMixDest;
int dsmMixLeftVol, dsmMixRightVol;
float dsmMixLeftVolFloat, dsmMixRightVolFloat;
int dsmMixStep;
uchar *dsmMixSample;
int dsmMixLeftVolInt, dsmMixRightVolInt;
int* dsmMixLeftVolTable;
int* dsmMixRightVolTable;
int* dsmMixLeftUlawVolTable;
int* dsmMixRightUlawVolTable;


static int nextSampleOffset;
static unsigned oldPlayPos;


static unsigned SamplesUntilEnd(dsmChannel *chan)
{
    unsigned num;
    unsigned lpStart, lpEnd;
    int lpType;

    oldPlayPos = chan->playPos;
    nextSampleOffset = 1;

    /* Return a maximum of 32k-1 samples to prevent overflow problems */

    if ( chan->inTranslation )
    {
        num = chan->numTransSamples - chan->transPos;
        if ( num > 0x7FFF )
            return 0x7FFF0000;
        return ((num << 16) - chan->playPosLow);
    }

    if ( chan->loopNum == 2 )
    {
        lpStart = chan->loop2Start;
        lpEnd = chan->loop2End;
        lpType = chan->loop2Type;
    }
    else
    {
        lpStart = chan->loop1Start;
        lpEnd = chan->loop1End;
        lpType = chan->loop1Type;
    }

    assert((lpType == loopNone) || (lpType == loopUnidir) ||
           (lpType == loopBidir));
    assert((lpType == loopNone) || (lpStart < lpEnd));

    /* When using interpolating mixing, we'll first mix everything normally
       until the very last sample of the loop/sample/stream. Then the last
       sample will be mixed separately, setting nextSampleOffset to a
       correct value, to make sure we won't interpolate past the end.
       This doesn't make this code exactly pretty, but saves us from quite a
       bit of headache elsewhere. */

    /* Check if we'd reach the stream write position before loop end: */
    if ( (chan->sampleHandle == DSM_SMP_STREAM) &&
         (chan->streamWritePos >= chan->playPos) &&
         (chan->streamWritePos <= lpEnd) )
    {
        if ( chan->playPos >= chan->streamWritePos )
            return 0;
        if ( dsmMode & dsmMixInterpolation )
        {
            if ( (chan->playPos + 1) < chan->streamWritePos )
            {
                num = (chan->streamWritePos - 1) - chan->playPos;
            }
            else
            {
                /* The last sample of the stream - don't interpolate past the
                   stream write position: */
                nextSampleOffset = 0;
                num = chan->streamWritePos - chan->playPos;
            }
        }
        else
        {
            num = chan->streamWritePos - chan->playPos;
        }
        if ( num > 0x7FFF )
            return 0x7FFF0000;
        return ((num << 16) - chan->playPosLow);        
    }    

    switch ( lpType )
    {
        case loopNone:
            if ( dsmMode & dsmMixInterpolation )
            {
                if ( (chan->playPos + 1) < chan->sampleLength )
                {
                    num = (chan->sampleLength - 1) - chan->playPos;
                }
                else
                {
                    /* The last sample */
                    nextSampleOffset = 0;
                    num = chan->sampleLength - chan->playPos;
                }
            }
            else
            {
                num = chan->sampleLength - chan->playPos;
            }
            if ( num > 0x7FFF )
                return 0x7FFF0000;
            return ((num << 16) - chan->playPosLow);

        case loopUnidir:
            if ( dsmMode & dsmMixInterpolation )
            {
                if ( (chan->playPos + 1) < lpEnd )
                {
                    num = (lpEnd - 1) - chan->playPos;
                }
                else
                {
                    /* The last sample of the loop */
                    nextSampleOffset = lpStart - chan->playPos;
                    num = lpEnd - chan->playPos;
                }
            }
            else
            {
                num = lpEnd - chan->playPos;
            }
            if ( num > 0x7FFF )
                return 0x7FFF0000;
            return ((num << 16) - chan->playPosLow);
            
            
        case loopBidir:
            if ( chan->direction == -1 )
            {
                /* Backwards */
                if ( dsmMode & dsmMixInterpolation )
                {
                    if ( chan->playPos == (lpEnd-1) )
                    {
                        /* First sample of the loop backwards */
                        nextSampleOffset = 0;
                        num = 1;
                    }
                    else
                    {                        
                        num = chan->playPos - lpStart;
                    }                       
                }
                else
                {
                    num = chan->playPos - lpStart;
                }
                if ( num >= 0x7FFF )
                    return 0x7FFF0000;
                return ((num << 16) + chan->playPosLow);
            }
            else
            {
                /* Forward */
                if ( dsmMode & dsmMixInterpolation )
                {
                    if ( (chan->playPos + 1) < lpEnd )
                    {
                        num = (lpEnd - 1) - chan->playPos;
                    }
                    else
                    {
                        /* The last sample of the loop */
                        nextSampleOffset = 0;
                        num = lpEnd - chan->playPos;
                    }
                }
                else
                {
                    num = lpEnd - chan->playPos;
                }
                if ( num > 0x7FFF )
                    return 0x7FFF0000;
                return ((num << 16) - chan->playPosLow);
            }
    }

    return 0;
}



static int AmigaChange(dsmChannel *chan, unsigned chanNum)
{
    /* A sample end or sample loop end has been reached, the sample
       has been changed, and both old and new samples use Amiga compatible
       looping - handle Amiga Loop Emulation sample change: */
    dsmChangeSample(chanNum);

    if ( chan->loopMode == sdLoopAmiga )
    {
        /* Looping - start playback from loop beginning */
        chan->playPos = chan->loop1Start;
        chan->playPosLow = 0;
        return 0;
    }

    /* Not looping - finish the sample: */
    chan->status = dsmChanEnd;
    return 1;
}



static int HandleEnd(dsmChannel *chan, unsigned chanNum)
{
    unsigned lpStart, lpEnd;
    int lpType;
    int n;

    if ( chan->inTranslation )
    {
        if ( chan->transPos >= chan->numTransSamples )
        {
            chan->inTranslation = 0;
            return 0;
        }
        return 1;
    }

    if ( chan->loopNum == 2 )
    {
        lpStart = chan->loop2Start;
        lpEnd = chan->loop2End;
        lpType = chan->loop2Type;
    }
    else
    {
        lpStart = chan->loop1Start;
        lpEnd = chan->loop1End;
        lpType = chan->loop1Type;
    }

    /* First, check if we reached a stream write position that is exactly
       at the stream buffer end. If so, playback needs to stop here and not
       wrap to loop start, and thus the test needs to be done here. */
    if ( (chan->sampleHandle == DSM_SMP_STREAM) &&
         (chan->playPos >= chan->streamWritePos) &&
         (chan->streamWritePos == lpEnd) )
        return 1;
    
    if ( lpType == loopNone )
    {
        /* No loop - did we reach sample end? */
        if ( chan->playPos >= chan->sampleLength )
        {
            assert(chan->sampleHandle > 0);
            
            /* Check for ALE sample change: */
            if ( (chan->sampleChanged) &&
                 ((chan->loopMode == sdLoopAmiga) ||
                  (chan->loopMode == sdLoopAmigaNone)) &&
                 ((dsmSamples[chan->sampleHandle-1].loopMode == sdLoopAmiga) ||
                  (dsmSamples[chan->sampleHandle-1].loopMode ==
                   sdLoopAmigaNone)) )
                return AmigaChange(chan, chanNum);

            /* No sample change - we are finished: */
            chan->status = dsmChanEnd;
            return 1;
        }
        else
            return 0;
    }

    if ( chan->direction == -1 )
    {
        /* Going backwards - did we reach loop start? (signed comparison
           takes care of possible wraparound) */
        if ( (((int) chan->playPos) < ((int) lpStart)) ||
             ((chan->playPos == lpStart) && (chan->playPosLow == 0)) )
        {
            if ( chan->LoopCallback != NULL )
                chan->LoopCallback(chanNum);

            chan->direction = 1;
            n = ((((int) lpStart) - ((int) chan->playPos)) << 16) -
                chan->playPosLow - 1;
            /* -1 is compensation for the fudge factor at loop end, see
               below */
            chan->playPos = (unsigned) ((int) lpStart) + (n >> 16);
            chan->playPosLow = n & 0xFFFF;

            /* Don't die on overshort loops: */
            if ( chan->playPos >= lpEnd )
            {
                chan->playPos = lpStart;
                return 1;
            }
        }
        return 0;
    }

    /* Going forward - did we reach loop end? */
    if ( chan->playPos >= lpEnd )
    {
        if ( chan->LoopCallback != NULL )
            chan->LoopCallback(chanNum);

        assert(chan->sampleHandle > 0);

        /* Check for ALE sample change: */
        if ( (chan->sampleChanged) &&
             ((chan->loopMode == sdLoopAmiga) ||
              (chan->loopMode == sdLoopAmigaNone)) &&
             ((dsmSamples[chan->sampleHandle-1].loopMode == sdLoopAmiga) ||
              (dsmSamples[chan->sampleHandle-1].loopMode == sdLoopAmigaNone)) )
            return AmigaChange(chan, chanNum);

        /* Go to the second loop if the sound has been released: */
        if ( (chan->loopNum == 1) && (chan->status == dsmChanReleased) )
        {
            chan->loopNum = 2;
            return 0;
        }

        if ( lpType == loopUnidir )
        {
            /* Unidirectional loop - just loop to the beginning */
            chan->playPos = lpStart + (chan->playPos - lpEnd);
            
            /* Don't die on overshort loops: */
            if ( chan->playPos >= lpEnd )
            {
                chan->playPos = lpStart;
                return 1;
            }
            
            return 0;
        }
        else
        {
            /* Bidirectional loop - change direction */
            chan->direction = -1;
            n = ((chan->playPos - lpEnd) << 16) + chan->playPosLow + 1;
            /* +1 is a fudge factor to make sure we'll access the correct
               samples all the time - a similar adjustment is also done
               at the other end of the loop. This screws up interpolation
               a little when sample rate == mixing rate, but little enough
               that it can't be heard. */

            if ( lpEnd < 0x10000 )
            {
                chan->playPos = ((lpEnd << 16) - n) >> 16;                
                chan->playPosLow = ((lpEnd << 16) - n) & 0xFFFF;
            }
            else
            {
                chan->playPos = ((0xFFFF0000 - n) >> 16) + (lpEnd - 0xFFFF);
                chan->playPosLow = (0xFFFF0000 - n) & 0xFFFF;
            }

            /* Don't die on overshort loops: */
            if ( chan->playPos <= lpStart )
            {
                chan->playPos = lpEnd;
                return 1;
            }
            
            return 0;
        }        
    }

    /* First, check if we reached stream write position: */
    /* Check if we reached stream write position: */
    if ( (chan->sampleHandle == DSM_SMP_STREAM) &&
         (chan->playPos >= chan->streamWritePos) &&
         (oldPlayPos <= chan->streamWritePos) )
        return 1;    

    return 0;
}



static void CalcVolumes(dsmChannel *chan)
{    
    float volScale = ((float) dsmAmplification) /
        (((float) (64.0 * 64.0 * 65536.0)) * ((float) dsmNumChannels));

    if ( dsmMode & dsmMixStereo )
    {
        if ( dsmMode & dsmMixFloat )
        {
            dsmMixLeftVolFloat = volScale * ((float) chan->leftVol);
            dsmMixRightVolFloat = volScale * ((float) chan->rightVol);
        }
        else
        {
            dsmMixLeftVolInt = ((chan->leftVol * dsmAmplification) /
                                dsmNumChannels) / (64 * 64 * 256);
            dsmMixRightVolInt = ((chan->rightVol * dsmAmplification) /
                                 dsmNumChannels) / (64 * 64 * 256);
            dsmMixLeftVolTable = (int*) &dsmVolumeTable[
                256 * (((chan->leftVol >> 16) + VOLADD) >> VOLSHIFT)];
            dsmMixRightVolTable = (int*)&dsmVolumeTable[
                256 * (((chan->rightVol >> 16) + VOLADD) >> VOLSHIFT)];
            dsmMixLeftUlawVolTable = (int*) &dsmUlawVolumeTable[256 *
                (((chan->leftVol >> 16) + VOLADD) >> VOLSHIFT)];
            dsmMixRightUlawVolTable = (int*) &dsmUlawVolumeTable[256 *
                (((chan->rightVol >> 16) + VOLADD) >> VOLSHIFT)];
        }
        if ( chan->panning == panSurround )
        {
            dsmMixRightVolInt = -dsmMixRightVolInt;
            dsmMixRightVolFloat = -dsmMixRightVolFloat;
        }
    }
    else
    {
        /* Mono mixing - if we are playing a stereo sample, halve the
           volume: */
        if ( (chan->sampleType == smp8bitStereo) ||
             (chan->sampleType == smp16bitStereo) ||
             (chan->sampleType == smpUlawStereo) )
        {
            if ( dsmMode & dsmMixFloat )
            {
                dsmMixLeftVolFloat = 0.5f * volScale * ((float) chan->leftVol);
            }
            else
            {
                dsmMixLeftVolInt = (((chan->leftVol / 2) * dsmAmplification) /
                                    dsmNumChannels) / (64 * 64 * 256);
                dsmMixLeftVolTable = (int*)
                    &dsmVolumeTable[256 * ((((chan->leftVol / 2) >> 16) +
                                            VOLADD) >>(VOLSHIFT))];
                dsmMixLeftUlawVolTable = (int*)
                    &dsmUlawVolumeTable[256 * ((((chan->leftVol / 2) >> 16) +
                                                VOLADD) >> (VOLSHIFT))];
            }
        }
        else
        {
            if ( dsmMode & dsmMixFloat )
            {
                dsmMixLeftVolFloat = volScale * ((float) chan->leftVol);
            }
            else
            {
                dsmMixLeftVolInt = ((chan->leftVol * dsmAmplification) /
                                    dsmNumChannels) / (64 * 64 * 256);
                dsmMixLeftVolTable = (int*) &dsmVolumeTable[256 *
                    (((chan->leftVol + VOLADD) >> 16) >> VOLSHIFT)];
                dsmMixLeftUlawVolTable = (int*) &dsmUlawVolumeTable[256 *
                    (((chan->leftVol + VOLADD) >> 16) >> VOLSHIFT)];
            }
        }
    }
}



static void RampVolume(dsmChannel *chan)
{
    int continueRamp = 0;
    
    if ( chan->leftVolTarget > chan->leftVol )
    {
        if ( (chan->leftVolTarget - chan->leftVol) > chan->leftRampSpeed )
        {
            chan->leftVol += chan->leftRampSpeed;
            continueRamp = 1;
        }
        else
            chan->leftVol = chan->leftVolTarget;
    }
    if ( chan->leftVolTarget < chan->leftVol )
    {
        if ( (chan->leftVol - chan->leftVolTarget) > chan->leftRampSpeed )
        {
            chan->leftVol -= chan->leftRampSpeed;
            continueRamp = 1;
        }
        else
            chan->leftVol = chan->leftVolTarget;
    }

    if ( chan->rightVolTarget > chan->rightVol )
    {
        if ( (chan->rightVolTarget - chan->rightVol) > chan->rightRampSpeed )
        {
            chan->rightVol += chan->rightRampSpeed;
            continueRamp = 1;
        }
        else
            chan->rightVol = chan->rightVolTarget;
    }
    if ( chan->rightVolTarget < chan->rightVol )
    {
        if ( (chan->rightVol - chan->rightVolTarget) > chan->rightRampSpeed )
        {
            chan->rightVol -= chan->rightRampSpeed;
            continueRamp = 1;
        }
        else
            chan->rightVol = chan->rightVolTarget;
    }

    chan->volumeRamping = continueRamp;
}





int CALLING dsmMixChannel(unsigned channel, dsmMixRoutineSet *routineSet,
                          unsigned numSamples, void *dest)
{
    dsmChannel *chan;
    int step;
    dsmMixRoutine *mixRoutine;
    unsigned mixNow;
    unsigned mixUntilEnd;
    unsigned samplesUntilEnd;
    unsigned sampleSize;
    unsigned num;
    unsigned prevMix = 1;
        
    chan = &dsmChannels[channel];

    /* Check that we have something: */
    if ( (chan->rate == 0) || (chan->sample == NULL) ||
         (chan->samplePos == sdSmpNone) || (chan->sampleLength == 0) )
        return OK;

    /* Calculate resampling step (16.16 fixed point): */
    step = (((chan->rate / dsmMixRate) << 16) +
            ((chan->rate % dsmMixRate) << 16) / dsmMixRate);    
    assert(step > 0);

    /* Calculate left and right volumes from panning: */
/*    CalcVolumes(chan, volume); */
    
    dsmMixDest = dest;

    while ( numSamples )
    {
        if ( ((chan->status == dsmChanStopped) ||
              (chan->status == dsmChanEnd)) &&
             (!chan->inTranslation) )
            return OK;

        switch ( chan->sampleType )
        {
            case smp8bitStereo:
            case smpUlawStereo:
            case smp16bitMono:
                sampleSize = 2;
                break;

            case smp16bitStereo:
                sampleSize = 4;
                break;

            default:
                sampleSize = 1;
                break;
        }
        
        /* Calculate the number of source samples until the end of loop /
           sample / whatever */
        samplesUntilEnd = SamplesUntilEnd(chan);
        /* samplesUntilEnd is a maximum of 32k - no overflow problems */

        /* Calculate the number of destination samples: (note rounding) */
        mixUntilEnd = samplesUntilEnd / step;
        if ( samplesUntilEnd % step )
            mixUntilEnd++;

        if ( mixUntilEnd > numSamples )
            mixNow = numSamples;
        else
            mixNow = mixUntilEnd;
        numSamples -= mixNow;

        assert((mixNow > 0) || (prevMix > 0));

        /* This should NEVER happen, but better be safe than sorry, otherwise
           we might end up in an infinite loop. */
        if ( (mixNow == 0) && (prevMix == 0) )
            return OK;
        
        prevMix = mixNow;

        if ( mixNow )
        {
            /* Prepare to mix: */
            dsmMixSrcPos = chan->playPosLow & 0xFFFF;
            if ( chan->inTranslation )
            {
                dsmMixSample = chan->transBuffer +
                    sampleSize * chan->transPos;
            }
            else
            {
                dsmMixSample = chan->sample + sampleSize * chan->playPos;
            }

            /* Pick the correct mixing routine: */
            if ( (chan->panning == panMiddle) &&
                 (routineSet->middleRoutines[chan->sampleType].mixLoop
                  != NULL) )
                mixRoutine = &routineSet->middleRoutines[chan->sampleType];
            else
            {
                if ( (chan->panning == panSurround) &&
                     (routineSet->surroundRoutines[chan->sampleType].mixLoop
                      != NULL) )
                    mixRoutine = &routineSet->surroundRoutines[
                        chan->sampleType];
                else
                    mixRoutine = &routineSet->routines[chan->sampleType];
            }
            
            if ( chan->direction == -1 )
                dsmMixStep = -step;
            else
                dsmMixStep = step;

            assert((((unsigned) dsmMixDest) % sampleSize) == 0);
            assert((((unsigned) dsmMixSample) % sampleSize) == 0);
            assert((mixRoutine->mainLoopAlign) &&
                   (mixRoutine->mainLoopRepeat));

            /* Do possible volume ramping: */
            while ( (chan->volumeRamping) && mixNow )
            {
                CalcVolumes(chan);
                mixRoutine->mixLoop(1, nextSampleOffset);
                mixNow--;
                RampVolume(chan);
            }

            if ( (chan->leftVol == 0) && (chan->rightVol == 0) )
            {
                /* The volume is zero - we can just increment the position
                   and not really mix anything: */
                dsmMixSrcPos += mixNow * dsmMixStep;

                if ( chan->status == dsmChanFadeOut )
                {
                    chan->status = dsmChanStopped;
                    chan->volumeRamping = 0;
                }
            }
            else
            {
                assert((mixNow == 0) || (channel < dsmNumChannels));
                
                CalcVolumes(chan);

                /* Ensure proper alignment and do all mixing if
                   nextSampleOffset != 1: */
                if ( mixNow && ((nextSampleOffset != 1) ||
                                ((((unsigned) dsmMixDest)
                                 % mixRoutine->mainLoopAlign) != 0)) )
                {
                    if ( nextSampleOffset != 1 )
                    {
                        num = mixNow;
                    }
                    else
                    {
                        num = mixRoutine->mainLoopAlign -
                            (((unsigned) dsmMixDest) %
                             mixRoutine->mainLoopAlign);
                        if ( num > mixNow )
                            num = mixNow;
                    }
                    mixRoutine->mixLoop(num, nextSampleOffset);
                    mixNow -= num;                    
                }

                /* Do the main mixing loop: */
                if ( (mixNow / mixRoutine->mainLoopRepeat) > 0 )
                {
                    num = mixRoutine->mainLoopRepeat *
                        (mixNow / mixRoutine->mainLoopRepeat);
                    mixRoutine->mainMixLoop(num, 1);
                    mixNow -= num;
                }

                /* And mix what's left: */
                if ( mixNow )
                {
                    mixRoutine->mixLoop(mixNow, 1);
                }
            }

            /* Put changed parts of state back to channel structure: */
            chan->playPosLow = dsmMixSrcPos & 0xFFFF;
            if ( chan->inTranslation )
            {
                chan->transPos = (dsmMixSrcPos >> 16) +
                    ((dsmMixSample - chan->transBuffer) / sampleSize);
            }
            else
            {
                chan->playPos = (dsmMixSrcPos >> 16) +
                    ((dsmMixSample - chan->sample) / sampleSize);
            }
        }

        /* Check if we reached loop/sample/whatever end. If yes, handle it,
           and continue loop if necessary: */
        if ( HandleEnd(chan, channel) )
            return OK;
    }

    return OK;
}



void CALLING dsmClearBufferInt(unsigned numSamples)
{
    if ( dsmMode & dsmMixStereo )
        memset(dsmMixBuffer, 0, 2*numSamples*sizeof(int));
    else
        memset(dsmMixBuffer, 0, numSamples*sizeof(int));
}


void CALLING dsmClearBufferFloat(unsigned numSamples)
{
    unsigned i,n;
    float *buf = (float*) dsmMixBuffer;

    if ( dsmMode & dsmMixStereo )
        n = 2*numSamples;
    else
        n = numSamples;

    for ( i = 0; i < n; i++ )
        buf[i] = (float) 0.0;
}


#ifdef M_X86_ASSEMBLER

void CALLING dsmConvertInt16(unsigned numSamples, S16 *dest);

#else

void CALLING dsmConvertInt16(unsigned numSamples, S16 *dest)
{
    int *buf = (int*) dsmMixBuffer;
    int n;

    while ( numSamples )
    {
        n = *(buf++);
        if ( n < -32768 )
            n = -32768;
        if ( n > 32767 )
            n = 32767;
        *dest++ = (S16) n;
        numSamples--;
    }
}

#endif


void CALLING dsmConvertInt8(unsigned numSamples, U8 *dest)
{
    int *buf = (int*) dsmMixBuffer;
    int n;

    while ( numSamples )
    {
        n = (*(buf++)) >> 8;
        if ( n < -128 )
            n = -128;
        if ( n > 127 )
            n = 127;
        *dest++ = (U8) (128 + n);
        numSamples--;
    }
}



#ifdef M_X86_ASSEMBLER

void CALLING dsmConvertFloat16(unsigned numSamples, S16 *dest);

#else

void CALLING dsmConvertFloat16(unsigned numSamples, S16 *dest)
{
    float *buf = (float*) dsmMixBuffer;
    int n;

    while ( numSamples )
    {
        n = (int) (*(buf++));
        if ( n < -32768 )
            n = -32768;
        if ( n > 32767 )
            n = 32767;
        *dest++ = (S16) n;
        numSamples--;
    }
}

#endif



void CALLING dsmConvertFloat8(unsigned numSamples, U8 *dest)
{
    float *buf = (float*) dsmMixBuffer;
    int n;

    while ( numSamples )
    {
        n = ((int) (*(buf++))) >> 8;
        if ( n < -128 )
            n = -128;
        if ( n > 127 )
            n = 127;
        *dest++ = (U8) (128+n);
        numSamples--;
    }
}




/****************************************************************************\
*
* Function:     int dsmMixData(unsigned numSamples, int mix8bit, void *dest)
*
* Description:  Mixes sound data to *dest
*
* Input:        unsigned numSamples     number of destination samples to mix.
*                                       In stereo mode, a "sample" consists
*                                       really of two samples
*               void *dest              destination buffer
*
* Returns:      MIDAS error code. Mixed data is written to *dest. The output
*               data is 8-bit unsigned bytes for 8-bit output, and 16-bit
*               signed words for 16-bit output.
*
\****************************************************************************/

int CALLING dsmMixData(unsigned numSamples, void *dest)
{
    int         error;
    unsigned    ch;
    dsmChannel  *chan;
    uchar       *mixDest = (uchar*) dest;
    unsigned    doNow;
    unsigned    destSampleSize;
    unsigned    bufSampleSize;
    dsmPostProcFunction ppfunc;
    dsmPostProcessor *pp;

    if ( dsmMode & dsmMix8bit )
        destSampleSize = 1;
    else
        destSampleSize = 2;
    if ( dsmMode & dsmMixStereo )
        destSampleSize *= 2;

    if ( dsmMode & dsmMixInteger )
        bufSampleSize = sizeof(int);
    else
        bufSampleSize = sizeof(float);
    if ( dsmMode & dsmMixStereo )
        bufSampleSize *= 2;

    while ( numSamples )
    {
        /* Check how much to mix on this round: */
        if ( numSamples > (dsmMixBufferSize / bufSampleSize) )
            doNow = dsmMixBufferSize / bufSampleSize;
        else
            doNow = numSamples;

        /* Clear the buffer: */
        if ( dsmMode & dsmMixInteger )
            dsmClearBufferInt(doNow);
        else
            dsmClearBufferFloat(doNow);

        /* Mix all channels to buffer: */
        if ( (!dsmPaused) && (dsmChPlay) )
        {
            for ( ch = 0; ch < dsmAllocatedChannels; ch++ )
            {
                chan = &dsmChannels[ch];
                
                /* Mix for this channel: */
                if ( (error = dsmMixChannel(ch, dsmActMixRoutines, doNow,
                                            dsmMixBuffer)) != OK )
                    PASSERROR(ID_dsmMixData);
            }
        }

        /* Do post-processing: */
        pp = dsmPostProcList->next;
        while ( pp != dsmPostProcList )
        {
            ppfunc = NULL;
            if ( dsmMode & dsmMixInteger )
            {
                if ( dsmMode & dsmMixStereo )
                    ppfunc = pp->intStereo;
                else
                    ppfunc = pp->intMono;
            }
            else
            {
                if ( dsmMode & dsmMixStereo )
                    ppfunc = pp->floatStereo;
                else
                    ppfunc = pp->floatMono;
            }

            if ( ppfunc != NULL )
                (*ppfunc)(dsmMixBuffer, doNow, pp->userData);

            assert(pp != pp->next);
            pp = pp->next;
        }

        /* Convert data to correct output format: */
        if ( dsmMode & dsmMixInteger )
        {
            if ( dsmMode & dsmMix16bit )
            {
                if ( dsmMode & dsmMixStereo )
                    dsmConvertInt16(2*doNow, (S16*) mixDest);
                else
                    dsmConvertInt16(doNow, (S16*) mixDest);
            }
            else
            {
                if ( dsmMode & dsmMixStereo )
                    dsmConvertInt8(2*doNow, (U8*) mixDest);
                else
                    dsmConvertInt8(doNow, (U8*) mixDest);                
            }
        }
        else
        {
            if ( dsmMode & dsmMix16bit )
            {
                if ( dsmMode & dsmMixStereo )
                    dsmConvertFloat16(2*doNow, (S16*) mixDest);
                else
                    dsmConvertFloat16(doNow, (S16*) mixDest);
            }
            else
            {
                if ( dsmMode & dsmMixStereo )
                    dsmConvertFloat8(2*doNow, (U8*) mixDest);
                else
                    dsmConvertFloat8(doNow, (U8*) mixDest);                
            }
        }

        mixDest += doNow * destSampleSize;
        numSamples -= doNow;
    }

    return OK;
}





/*
 * $Log: dsmmix.c,v $
 * Revision 1.20.2.3  1998/02/01 14:43:42  pekangas
 * Fixed >64k bidirectional loops
 *
 * Revision 1.20.2.2  1997/08/22 14:31:54  pekangas
 * Fixed a problem with extremely short loops (length less than the mixing
 * step), and fixed a potential problem with zero-length loops.
 *
 * Revision 1.20.2.1  1997/08/20 12:48:04  pekangas
 * Manually merged fixes from the main trunk (oops)
 *
 * Revision 1.21  1997/08/19 17:51:24  pekangas
 * Fixed bidirectional loops clicking with interpolating mixing
 *
 * Revision 1.20  1997/08/01 18:15:24  jpaana
 * Fixed one signess bug
 *
 * Revision 1.19  1997/07/31 14:29:46  pekangas
 * Fixed a Visual C warning
 *
 * Revision 1.18  1997/07/31 10:56:39  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.17  1997/07/30 16:19:30  pekangas
 * Added optimized 16-bit integer conversion
 *
 * Revision 1.16  1997/07/30 14:18:33  pekangas
 * Added 8-bit conversion routines
 *
 * Revision 1.15  1997/07/24 17:10:01  pekangas
 * Added surround support and support for separate surround and middle mixing
 * routines.
 *
 * Revision 1.14  1997/07/24 13:27:20  pekangas
 * Fixed support for u-law stereo samples
 *
 * Revision 1.13  1997/07/17 09:55:45  pekangas
 * Added support for integer mono mixing
 *
 * Revision 1.12  1997/07/11 13:34:59  pekangas
 * Post-processing functions are now always done, even if no channels are
 * open. This causes reverbs to sound much better.
 *
 * Revision 1.11  1997/07/10 18:40:22  pekangas
 * Added echo effect support
 *
 * Revision 1.10  1997/07/05 14:21:10  pekangas
 * Fixed the clicks for good
 *
 * Revision 1.9  1997/07/03 15:03:46  pekangas
 * Interpolating mixing no longer indexes past the real sample or loop end,
 * thus replicating data for it is not necessary
 *
 * Revision 1.8  1997/06/27 17:28:23  pekangas
 * Added zero-volume optimizations
 *
 * Revision 1.7  1997/06/27 13:28:04  pekangas
 * Fixed handling stream write position when sampling rate is greater than
 * the mixing rate
 *
 * Revision 1.6  1997/06/20 10:09:46  pekangas
 * Added volume ramping
 *
 * Revision 1.5  1997/06/11 14:37:07  pekangas
 * Added support for post-processing routines.
 * Added automatic amplification for dsmOpenChannel().
 * Removed unused outputBits -argument from dsmInit().
 *
 * Revision 1.4  1997/06/11 12:40:59  pekangas
 * Fixed bidirectional loops
 *
 * Revision 1.3  1997/06/05 20:18:46  pekangas
 * Added preliminary support for interpolating mixing (mono only at the
 * moment)
 *
 * Revision 1.2  1997/06/04 15:22:22  pekangas
 * Added an assembler version of the float->int conversion routine
 *
 * Revision 1.1  1997/05/30 18:25:02  pekangas
 * Initial revision
 *
*/
