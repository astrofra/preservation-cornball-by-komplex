/*      empty.c
 *
 * Benchmarks polling without sound
 *
 * $Id: adpcm.c,v 1.3 1997/07/31 10:56:31 pekangas Exp $
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


#define NUMSAMPLES 10000


int main(void)
{
    /* Doesn't work at the moment */
    return 0;
#if 0    
    benchTime   total;
    int         c;
    uchar       *adpcm;
    short       *samples;
    mAdpcmState *state;

    adpcm = malloc(NUMSAMPLES/2);
    samples = malloc(2*NUMSAMPLES);

    /* Generate random data to adpcm - we'll discard a couple of the lowest
       bits returned by rand() as they aren't often too random: */
    for ( c = 0; c < (NUMSAMPLES/2); c++ )
        adpcm[c] = (rand() >> 2) & 0xFF;

    mAdpcmInitState(1, &state);

    printf("Decoding 1-channel data\n");

    while ( !kbhit() )
    {
        total = benchGetTime();
        for ( c = 0; c < 100; c++ )
            mAdpcmDecode(adpcm, samples, NUMSAMPLES/2, state);
        total = benchGetTime() - total;

        printf("Time for 1000000: %.0f, %.0f samples/sec, %.2f%% for 44100\n",
            total, 1000000.0 / (total / 1000000.0),
            100 * ((total / 1000000.0) * 44100.0) / 1000000.0);
    }

    mAdpcmFreeState(state);

    getch();

    mAdpcmInitState(2, &state);

    printf("Decoding 2-channel data\n");

    while ( !kbhit() )
    {
        total = benchGetTime();
        for ( c = 0; c < 100; c++ )
            mAdpcmDecode(adpcm, samples, NUMSAMPLES/2, state);
        total = benchGetTime() - total;

        printf("Time for 1000000: %.0f, %.0f samples/sec, %.2f%% for 44100\n",
            total, 1000000.0 / (total / 1000000.0),
            100 * ((total / 1000000.0) * 88200.0) / 1000000.0);
    }

    mAdpcmFreeState(state);

    getch();

    mAdpcmInitState(1, &state);

    printf("Encoding 1-channel data\n");

    while ( !kbhit() )
    {
        total = benchGetTime();
        for ( c = 0; c < 100; c++ )
            mAdpcmEncode(samples, adpcm, NUMSAMPLES*2, state);
        total = benchGetTime() - total;

        printf("Time for 1000000: %.0f, %.0f samples/sec, %.2f%% for 44100\n",
            total, 1000000.0 / (total / 1000000.0),
            100 * ((total / 1000000.0) * 44100.0) / 1000000.0);
    }

    mAdpcmFreeState(state);

    getch();


    mAdpcmInitState(2, &state);

    printf("Encoding 2-channel data\n");

    while ( !kbhit() )
    {
        total = benchGetTime();
        for ( c = 0; c < 100; c++ )
            mAdpcmEncode(samples, adpcm, NUMSAMPLES*2, state);
        total = benchGetTime() - total;

        printf("Time for 1000000: %.0f, %.0f samples/sec, %.2f%% for 44100\n",
            total, 1000000.0 / (total / 1000000.0),
            100 * ((total / 1000000.0) * 88200.0) / 1000000.0);
    }

    mAdpcmFreeState(state);

    getch();



    return 0;
#endif    
}
