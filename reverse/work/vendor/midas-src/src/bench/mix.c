/*      mix.c
 *
 * Mixing routine benchmarks
 *
 * $Id: mix.c,v 1.6.2.1 1997/08/25 19:38:48 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "bench.h"
#include "midas.h"
#include "dsm.h"
#include "mprof.h"
#include "midasdll.h"


void *buf;
benchTime overhead;
unsigned emptyCallCycles;
benchTime mixEmpty;



void MIDASerror(int error)
{
    printf("MIDAS Error: %s\n", MIDASgetErrorMessage(error));
    exit(EXIT_FAILURE);
}


static void CALLING EmptyMixRoutine(unsigned numSamples)
{
    numSamples = numSamples;
}



void CheckOverhead(void)
{
    benchTime t;
    unsigned i;
    unsigned c;
    void (CALLING *mixLoop)(unsigned numSamples);    
    
    printf("Checking overhead\n");
    Sleep(100);

    overhead = 1000000;

    for ( i = 0; i < 1000; i++ )
    {
        t = benchGetTime();
        t = benchGetTime() - t;
        if ( t < overhead )
            overhead = t;
    }

    printf("Overhead: %f microseconds\n", overhead);

    Sleep(100);

    emptyCallCycles = 100000;
    mixLoop = &EmptyMixRoutine;

#ifdef HAVE_CYCLE_COUNT    
    for ( i = 0; i < 100; i++ )
    {
        c = benchGetCycleCount();
        mixLoop(1742);
        c = benchGetCycleCount() - c;
        if ( c < emptyCallCycles )
            emptyCallCycles = c;
    }

    printf("Empty call %u cycles\n", emptyCallCycles);
#endif    
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



unsigned CreateSample(int sampleType, unsigned sampleLength)
{
    static unsigned smpHandle;
    static sdSample smp;
    static uchar *sbuf, *s;
    unsigned sampleSize = SampleSize(sampleType);
    unsigned n;
    int error;
    
    if ( (error = memAlloc(sampleLength * sampleSize, (void**) &sbuf)) != OK )
        MIDASerror(error);
    
    smp.sample = sbuf;
    smp.samplePos = sdSmpConv;
    smp.sampleType = sampleType;
    smp.sampleLength = sampleLength * sampleSize;
    smp.loopMode = sdLoop1;
    smp.loop1Start = 0;
    smp.loop1End = sampleLength * sampleSize;
    smp.loop1Type = loopUnidir;
    smp.loop2Start = 0;
    smp.loop2End = 0;
    smp.loop2Type = loopNone;

    s = sbuf;
    for ( n = 0; n < sampleSize; n++ )
    {
        *(s++) = rand() & 0xFF;
    }

    if ( (error = dsmAddSample(&smp, 1, &smpHandle)) != OK )
        MIDASerror(error);

    return smpHandle;
}


void PlaySample(unsigned sample, unsigned rate, unsigned volume)
{
    int error;

    if ( (error = dsmSetSample(0, sample)) != OK )
        MIDASerror(error);
    if ( (error = dsmSetVolume(0, volume)) != OK )
        MIDASerror(error);
    if ( (error = dsmPlaySound(0, rate)) != OK )
        MIDASerror(error);
}


void StopSample(void)
{
    int error;

    if ( (error = dsmStopSound(0)) != OK )
        MIDASerror(error);
}



void CALLING dsmClearBufferFloat(unsigned numSamples);
void CALLING dsmClearBufferInt(unsigned numSamples);
void CALLING dsmConvertFloat16(unsigned numSamples, S16 *dest);
void CALLING dsmConvertInt16(unsigned numSamples, S16 *dest);


void TestEmptyMix(unsigned mixMode)
{
    int error, i;
    benchTime t;
    benchTime min = 1000000, total = 0;
#ifdef HAVE_CYCLE_COUNT    
    unsigned c, minc, totalc, mino, totalo;
    unsigned n;
    void (CALLING *clear)(unsigned numSamples);
    void (CALLING *convert)(unsigned numSamples, S16 *dest);
#endif    

    printf("\nTesting empty mixing for mixing mode 0x%x\n", mixMode);
    Sleep(100);

    if ( (error = dsmInit(44100, mixMode)) != OK )
        MIDASerror(error);
    if ( (error = dsmOpenChannels(1)) != OK )
        MIDASerror(error);

    for ( i = 0; i < 50; i++ )
    {
        t = benchGetTime();
        if ( (error = dsmMixData(44100, buf)) != OK )
            MIDASerror(error);
        t = benchGetTime() - t - overhead;
        total = total + t;
        if ( min > t )
            min = t;
    }

#ifdef HAVE_CYCLE_COUNT    
    minc = 1000000; totalc = 0;
    if ( mixMode & dsmMixInteger )
        clear = &dsmClearBufferInt;
    else
        clear = &dsmClearBufferFloat;

    for ( i = 0; i < 100; i++ )
    {
        c = benchGetCycleCount();
        clear(1000);
        c = benchGetCycleCount() - c;
        c = c - emptyCallCycles;
        if ( minc > c )
            minc = c;
        totalc = totalc + c;
    }

    mino = 1000000; totalo = 0;
    if ( mixMode & dsmMixInteger )
        convert = dsmConvertInt16;
    else
        convert = &dsmConvertFloat16;

    if ( mixMode & dsmMixStereo )
        n = 2000;
    else
        n = 1000;
    
    for ( i = 0; i < 100; i++ )
    {
        c = benchGetCycleCount();
        convert(n, (S16*) buf);
        c = benchGetCycleCount() - c;
        c = c - emptyCallCycles;
        if ( mino > c )
            mino = c;
        totalo += c;
    }
#endif    

    if ( (error = dsmClose()) != OK )
        MIDASerror(error);

    printf("Empty mixing 44100: Min %.0f, Avg %.0f, %.2f%% of CPU\n",
           min, total / 50.0, 100.0 * ((total / 50.0) / 1000000.0));
    mixEmpty = min;

#ifdef HAVE_CYCLE_COUNT    
    printf("Buffer clear 1000: Min %u, Avg %u cycles, "
           "Min %u Avg %u per sample\n", minc, totalc/100, minc/1000,
           totalc/(100*1000));
    
    printf("Convert+clip 1000: Min %u, Avg %u cycles, "
           "Min %u Avg %u per sample\n", mino, totalo/100, mino/1000,
           totalo/(100*1000));
#endif    
}



void TestOneChannelMix(unsigned sampleType, unsigned mixMode, int panning)
{
    int error, i;
    benchTime t;
    benchTime min = 1000000, total = 0;
    unsigned sample;
#ifdef HAVE_CYCLE_COUNT    
    void (CALLING *mixLoop)(unsigned numSamples, int nextSampleOffset);
    unsigned c;
    unsigned minm, totalm;
#endif    

    printf("\nTesting single channel mixing: sample type 0x%x, "
           "mixing mode 0x%x, panning %i\n", sampleType, mixMode,
           panning);
    Sleep(100);

    if ( (error = dsmInit(44100, mixMode)) != OK )
        MIDASerror(error);
    if ( (error = dsmOpenChannels(1)) != OK )
        MIDASerror(error);
    if ( (error = dsmSetPanning(0, panning)) != OK )
        MIDASerror(error);
    sample = CreateSample(sampleType, 44100);
    PlaySample(sample, 32000, 64);

    for ( i = 0; i < 50; i++ )
    {
        t = benchGetTime();
        if ( (error = dsmMixData(44100, buf)) != OK )
            MIDASerror(error);
        t = benchGetTime() - t - overhead;
        total = total + t;
        if ( min > t )
            min = t;
    }

    StopSample();

#ifdef HAVE_CYCLE_COUNT
    PlaySample(sample, 32000, 64);
    dsmMixData(31, buf);
    mixLoop = dsmActMixRoutines->routines[sampleType].mainMixLoop;

    minm = 1000000; totalm = 0;
    for ( i = 0; i < 128; i++ )
    {
        dsmMixData(1, buf);        
        c = benchGetCycleCount();
        mixLoop(1024, 0);
        c = benchGetCycleCount() - c;
        c = c - emptyCallCycles;
        if ( minm > c )
            minm = c;
        totalm = totalm + c;
    }

    StopSample();
#endif   
    
    if ( (error = dsmRemoveSample(sample)) != OK )
        MIDASerror(error);

    if ( (error = dsmClose()) != OK )
        MIDASerror(error);

    printf("Single channel at 44100:  Min %.0f, Avg %.0f, %.2f%% of CPU\n",
           min, total / 50.0, 100.0 * ((total / 50.0) / 1000000.0));
    min -= mixEmpty;
    total -= mixEmpty * 50.0;
    printf("In addition to empty mix: Min %.0f, Avg %.0f, %.2f%% of CPU\n",
           min, total / 50.0, 100.0 * ((total / 50.0) / 1000000.0));

#ifdef HAVE_CYCLE_COUNT
    printf("Mixing 1024: Min %u, Avg %u cycles, Min %u Avg %u per sample\n",
           minm, totalm/128, minm/1024, totalm/(128*1024));
#endif    
}



int main(void)
{
    Sleep(500);
    SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    MIDASstartup();
    MIDASsetOption(MIDAS_OPTION_FILTER_MODE, MIDAS_FILTER_NONE);
#ifdef M_PROF
    mProfInit();
#endif

    buf = malloc(4*44100);    

    CheckOverhead();

    /*
    TestEmptyMix(dsmMix16bitStereo);
    TestOneChannelMix(smp8bitMono, dsmMix16bitStereo);
    TestOneChannelMix(smp16bitMono, dsmMix16bitStereo);
    TestEmptyMix(dsmMix16bitMono);
    TestOneChannelMix(smp8bitMono, dsmMix16bitMono);
    TestOneChannelMix(smp16bitMono, dsmMix16bitMono);
    */
    TestEmptyMix(dsmMixInteger | dsmMix16bitMono);
    TestOneChannelMix(smp8bitMono, dsmMixInteger | dsmMix16bitMono, -32);
    TestOneChannelMix(smp16bitMono, dsmMixInteger | dsmMix16bitMono, -32);
    TestEmptyMix(dsmMixInteger | dsmMix16bitStereo);
    TestOneChannelMix(smp8bitMono, dsmMixInteger | dsmMix16bitStereo, -32);
    TestOneChannelMix(smp8bitMono, dsmMixInteger | dsmMix16bitStereo, 0);
    TestOneChannelMix(smp16bitMono, dsmMixInteger | dsmMix16bitStereo, -32);
    TestOneChannelMix(smp16bitMono, dsmMixInteger | dsmMix16bitStereo, 0);
    
    TestEmptyMix(dsmMix16bitMono | dsmMixInterpolation);
    TestOneChannelMix(smp8bitMono, dsmMix16bitMono | dsmMixInterpolation, 32);
    TestOneChannelMix(smp16bitMono, dsmMix16bitMono | dsmMixInterpolation, 32);
    TestEmptyMix(dsmMix16bitStereo | dsmMixInterpolation);
    TestOneChannelMix(smp8bitMono, dsmMix16bitStereo | dsmMixInterpolation, 32);
    TestOneChannelMix(smp16bitMono, dsmMix16bitStereo | dsmMixInterpolation, 32);
    
    return 0;
}
