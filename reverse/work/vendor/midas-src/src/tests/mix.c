/*      mix.c
 *
 * MIDAS mixing test
 *
 * $Id: mix.c,v 1.11.2.1 1997/08/22 17:11:06 pekangas Exp $
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
#include <stdlib.h>
#include <string.h>
#include "testutil.h"
#include "midas.h"
#include "mulaw.h"
#include "midasdll.h"
#include "mprof.h"
#include "dsm.h"

#define VSTART 0x1000
#define VLOOP1 0x2000
#define VAFTER1 0x3000
#define VLOOP2 0x0400

#define BUFFERSIZE 65536

#define STREAMLEN 4217

uchar *buf;

int leftVol = 64, rightVol = 64;
int ulaw = 0;


#define TESTIT(x, m) \
    if ( x ) \
    { \
        TEST_PASS(m); \
    } \
    else \
    { \
        TEST_FAIL(m); \
    }          


void MIDASerror(void)
{
    printf("MIDAS error: %s\n", MIDASgetErrorMessage(MIDASgetLastError()));

#ifdef DEBUG
    errPrintList();
#endif

    exit(EXIT_FAILURE);
}


void Error(char *msg)
{
    printf("Error: %s\n", msg);
    exit(EXIT_FAILURE);
}


void TestInit(void)
{
    int error;
    unsigned mem1, mem2,mem3,mem4;

    TestPrintf("\n");
    TEST_NOTEV(("----"));
    TEST_NOTE(("Testing initialization"));
    TEST_NOTEV(("----"));
    mem1 = MemUsed();
    if ( (error = dsmInit(44100, dsmMix16bit | dsmMixMono)) != OK )
        FatalMidasError(error);
    mem2 = MemUsed();
    TEST_PASS(("dsmInit for 44100Hz, 16-bit mono"));
    TEST_NOTEV(("Memory used: %u bytes", mem2-mem1));

    if ( (error = dsmClose()) != OK )
        FatalMidasError(error);
    mem3 = MemUsed();
    TEST_PASS(("dsmClose"));
    TEST_NOTEV(("Memory left allocated: %u bytes", mem3-mem1));

    if ( (error = dsmInit(44100, dsmMix16bit | dsmMixMono)) != OK )
        FatalMidasError(error);
    mem4 = MemUsed();
    TEST_PASS(("Second dsmInit"));
    if ( mem4 > mem2 )
    {
        TEST_FAIL(("Additional memory used: %u bytes", mem4-mem2));
    }
    else
    {
        TEST_PASS(("No additional memory used"));
    }

    if ( (error = dsmClose()) != OK )
        FatalMidasError(error);
    mem4 = MemUsed();
    TEST_PASS(("Second dsmClose"));
    TEST_NOTEV(("Memory left allocated: %u bytes", mem4-mem1));
    if ( mem4 > mem3 )
    {
        TEST_FAIL(("Additional memory left allocated: %u bytes", mem4-mem3));
    }
    else
    {
        TEST_PASS(("No additional memory left allocated"));
    }
}


unsigned SampleSize(int sampleType)
{
    switch ( sampleType )
    {
        case smp8bitMono:
        case smpUlawMono:
            return 1;

        case smp8bitStereo:
        case smpUlawStereo:
        case smp16bitMono:
            return 2;

        case smp16bitStereo:
            return 4;
    }

    return 1;
}


uchar *FillSample(uchar *sample, unsigned numSamples, int sampleType,
                  int values)
{
    uchar v8[2];
    static uchar vu[2];
    static short v16[2];
    int v = 0;
    uchar *s8 = sample;
    short *s16 = (short*) sample;

    v8[0] = 128 + (values >> 8);
    v8[1] = 128 + ((-values) >> 8);
    v16[0] = values;
    v16[1] = -values;

    mUlawEncode(v16, 2 * sizeof(short), vu);

    switch ( sampleType )
    {
        case smp8bitMono:
            while ( numSamples )
            {
                *(s8++) = v8[v];
                v ^= 1;
                numSamples--;
            }
            return s8;

        case smp8bitStereo:
            while ( numSamples )
            {
                *(s8++) = v8[v];
                *(s8++) = v8[v];
                v ^= 1;
                numSamples--;
            }
            return s8;

        case smpUlawMono:
            while ( numSamples )
            {
                *(s8++) = vu[v];
                v ^= 1;
                numSamples--;
            }
            return s8;

        case smpUlawStereo:
            while ( numSamples )
            {
                *(s8++) = vu[v];
                *(s8++) = vu[v];
                v ^= 1;
                numSamples--;
            }
            return s8;

        case smp16bitMono:
            while ( numSamples )
            {
                *(s16++) = v16[v];
                v ^= 1;
                numSamples--;
            }
            return (uchar*) s16;

        case smp16bitStereo:
            while ( numSamples )
            {
                *(s16++) = v16[v];
                *(s16++) = v16[v];
                v ^= 1;
                numSamples--;
            }
            return (uchar*) s16;
    }

    return NULL;
}


unsigned CreateSample(int sampleType, unsigned sampleLength, int loopMode,
                      unsigned loop1Start, unsigned loop1End, int loop1Type,
                      unsigned loop2Start, unsigned loop2End, int loop2Type,
                      int valueMul)
{
    static unsigned smpHandle;
    static sdSample smp;
    static uchar *sbuf, *s;
    unsigned sampleSize = SampleSize(sampleType);
    int error;
    
    if ( (error = memAlloc(sampleLength * sampleSize, (void**) &sbuf)) != OK )
        FatalMidasError(error);
    
    smp.sample = sbuf;
    smp.samplePos = sdSmpConv;
    smp.sampleType = sampleType;
    smp.sampleLength = sampleLength * sampleSize;
    smp.loopMode = loopMode;
    smp.loop1Start = loop1Start * sampleSize;
    smp.loop1End = loop1End * sampleSize;
    smp.loop1Type = loop1Type;
    smp.loop2Start = loop2Start * sampleSize;
    smp.loop2End = loop2End * sampleSize;
    smp.loop2Type = loop2Type;

    s = sbuf;

    switch ( loopMode )
    {
        case sdLoopNone:
        case sdLoopAmigaNone:
            FillSample(s, sampleLength, sampleType, VSTART * valueMul);
            break;

        case sdLoop1:
        case sdLoopAmiga:
        case sdLoop1Rel:
            s = FillSample(s, loop1Start, sampleType, VSTART * valueMul);
            s = FillSample(s, loop1End-loop1Start, sampleType,
                           VLOOP1 * valueMul);
            FillSample(s, sampleLength-loop1End, sampleType,
                       VAFTER1 * valueMul);
            break;

        case sdLoop2:
            s = FillSample(s, loop1Start, sampleType, VSTART * valueMul);
            s = FillSample(s, loop1End-loop1Start, sampleType,
                           VLOOP1 * valueMul);
            s = FillSample(s, loop2Start-loop1End, sampleType,
                           VAFTER1 * valueMul);
            FillSample(s, loop2End-loop2Start, sampleType, VLOOP2 * valueMul);
            break;
    }

    if ( (error = dsmAddSample(&smp, 1, &smpHandle)) != OK )
        FatalMidasError(error);

    return smpHandle;
}


void SetVolume(unsigned volume)
{
    int error;
    
    if ( (error = dsmSetVolume(0, volume)) != OK )
        FatalMidasError(error);
}


void SetPanning(int panning)
{
    int error;
    
    if ( (error = dsmSetPanning(0, panning)) != OK )
        FatalMidasError(error);
}


void PlaySample(unsigned sample, unsigned rate, unsigned volume)
{
    int error;

    if ( (error = dsmSetSample(0, sample)) != OK )
        FatalMidasError(error);
    if ( (error = dsmSetVolume(0, volume)) != OK )
        FatalMidasError(error);
    if ( (error = dsmPlaySound(0, rate)) != OK )
        FatalMidasError(error);
}


void SetSample(unsigned sample)
{
    int error;

    if ( (error = dsmSetSample(0, sample)) != OK )
        FatalMidasError(error);
}



void StopSample(void)
{
    int error;

    if ( (error = dsmStopSound(0)) != OK )
        FatalMidasError(error);
}


void ReleaseSample(void)
{
    int error;

    if ( (error = dsmReleaseSound(0)) != OK )
        FatalMidasError(error);
}


void MixData(unsigned mixMode, unsigned numSamples, unsigned startPos)
{
    int error;
    uchar *b = buf;

    if ( (mixMode & dsmMix8bitMono) == dsmMix8bitMono )
        b += startPos;
    if ( (mixMode & dsmMix8bitStereo) == dsmMix8bitStereo )
        b += 2*startPos;
    if ( (mixMode & dsmMix16bitMono) == dsmMix16bitMono )
        b += 2*startPos;
    if ( (mixMode & dsmMix16bitStereo) == dsmMix16bitStereo )
        b += 4*startPos;

    if ( (error = dsmMixData(numSamples, (void*) b)) != OK )
        FatalMidasError(error);
}


int Equal8(uchar a, uchar b)
{
    if ( ulaw )
    {
        if ( (a == b) || (((((int) a) - (((int) b))) < 4) &&
                          ((((int) a) - (((int) b))) > -4)) )
            return 1;
    }
    else
    {
        if ( (a == b) || ((a-1) == b) || ((a+1) == b) )
            return 1;
    }
    return 0;
}


int Equal16(short a, short b)
{
    if ( ulaw )
    {
        if ( (a == b) || (((a-b) < 512) && ((a-b) > -512)) )
            return 1;
    }
    else
    {
        if ( (a == b) || ((a-1) == b) || ((a+1) == b) )
            return 1;
    }
    return 0;    
}
        

int Check2Val(unsigned mixMode, unsigned startPos, unsigned numSamples,
              int values)
{
    uchar v8[2], vr8[2];
    short v16[2], vr16[2];
    int v = 0;
    uchar *s8 = buf;
    short *s16 = (short*) buf;
    unsigned orgSamples = numSamples;

#define C2FAIL { TEST_NOTE(("Check2Val: Failed at %u + %u/%u", \
                            startPos, (orgSamples-numSamples), orgSamples)); \
                 return 0; }

    v8[0] = 128 + (((leftVol * values) / 64) >> 8);
    v8[1] = 128 + ((-((leftVol * values) / 64)) >> 8);
    vr8[0] = 128 + (((rightVol * values) / 64) >> 8);
    vr8[1] = 128 + ((-((rightVol * values) / 64)) >> 8);
    v16[0] = (leftVol * values) / 64;
    v16[1] = -(leftVol * values) / 64;
    vr16[0] = (rightVol * values) / 64;
    vr16[1] = -(rightVol * values) / 64;

    if ( (mixMode & dsmMix8bitMono) == dsmMix8bitMono )
    {
        s8 += startPos;
        while ( numSamples )
        {
            if ( !Equal8(*(s8++), v8[v]) )
                C2FAIL;
            v ^= 1;
            numSamples--;
        }
        return 1;
    }

    if ( (mixMode & dsmMix8bitStereo) == dsmMix8bitStereo )
    {
        s8 += 2*startPos;
        while ( numSamples )
        {
            if ( !Equal8(*(s8++), v8[v]) )
                C2FAIL;
            if ( !Equal8(*(s8++), vr8[v]) )
                C2FAIL;
            v ^= 1;
            numSamples--;
        }
        return 1;
    }

    if ( (mixMode & dsmMix16bitMono) == dsmMix16bitMono )
    {
        s16 += startPos;
        while ( numSamples )                
        {
            if ( !Equal16(*(s16++), v16[v]) )
                C2FAIL;
            v ^= 1;
            numSamples--;
        }
        return 1;
    }
            
    if ( (mixMode & dsmMix16bitStereo) == dsmMix16bitStereo )
    {
        s16 += 2*startPos;
        while ( numSamples )                
        {
            if ( !Equal16(*(s16++), v16[v]) )
                C2FAIL;
            if ( !Equal16(*(s16++), vr16[v]) )
                C2FAIL;
            v ^= 1;
            numSamples--;
        }
        return 1;
    }

    return 0;
}


int CheckVal(unsigned mixMode, unsigned startPos, unsigned numSamples,
             int value)
{
    uchar v8 = 128 + (((leftVol * value) / 64) >> 8);
    uchar vr8 = 128 + (((rightVol * value) / 64) >> 8);    
    short v16 = (leftVol * value) / 64;
    short vr16 = (rightVol * value) / 64;
    uchar *s8 = buf;
    short *s16 = (short*) buf;

    if ( (mixMode & dsmMix8bitMono) == dsmMix8bitMono )
    {
        s8 += startPos;
        while ( numSamples )            
        {
            if ( !Equal8(*(s8++), v8) )
                return 0;
            numSamples--;
        }
        return 1;
    }

    if ( (mixMode & dsmMix8bitStereo) == dsmMix8bitStereo )
    {
        s8 += 2*startPos;
        while ( numSamples )
        {            
            if ( !Equal8(*(s8++), v8) )
                return 0;
            if ( !Equal8(*(s8++), vr8) )
                return 0;
            numSamples--;
        }
        return 1;
    }

    if ( (mixMode & dsmMix16bitMono) == dsmMix16bitMono )
    {
        s16 += startPos;
        while ( numSamples )                
        {
            if ( !Equal16(*(s16++), v16) )
                return 0;
            numSamples--;
        }
        return 1;
    }
            
    if ( (mixMode & dsmMix16bitStereo) == dsmMix16bitStereo )
    {
        s16 += 2*startPos;
        while ( numSamples )                
        {
            if ( !Equal16(*(s16++), v16) )
                return 0;
            if ( !Equal16(*(s16++), vr16) )
                return 0;
            numSamples--;
        }
        return 1;
    }

    return 0;
}



void TestSampleType(int sampleType, unsigned mixRate, unsigned mixMode,
                    int panning, int ulawAutoConvert)
{
    unsigned sample, sample2;
    int error;


    TEST_NOTEV(("+++ Testing sample type %i, panning %i", sampleType,
               panning));

    if ( mixMode & dsmMixStereo )
    {
        switch ( panning )
        {
            case panMiddle:
                leftVol = 64;
                rightVol = 64;
                break;

            case panSurround:
                leftVol = 64;
                rightVol = -64;
                break;

            default:
                if ( panning < 0 )
                {
                    leftVol = 64;
                    rightVol = 64 + panning;
                }
                else
                {
                    leftVol = 64 - panning;
                    rightVol = 64;
                }
                break;
        }
    }
    else
    {
        leftVol = 64;
        rightVol = 64;
    }    

    if ( (sampleType == smpUlawMono) || (sampleType == smpUlawStereo) )
    {
        ulaw = 1;
        TEST_NOTEV(("u-law sample mode"));
    }
    else
        ulaw = 0;

    if ( ulawAutoConvert )
    {
        ulaw = 1;
        TEST_NOTEV(("u-law auto conversion mode"));
    }

    SetPanning(panning);
    
    /**  Non-looping sample **/
    
    sample = CreateSample(sampleType, 1000, sdLoopNone, 0, 0, loopNone,
                          0, 0, loopNone, 1);


    /* Make sure that volume is ramped to 64: */
    PlaySample(sample, mixRate, 64);
    MixData(mixMode, 256, 0);
    StopSample();
    MixData(mixMode, 256, 0);
    
    PlaySample(sample, mixRate, 64);
    MixData(mixMode, 1024, 0);
    
    if ( Check2Val(mixMode, 0, 200, VSTART) )
    {
        TEST_PASS(("Resample 1"));
    }
    else
    {
        TEST_FAIL(("Resample 1 failed, skipping the rest"));
        if ( (error = dsmRemoveSample(sample)) != OK )
            FatalMidasError(error);
        return;
    }    

    TESTIT(((Check2Val(mixMode, 0, 1000, VSTART)) &&
            (CheckVal(mixMode, 1000, 24, 0))),
           ("Non-looping sample"));

    StopSample();
    MixData(mixMode, 256, 0);
    if ( (error = dsmRemoveSample(sample)) != OK )
        FatalMidasError(error);


    /** Unidirectional loop **/
    
    sample = CreateSample(sampleType, 18, sdLoop1, 0, 18, loopUnidir,
                          0, 0, loopNone, 1);

    PlaySample(sample, 3*mixRate, 64);
    MixData(mixMode, 500, 0);
    TESTIT(Check2Val(mixMode, 0, 500, VLOOP1),
           ("Unidirectional loop with resample 3"));

    StopSample();
    MixData(mixMode, 256, 0);    

    PlaySample(sample, 2*mixRate, 64);
    MixData(mixMode, 400, 0);
    TESTIT(CheckVal(mixMode, 0, 400, VLOOP1),
           ("Resample 2"));

    StopSample();
    MixData(mixMode, 256, 0);    
    if ( (error = dsmRemoveSample(sample)) != OK )
        FatalMidasError(error);
    
    
    /** overshort loop **/
    
    sample = CreateSample(sampleType, 10, sdLoop1, 9, 10, loopUnidir,
                          0, 0, loopNone, 1);

    PlaySample(sample, 3 * mixRate / 2, 64);
    MixData(mixMode, 100, 0);
    StopSample();
    MixData(mixMode, 256, 0);    
    if ( (error = dsmRemoveSample(sample)) != OK )
        FatalMidasError(error);

    TEST_PASS(("Overshort loop - didn't crash"));
    
    
    /** Data + unidirectional loop **/
    
    sample = CreateSample(sampleType, 100, sdLoop1, 50, 100, loopUnidir,
                          0, 0, loopNone, 1);

    PlaySample(sample, mixRate, 64);
    MixData(mixMode, 150, 0);
    MixData(mixMode, 150, 150);
    TESTIT((Check2Val(mixMode, 0, 50, VSTART) &&
            Check2Val(mixMode, 50, 250, VLOOP1)),
           ("Data + unidirectional loop"));

    StopSample();
    MixData(mixMode, 256, 0);    
    if ( (error = dsmRemoveSample(sample)) != OK )
        FatalMidasError(error);


    /** Unidirectional loop with release **/
      
    sample = CreateSample(sampleType, 200, sdLoop1Rel, 50, 100, loopUnidir,
                          0, 0, loopNone, 1);

    PlaySample(sample, mixRate, 64);
    MixData(mixMode, 225, 0);
    ReleaseSample();
    MixData(mixMode, 225, 225);
    TESTIT((Check2Val(mixMode, 0, 50, VSTART) &&
            Check2Val(mixMode, 50, 200, VLOOP1) &&
            Check2Val(mixMode, 250, 100, VAFTER1) &&
            CheckVal(mixMode, 350, 50, 0)),
           ("Unidirectional loop with release"));

    StopSample();
    MixData(mixMode, 256, 0);    
    if ( (error = dsmRemoveSample(sample)) != OK )
        FatalMidasError(error);


    /* Bidirectional loop */
    if ( (mixMode & dsmMixInterpolation)  == 0 )
    {
        /* Can't test this for interpolating routines - they mix a bit off with
           bidirectional loops */        
        sample = CreateSample(sampleType, 100, sdLoop1, 0, 99, loopBidir,
                              0, 0, loopNone, 1);

        PlaySample(sample, mixRate, 64);
        MixData(mixMode, 300, 0);
        TESTIT((Check2Val(mixMode, 0, 99, VLOOP1) &&
                CheckVal(mixMode, 98, 2, VLOOP1) &&
                Check2Val(mixMode, 99, 99, VLOOP1) &&
                CheckVal(mixMode, 197, 2, VLOOP1) &&
                Check2Val(mixMode, 198, 99, VLOOP1)),
               ("Bidirectional loop"));

        StopSample();
        MixData(mixMode, 256, 0);        
        if ( (error = dsmRemoveSample(sample)) != OK )
            FatalMidasError(error);
    }
    
    /* Bidirectional loop with release*/
    if ( (mixMode & dsmMixInterpolation)  == 0 )
    {
        /* Can't test this for interpolating routines - they mix a bit off with
           bidirectional loops */        
        sample = CreateSample(sampleType, 200, sdLoop1Rel, 0, 99, loopBidir,
                              0, 0, loopNone, 1);

        PlaySample(sample, mixRate, 64);
        MixData(mixMode, 120, 0);
        ReleaseSample();
        MixData(mixMode, 200, 120);
        TESTIT((Check2Val(mixMode, 0, 99, VLOOP1) &&
                CheckVal(mixMode, 98, 2, VLOOP1) &&
                Check2Val(mixMode, 99, 99, VLOOP1) &&
                CheckVal(mixMode, 197, 2, VLOOP1) &&
                Check2Val(mixMode, 198, 99, VLOOP1) &&
                Check2Val(mixMode, 297, 23, VAFTER1)),
               ("Bidirectional loop with release"));

        StopSample();
        MixData(mixMode, 256, 0);        
        if ( (error = dsmRemoveSample(sample)) != OK )
            FatalMidasError(error);
    }

    
    /* Two loops */

    sample = CreateSample(sampleType, 200, sdLoop2, 0, 100, loopUnidir,
                          150, 200, loopUnidir, 1);

    PlaySample(sample, mixRate, 64);
    MixData(mixMode, 150, 0);
    ReleaseSample();
    MixData(mixMode, 250, 150);
    TESTIT((Check2Val(mixMode, 0, 200, VLOOP1) &&
            Check2Val(mixMode, 200, 50, VAFTER1) &&
            Check2Val(mixMode, 250, 150, VLOOP2)),
           ("Two loops"));

    StopSample();
    MixData(mixMode, 256, 0);    
    if ( (error = dsmRemoveSample(sample)) != OK )
        FatalMidasError(error);


    /* Amiga-compatible looping */

    sample = CreateSample(sampleType, 100, sdLoopAmigaNone, 0, 0, loopNone,
                          0, 0, loopNone, 1);
    sample2 = CreateSample(sampleType, 100, sdLoopAmiga, 50, 100, loopUnidir,
                           0, 0, loopNone, 1);
    PlaySample(sample, mixRate, 64);
    MixData(mixMode, 50, 0);
    SetSample(sample2);
    MixData(mixMode, 250, 50);

    TESTIT((Check2Val(mixMode, 0, 100, VSTART) &&
            Check2Val(mixMode, 100, 200, VLOOP1)),
           ("Amiga-compatible looping"));

    StopSample();
    MixData(mixMode, 256, 0);    
    if ( (error = dsmRemoveSample(sample)) != OK )
        FatalMidasError(error);
    if ( (error = dsmRemoveSample(sample2)) != OK )
        FatalMidasError(error);
}


unsigned GetPos(void)
{
    int error;
    static unsigned pos;

    if ( (error = dsmGetPosition(0, &pos)) != OK )
        FatalMidasError(error);
    return pos;
}


void SetStreamPos(unsigned pos)
{
    int error;

    if ( (error = dsmSetStreamWritePosition(0, pos)) != OK )
        FatalMidasError(error);    
}



void TestStream(unsigned mixRate, unsigned mixMode)
{
    uchar *sbuf;
    int error;

    TEST_NOTEV(("+++ Stream playback"));

    m16bitUlawAutoConvert = 0;

    sbuf = malloc(STREAMLEN);
    FillSample(sbuf, STREAMLEN, smp8bitMono, VLOOP1);
    memset(buf, 0, BUFFERSIZE);

    if ( (error = dsmStartStream(0, sbuf, STREAMLEN, smp8bitMono, mixRate))
         != OK )
        FatalMidasError(error);

    TESTIT((GetPos() == 0),
           ("Initial stream position: %u", GetPos()));
    MixData(mixMode, 200, 0);
    TESTIT((GetPos() == 0),
           ("Stream position before moving write position: %u", GetPos()));
    SetStreamPos(713);
    MixData(mixMode, 900, 0);
    TESTIT((GetPos() == 713),
           ("Stream position at write position: %u", GetPos()));
    SetStreamPos(STREAMLEN);
    MixData(mixMode, STREAMLEN + 100, 0);
    TESTIT((GetPos() == STREAMLEN),
           ("Stream position at end: %u", GetPos()));
    SetStreamPos(17);
    MixData(mixMode, 100, 0);
    TESTIT((GetPos() == 17),
           ("Stream position wrapped: %u", GetPos()));
    
    if ( (error = dsmStopStream(0)) != OK )
        FatalMidasError(error);

    free(sbuf);
}



void TestMixMode(unsigned mixRate, unsigned mixMode)
{
    int error;
#define NUMPANVALUES 6
    static int panningValues[NUMPANVALUES] = { panMiddle, panLeft,
                                               panRight, -32, 24,
                                               panSurround };
    int pan;
    unsigned i;
    

    TestPrintf("\n");
    TEST_NOTEV(("----"));
    TEST_NOTE(("Testing rate %u, mode 0x%x", mixRate, mixMode));
    TEST_NOTEV(("----"));

    m16bitUlawAutoConvert = 0;

    if ( (error = dsmInit(mixRate, mixMode)) != OK )
        FatalMidasError(error);
    if ( (error = dsmOpenChannels(1)) != OK )
        FatalMidasError(error);

    for ( i = 0; i < NUMPANVALUES; i++ )
    {
        pan = panningValues[i];
        TestSampleType(smp8bitMono, mixRate, mixMode, pan, 0);
        TestSampleType(smp16bitMono, mixRate, mixMode, pan, 0);
        TestSampleType(smp8bitStereo, mixRate, mixMode, pan, 0);
        TestSampleType(smp16bitStereo, mixRate, mixMode, pan, 0);
        TestSampleType(smpUlawMono, mixRate, mixMode, pan, 0);
        TestSampleType(smpUlawStereo, mixRate, mixMode, pan, 0);
    }

    TestStream(mixRate, mixMode);

    if ( (error = dsmCloseChannels()) != OK )
        FatalMidasError(error);
    if ( (error = dsmClose()) != OK )
        FatalMidasError(error);


    /* Test u-law auto conversion: */

    m16bitUlawAutoConvert = 1;

    if ( (error = dsmInit(mixRate, mixMode)) != OK )
        FatalMidasError(error);
    if ( (error = dsmOpenChannels(1)) != OK )
        FatalMidasError(error);

    for ( i = 0; i < NUMPANVALUES; i++ )
    {
        pan = panningValues[i];
        TestSampleType(smp16bitMono, mixRate, mixMode, pan, 1);
        TestSampleType(smp16bitStereo, mixRate, mixMode, pan, 1);
    }

    TestStream(mixRate, mixMode);

    if ( (error = dsmCloseChannels()) != OK )
        FatalMidasError(error);
    if ( (error = dsmClose()) != OK )
        FatalMidasError(error);

    m16bitUlawAutoConvert = 0;
}



int main(int argc, char *argv[])
{
    MIDASstartup();

#ifdef M_PROF
    mProfInit();
#endif

    /* Disable volume ramping - it make testing almost impossible */
    mDisableVolumeRamping = 1;

    /* Disable filtering: */
    mOutputFilterMode = 0;

    /* Disable u-law auto conversion: */
    m16bitUlawAutoConvert = 0;

    buf = malloc(BUFFERSIZE);

    printf("Opening test output log\n");

    if ( argc > 1 )
        StartPrint("mix.log", 1);
    else
        StartPrint("mix.log", 0);

    TEST_NOTE(("==============="));
    TEST_NOTE(("= Mixing test ="));
    TEST_NOTE(("==============="));
    TEST_NOTE(("[-v for verbose]"));

    TestInit();

    TestMixMode(22050, dsmMixFloat | dsmMix16bit | dsmMixMono);
    TestMixMode(32000, dsmMixFloat | dsmMix16bit | dsmMixStereo);
    TestMixMode(44100, (dsmMixFloat | dsmMix16bit | dsmMixMono |
                        dsmMixInterpolation));
    TestMixMode(16000, (dsmMixFloat | dsmMix16bit | dsmMixStereo |
                        dsmMixInterpolation));
    TestMixMode(28000, dsmMixInteger | dsmMix16bit | dsmMixMono);
    TestMixMode(38000, dsmMixInteger | dsmMix16bit | dsmMixStereo);
    TestMixMode(11025, dsmMixInteger | dsmMix8bit | dsmMixStereo);
    TestMixMode(8000, dsmMixFloat | dsmMix8bit | dsmMixMono);

    TEST_NOTE(("Test finished"));

    if ( failed )
        TestPrintfDisp("!! Test failed !!\n");
    else
        TestPrintfDisp("!! Test passed !!\n");

    EndPrint();

    return 0;
}



/*
 * $Log: mix.c,v $
 * Revision 1.11.2.1  1997/08/22 17:11:06  pekangas
 * Added a test for overshort loops
 *
 * Revision 1.11  1997/08/18 16:50:41  pekangas
 * midas.h no longer includes dsm.h
 *
 * Revision 1.10  1997/07/31 10:56:58  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.9  1997/07/31 09:41:28  pekangas
 * Added u-law autoconversion test
 *
 * Revision 1.8  1997/07/30 14:17:37  pekangas
 * Added and fixed 8-bit output testing, plus some code cleanup
 * Cleaned up output a bit
 *
 * Revision 1.7  1997/07/28 13:18:25  pekangas
 * Fixed to compile in DOS, both with Watcom C and GCC
 *
 * Revision 1.6  1997/07/26 14:14:22  pekangas
 * Disabled filtering
 *
 * Revision 1.5  1997/07/24 23:14:39  pekangas
 * Added testing for different panning positions
 * Added support for verbose on/off
 *
 * Revision 1.4  1997/07/17 09:50:55  pekangas
 * Added testing for integer mono mixing
 * Removed an extra dsmClose() from main() (oops)
 *
 * Revision 1.3  1997/07/05 14:22:30  pekangas
 * Added test for Amiga-compatible looping
 *
 * Revision 1.2  1997/06/26 14:35:21  pekangas
 * Added tests for interpolating routines
 * Fixed to work with volume ramping and sample translation buffers
 * Added some diagnostic NOTEs when the tests fail
 *
 * Revision 1.1  1997/05/30 18:24:07  pekangas
 * Initial revision
 *
*/
