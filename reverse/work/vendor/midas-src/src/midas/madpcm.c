/*      madpcm.c
 *
 * IMA ADPCM coding/decoding routines
 *
 * $Id: madpcm.c,v 1.4.2.2 1997/08/21 20:27:06 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include <stdio.h>

#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#include "mmem.h"
#include "sdevice.h"
#include "mprof.h"
#include "madpcm.h"


/* Step size adjustments: */
int mAdpcmStepSizeAdj[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };


/* Step sizes: */
static int stepSizes[89] =
{
    7,     8,     9,     10,    11,    12,    13,    14,    16,    17,
    19,    21,    23,    25,    28,    31,    34,    37,    41,    45,
    50,    55,    60,    66,    73,    80,    88,    97,    107,   118,
    130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
    337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
    876,   963,   1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
    2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
    5894,  6484,  7132,  7845,  8630,  9493,  10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};


/* A joint step size / dequantization table: */
extern int mAdpcmPredDiffTable[16*89];


/****************************************************************************\
*
* Function:     int mAdpcmInitState(unsigned numChannels,
*                   unsigned frameLength, mAdpcmState **state)
*
* Description:  Allocates and initializes the ADPCM state structure.
*
* Input:        unsigned numChannels    number of channels that will be used
*               unsigned frameLength    length of one encoded stream frame,
*                                       _including header_, in bytes. Only
*                                       used for encoding.
*               mAdpcmState **state     pointer to destination state pointer
*
* Returns:      MIDAS error code. The initialized ADPCM state structure will
*               be written to *state.
*
\****************************************************************************/

int CALLING mAdpcmInitState(unsigned numChannels, unsigned frameLength,
    mAdpcmState **state)
{
    static mAdpcmState *s;
    int         error;
    unsigned    c;

    /* Allocate the ADPCM state structure: */
    if ( (error = memAlloc(sizeof(mAdpcmState) +
        numChannels * sizeof(mAdpcmChannelState), (void**) &s)) != OK )
        PASSERROR(ID_mAdpcmInitState)

    /* Initialize it: */
    s->numChannels = numChannels;
    for ( c = 0; c < numChannels; c++ )
    {
        s->channels[c].predValue = 0;
        s->channels[c].stepSizeIndex = 0;
    }

    s->frameBytes = 0;
    s->headerBytes = 0;
    s->frameLength = frameLength;

    /* Allocate space for stream frame header: */
    s->headerSize = sizeof(mStreamFrameHeader) +
        numChannels * sizeof(mAdpcmFrameChannelHeader);
    if ( (error = memAlloc(s->headerSize, (void**) &s->header)) != OK )
        PASSERROR(ID_mAdpcmInitState)

    s->headerPtr = (U8*) s->header;
    s->chanHeaders = (mAdpcmFrameChannelHeader*)
        (s->headerPtr + sizeof(mStreamFrameHeader));

    *state = s;
    return OK;
}




/****************************************************************************\
*
* Function:     int mAdpcmFreeState(mAdpcmState *state)
*
* Description:  Deallocates an ADPCM state structure
*
* Input:        mAdpcmState *state      the state structure
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mAdpcmFreeState(mAdpcmState *state)
{
    int         error;

    /* Deallocate frame header: */
    if ( (error = memFree(state->header)) != OK )
        PASSERROR(ID_mAdpcmFreeState)

    /* Just deallocate the memory: */
    if ( (error = memFree(state)) != OK )
        PASSERROR(ID_mAdpcmFreeState)

    return OK;
}




/****************************************************************************\
*
* Function:     int mAdpcmEncode(short *input, unsigned inputLength,
*                   uchar *output, unsigned outputBufferLength,
*                   unsigned *inputUsed, unsigned *outputWritten,
*                   mAdpcmState *state)
*
* Description:  Encodes raw 16-bit signed audio data to 4-bit ADPCM
*
* Input:        short *input            pointer to input data
*               unsigned inputLength    length of input data, in bytes. MUST
*                                       contain an even number of samples
*                                       unless encoding the final block of a
*                                       stream.
*               uchar *output           pointer to destination data
*               unsigned outputBufferLength  output buffer length, in bytes
*               unsigned *inputUsed     pointer to the variable that will get
*                                       the number of input bytes used
*               unsigned *outputWritten pointer to the variable that will get
*                                       the number of output bytes written
*               mAdpcmState *state      ADPCM coder state
*
* Returns:      MIDAS error code. Coded data is written to *output, the number
*               of input bytes used is written to *inputUsed, and the number
*               of output bytes written is written to *outputWritten.
*
\****************************************************************************/

int CALLING mAdpcmEncode(short *input, unsigned inputLength, uchar *output,
    unsigned outputBufferLength, unsigned *inputUsed, unsigned *outputWritten,
    mAdpcmState *state)
{
    int         outLow = 0;
    mAdpcmChannelState *chan;
    unsigned    c, i;
    unsigned    hnow;
    unsigned    numChannels = state->numChannels;
    mAdpcmChannelState *channels = state->channels;
    int         diff;
    uchar       qdiff;
    int         stepSize;
    int         predDiff;
    mStreamFrameHeader *header;
    U8          *headerPtr, headerSum;

    *inputUsed = 0;
    *outputWritten = 0;

    while ( (outputBufferLength > 0) && (inputLength > 0) )
    {
        /* Interleave channels: */
        chan = channels;
        for ( c = 0; (c < numChannels) && (outputBufferLength > 0) &&
            (inputLength > 0); c++ )
        {
            /* Write our header if necessary: */
            if ( state->frameBytes == 0 )
            {
                /* Check if we need to build it first: */
                if ( state->headerBytes == 0 )
                {
                    /* Build the generic header: */
                    header = state->header;
                    header->magic17 = 0x17; header->magic42 = 0x42;
                    header->frameLength = (U16) state->frameLength;
                    if ( numChannels == 2 )
                        header->sampleType = smpAdpcmStereo;
                    else
                        header->sampleType = smpAdpcmMono;

                    /* Build channel headers: */
                    for ( i = 0; i < numChannels; i++ )
                    {
                        state->chanHeaders[i].predValue =
                            (S16) channels[i].predValue;
                        state->chanHeaders[i].stepSizeIndex =
                            (S8) channels[i].stepSizeIndex;
                    }

                    /* Calculate header checksum: */
                    header->sum = 0;
                    headerSum = 0; headerPtr = (U8*) header;
                    for ( i = 0; i < state->headerSize; i++ )
                        headerSum += *(headerPtr++);
                    header->sum = headerSum;

                    /* Mark that none of the header has been written: */
                    state->headerBytes = state->headerSize;
                    state->headerPtr = (U8*) header;
                }

                /* Make sure we won't overflow the output buffer: */
                if ( state->headerBytes > outputBufferLength )
                    hnow = outputBufferLength;
                else
                    hnow = state->headerBytes;

                /* Write the header bytes: */
                for ( i = 0; i < hnow; i++ )
                    *(output++) = *(state->headerPtr++);
                *outputWritten += hnow;
                outputBufferLength -= hnow;
                state->headerBytes -= hnow;

                /* If none of the header remains, we can continue coding: */
                if ( state->headerBytes == 0 )
                {
                    state->frameBytes = state->frameLength -
                        state->headerSize;
                }

                /* Make sure we won't continue coding if we filled the output
                   buffer: */
                if ( !outputBufferLength )
                    break;
            }

            /* Get the difference between the predicted value and real
               input: */
            diff = (*input++) - chan->predValue;
            inputLength -= 2;
            *inputUsed += 2;

            /* Check if the difference is negative, if so, adjust: */
            if ( diff < 0 )
            {
                qdiff = 8;              /* the sign bit in output */
                diff = -diff;
            }
            else
                qdiff = 0;

            /* Get step size: */
            stepSize = stepSizes[chan->stepSizeIndex];

            /* Quantize the difference based on step size, and calculate
               the predictor difference that corresponds to the quantized
               sample: */
            predDiff = stepSize / 8;
            if ( diff >= stepSize )
            {
                qdiff |= 4;
                diff -= stepSize;
                predDiff += stepSize;
            }

            stepSize /= 2;
            if ( diff >= stepSize )
            {
                qdiff |= 2;
                diff -= stepSize;
                predDiff += stepSize;
            }

            stepSize /= 2;
            if ( diff >= stepSize )
            {
                qdiff |= 1;
                predDiff += stepSize;
            }

            /* Update the predicted value and clip: */
            if ( qdiff & 8 )
                chan->predValue -= predDiff;
            else
                chan->predValue += predDiff;
            if ( chan->predValue > 32767 )
                chan->predValue = 32767;
            else if ( chan->predValue < -32768 )
                chan->predValue = -32768;

            /* Update step size index and clip: */
            chan->stepSizeIndex += mAdpcmStepSizeAdj[qdiff & 7];
            if ( chan->stepSizeIndex < 0 )
                chan->stepSizeIndex = 0;
            else if ( chan->stepSizeIndex > 88 )
                chan->stepSizeIndex = 88;

            /* Finally, write to output: */
            if ( outLow )
            {
                *output++ |= qdiff;
                state->frameBytes--;
                outputBufferLength--;
                *outputWritten += 1;
            }
            else
            {
                *output = (qdiff << 4);
            }
            outLow ^= 1;

            /* Next channel: */
            chan++;
        }
    }

    return OK;
}



#ifdef M_X86_ASSEMBLER
extern void CALLING mAdpcmDecodeMono(mAdpcmChannelState *channels,
                                     uchar *input, short *output,
                                     unsigned numSamples);
extern void CALLING mAdpcmDecodeStereo(mAdpcmChannelState *channels,
                                       uchar *input, short *output,
                                       unsigned numSamples);
#endif


/****************************************************************************\
*
* Function:     int mAdpcmDecode(uchar *input, unsigned inputLength,
*                   short *output, unsigned outputBufferLength,
*                   unsigned *inputUsed, unsigned *outputWritten,
*                   mAdpcmState *state)
*
* Description:  Encodes 4-bit ADPCM audio data into raw 16-bit signed data
*
* Input:        uchar *input            pointer to input data
*               unsigned inputLength    length of input data, in bytes
*               ushort*output           pointer to destination data
*               unsigned outputBufferLength  output buffer length, in bytes.
*               unsigned *inputUsed     pointer to the variable that will get
*                                       the number of input bytes used
*               unsigned *outputWritten pointer to the variable that will get
*                                       the number of output bytes written
*               mAdpcmState *state      ADPCM coder state
*
* Returns:      MIDAS error code. Decoded data is written to *output,
*               the number of input bytes used is written to *inputUsed, and
*               the number of output bytes written is written to
*               *outputWritten.
*
* Notes:        Decoding will be done for numChannels (mAdpcmInitState)
*               samples at a time, so both the input and output buffer must
*               contain data for an integral number of n-channel samples.
*               In addition, the output buffer must have space for an even
*               number of samples for mono streams.
*
\****************************************************************************/

int CALLING mAdpcmDecode(uchar *input, unsigned inputLength, short *output,
    unsigned outputBufferLength, unsigned *inputUsed, unsigned *outputWritten,
    mAdpcmState *state)
{
    int         inLow = 0;
    mAdpcmChannelState *chan;
    unsigned    c, i, hnow;
    unsigned    numChannels = state->numChannels;
    mAdpcmChannelState *channels = state->channels;
    uchar       qdiff;
    U8          *headerPtr, headerSum, readSum;
    mStreamFrameHeader *header;
    int         invalid;
#ifndef M_X86_ASSEMBLER    
    int         stepSizeIndex, predValue;
#endif    
    unsigned    samplesNow;

    *inputUsed = 0;
    *outputWritten = 0;

    if ( outputBufferLength & 3 )
    {
        ERROR(errInvalidArguments, ID_mAdpcmDecode);
        return errInvalidArguments;
    }

    M_PROF_START(M_ADPCM_PROF_DECODE, 1);

    while ( (outputBufferLength > 0) && (inputLength > 0) )
    {
        /* Read the header if necessary: */
        if ( state->frameBytes == 0 )
        {
            /* Initialize read counter and pointer if necessary: */
            if ( state->headerBytes == 0 )
            {
                state->headerBytes = state->headerSize;
                state->headerPtr = (U8*) state->header;
            }

            /* Make sure we won't underflow the input buffer: */
            if ( state->headerBytes > inputLength )
                hnow = inputLength;
            else
                hnow = state->headerBytes;

            /* Read the header bytes: */
            for ( i = 0; i < hnow; i++ )
                *(state->headerPtr++) = *(input++);
            *inputUsed += hnow;
            inputLength -= hnow;
            state->headerBytes -= hnow;

            /* If we have read the whole header, try to parse it: */
            if ( state->headerBytes == 0 )
            {
                header = state->header;

                /* First, check that the magic ID bytes are correct: */
                invalid = 0;
                if ( (header->magic17 != 0x17) ||
                     (header->magic42 != 0x42) )
                    invalid = 1;

                /* Calculate the checksum and check it: */
                if ( !invalid )
                {
                    headerSum = 0; headerPtr = (U8*) header;
                    readSum = header->sum; header->sum = 0;
                    for ( i = 0; i < state->headerSize; i++ )
                        headerSum += *(headerPtr++);

                    if ( headerSum != readSum )
                        invalid = 1;
                }

                if ( invalid )
                {
                    /* The header is invalid - discard its first byte,
                       and try again */
                    headerPtr = (U8*) header;
                    for ( i = 0; i < (state->headerSize-1); i++ )
                    {
                        *headerPtr = *(headerPtr + 1);
                        headerPtr++;
                    }
                    state->headerPtr = headerPtr;
                    state->headerBytes = 1;

                    /* Break out of the channel loop: */
                    break;
                }
                else
                {
                    /* OK, maybe we can now trust the header. Besides, if
                       the header is corrupted, the user has screwed up */

                    /* Get the frame length: */
                    state->frameLength = header->frameLength;
                    state->frameBytes = state->frameLength -
                        state->headerSize;

                    /* Get channel predictor status: */
                    for ( i = 0; i < numChannels; i++ )
                    {
                        channels[i].predValue =
                            state->chanHeaders[i].predValue;
                        channels[i].stepSizeIndex =
                            state->chanHeaders[i].stepSizeIndex;
                    }
                }
            }

            /* Stop if we used up all input: */
            if ( !inputLength )
                break;              /* break out of channel loop */
        }

        /* Calculate the number of samples we can do now: */
        if ( state->frameBytes < inputLength )
            samplesNow = 2 * state->frameBytes / numChannels;
        else
            samplesNow = 2 * inputLength / numChannels;
        if ( samplesNow > (outputBufferLength / (2 * numChannels)) )
            samplesNow = outputBufferLength / (2 * numChannels);

        inputLength -= numChannels * samplesNow / 2;
        *inputUsed += numChannels * samplesNow / 2;
        state->frameBytes -= numChannels * samplesNow / 2;
        outputBufferLength -= 2 * numChannels * samplesNow;
        *outputWritten += 2 * numChannels * samplesNow;

        switch ( numChannels )
        {
            case 1:
#ifdef M_X86_ASSEMBLER
                mAdpcmDecodeMono(channels, input, output, samplesNow);
                input += samplesNow / 2;
                output += samplesNow;
#else                
                /* Optimized mono decoding: */
                stepSizeIndex = channels[0].stepSizeIndex;
                predValue = channels[0].predValue;
                while ( samplesNow )
                {
                    qdiff = *input++;

                    predValue += mAdpcmPredDiffTable[(stepSizeIndex << 4) +
                                                    (qdiff >> 4)];
                    if ( predValue > 32767 )
                        predValue = 32767;
                    if ( predValue < -32768 )
                        predValue = -32768;
                    *output++ = predValue;

                    stepSizeIndex += mAdpcmStepSizeAdj[(qdiff>>4) & 7];
                    if ( stepSizeIndex < 0 )
                        stepSizeIndex = 0;
                    else if ( stepSizeIndex > 88 )
                        stepSizeIndex = 88;
                    
                    predValue += mAdpcmPredDiffTable[(stepSizeIndex << 4) +
                                                    (qdiff & 15)];
                    if ( predValue > 32767 )
                        predValue = 32767;
                    else if ( predValue < -32768 )
                        predValue = -32768;
                    *output++ = predValue;

                    stepSizeIndex += mAdpcmStepSizeAdj[qdiff & 7];
                    if ( stepSizeIndex < 0 )
                        stepSizeIndex = 0;
                    else if ( stepSizeIndex > 88 )
                        stepSizeIndex = 88;
                    
                    samplesNow -= 2;
                }
                channels[0].stepSizeIndex = stepSizeIndex;
                channels[0].predValue = predValue;
#endif                
                break;

            case 2:
#ifdef M_X86_ASSEMBLER
                mAdpcmDecodeStereo(channels, input, output, samplesNow);
                input += samplesNow;
                output += 2*samplesNow;
#else                
                /* Optimized stereo decoding: */
                while ( samplesNow )
                {
                    qdiff = *input++;

                    channels[0].predValue += mAdpcmPredDiffTable[
                        (channels[0].stepSizeIndex << 4) + (qdiff >> 4)];
                    if ( channels[0].predValue > 32767 )
                        channels[0].predValue = 32767;
                    else if ( channels[0].predValue < -32768 )
                        channels[0].predValue = -32768;
                    *output++ = channels[0].predValue;

                    channels[0].stepSizeIndex += mAdpcmStepSizeAdj[(qdiff>>4)
                                                                  & 7];
                    if ( channels[0].stepSizeIndex < 0 )
                        channels[0].stepSizeIndex = 0;
                    else if ( channels[0].stepSizeIndex > 88 )
                        channels[0].stepSizeIndex = 88;
                    
                    channels[1].predValue += mAdpcmPredDiffTable[
                        (channels[1].stepSizeIndex << 4) + (qdiff & 15)];
                    if ( channels[1].predValue > 32767 )
                        channels[1].predValue = 32767;
                    else if ( channels[1].predValue < -32768 )
                        channels[1].predValue = -32768;
                    *output++ = channels[1].predValue;

                    channels[1].stepSizeIndex += mAdpcmStepSizeAdj[qdiff & 7];
                    if ( channels[1].stepSizeIndex < 0 )
                        channels[1].stepSizeIndex = 0;
                    else if ( channels[1].stepSizeIndex > 88 )
                        channels[1].stepSizeIndex = 88;
                    
                    samplesNow--;                    
                }
#endif                
                break;

            default:
                while ( samplesNow )
                {
                    /* Do all channels: */
                    chan = channels;
                    for ( c = 0; c < numChannels; c++, chan++ )
                    {
                        /* Get the quantized difference: */
                        if ( inLow )
                            qdiff = (*(input++)) & 0x0F;
                        else
                            qdiff = (*input) >> 4;
                        inLow ^= 1;

                        chan->predValue += mAdpcmPredDiffTable[
                            (chan->stepSizeIndex << 4) + qdiff];
                
                        if ( chan->predValue > 32767 )
                            chan->predValue = 32767;
                        else if ( chan->predValue < -32768 )
                            chan->predValue = -32768;

                        /* Write it to output: */
                        *output++ = chan->predValue;

                        /* Update step size index and clip: */
                        chan->stepSizeIndex += mAdpcmStepSizeAdj[qdiff & 7];
                        if ( chan->stepSizeIndex < 0 )
                            chan->stepSizeIndex = 0;
                        else if ( chan->stepSizeIndex > 88 )
                            chan->stepSizeIndex = 88;
                    }
            
                    samplesNow--;
                }
                break;
        }
    }

    M_PROF_END_ROUNDS(M_ADPCM_PROF_DECODE, ((*outputWritten) /
                                            (2 * numChannels)));

    return OK;
}





/*
 * $Log: madpcm.c,v $
 * Revision 1.4.2.2  1997/08/21 20:27:06  pekangas
 * Added assembler optimized ADPCM decoding routines
 *
 * Revision 1.4.2.1  1997/08/21 18:02:50  pekangas
 * Greatly optimized the ADPCM decoding functions
 *
 * Revision 1.4  1997/07/31 10:56:45  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.3  1997/02/27 16:06:41  pekangas
 * Fixed GCC warnings
 *
 * Revision 1.2  1997/02/18 20:21:35  pekangas
 * New ADPCM encoding/decoding code: ADPCM streams now consist of frames,
 * and playback can be started at the beginning of any frame.
 *
 * Revision 1.1  1997/02/11 18:14:21  pekangas
 * Initial revision
 *
*/
