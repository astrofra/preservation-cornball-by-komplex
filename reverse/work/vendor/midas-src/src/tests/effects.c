/*      effects.c
 *
 * MIDAS sound effects test
 *
 * $Id: effects.c,v 1.2 1997/07/31 10:56:58 pekangas Exp $
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
#include <stdlib.h>
#include <string.h>
#include "testutil.h"
#include "midas.h"
#include "midasdll.h"
#include "mprof.h"


#define EFFECT1FILE "..\\..\\samples\\data\\powerups.wav"
MIDASsample sample1;
MIDASsamplePlayHandle play1;
DWORD channel1;


void MIDASerror(void)
{
    printf("MIDAS error: %s\n", MIDASgetErrorMessage(MIDASgetLastError()));

#ifdef DEBUG
    errPrintList();
#endif

    exit(EXIT_FAILURE);
}


#define TESTIT(x, m) \
    if ( x ) \
    { \
        TEST_PASS(m); \
    } \
    else \
    { \
        TEST_FAIL(m); \
    }          



void TestStatus(void)
{
    TEST_NOTE(("---"));
    TEST_NOTE(("Testing playing status"));
    TEST_NOTE(("---"));

    if ( (play1 = MIDASplaySample(sample1, channel1, 0, 22050, 64, 0)) == 0 )
        MIDASerror();

    TESTIT((MIDASgetSamplePlayStatus(play1) == MIDAS_SAMPLE_PLAYING),
           ("Initial playing status - playing"));

    Sleep(500);

    TESTIT((MIDASgetSamplePlayStatus(play1) == MIDAS_SAMPLE_PLAYING),
           ("Playing status after 500ms - playing"));

    Sleep(1000);

    TESTIT((MIDASgetSamplePlayStatus(play1) == MIDAS_SAMPLE_STOPPED),
           ("Playing status after 1500ms - stopped"));

    if ( !MIDASallocAutoEffectChannels(4) )
        MIDASerror();

    if ( (play1 = MIDASplaySample(sample1, MIDAS_CHANNEL_AUTO, 0, 22050, 64,
                                  0)) == 0 )
        MIDASerror(); 
    if ( MIDASplaySample(sample1, MIDAS_CHANNEL_AUTO, 1, 22050, 64, 0) == 0 )
        MIDASerror(); 
    if ( MIDASplaySample(sample1, MIDAS_CHANNEL_AUTO, 1, 22050, 64, 0) == 0 )
        MIDASerror(); 
    if ( MIDASplaySample(sample1, MIDAS_CHANNEL_AUTO, 1, 22050, 64, 0) == 0 )
        MIDASerror();

    Sleep(500);

    TESTIT((MIDASgetSamplePlayStatus(play1) == MIDAS_SAMPLE_PLAYING),
           ("Playing status after 3 other samples - playing"));

    if ( MIDASplaySample(sample1, MIDAS_CHANNEL_AUTO, 1, 22050, 64, 0) == 0 )
        MIDASerror();
    
    TESTIT((MIDASgetSamplePlayStatus(play1) == MIDAS_SAMPLE_STOPPED),
           ("Playing status after 4 other samples - stopped"));

    Sleep(1000);    

    if ( !MIDASfreeAutoEffectChannels() )
        MIDASerror();

    if ( !MIDASstopSample(sample1) )
        MIDASerror();
}



int main(int argc, char *argv[])
{
    MIDASstartup();

#ifdef M_PROF
    mProfInit();
#endif

    MIDASsetOption(MIDAS_OPTION_MIXBUFLEN, 250);

    printf("Opening test output log\n");

    if ( argc > 1 )
        StartPrint("effects.log", 1);
    else
        StartPrint("effects.log", 0);

    TEST_NOTE(("====================="));
    TEST_NOTE(("= Sound Effect Test ="));
    TEST_NOTE(("====================="));
    TEST_NOTE(("[-v for verbose]\n"));

    TEST_NOTE(("Initializing MIDAS"));
    if ( !MIDASinit() )
        MIDASerror();
    if ( !MIDASstartBackgroundPlay(0) )
        MIDASerror();
    if ( !MIDASopenChannels(8) )
        MIDASerror();

    TEST_NOTE(("Loading samples"));
    if ( (sample1 = MIDASloadWaveSample(EFFECT1FILE, 0)) == 0 )
        MIDASerror();
    if ( (channel1 = MIDASallocateChannel()) == MIDAS_ILLEGAL_CHANNEL )
        MIDASerror();

    TestStatus();

    TEST_NOTE((""));
    TEST_NOTE(("Deallocating samples"));
    if ( !MIDASfreeChannel(channel1) )
        MIDASerror();
    if ( !MIDASfreeSample(sample1) )
        MIDASerror();

    TEST_NOTE(("Uninitializing MIDAS"));
    if ( !MIDASstopBackgroundPlay() )
        MIDASerror();
    if ( !MIDASclose() )
        MIDASerror();


    TEST_NOTE(("Test finished"));

    if ( failed )
        TestPrintf("!! Test failed !!\n");
    else
        TestPrintf("!! Test passed !!\n");

    EndPrint();

    return 0;
}



/*
 * $Log: effects.c,v $
 * Revision 1.2  1997/07/31 10:56:58  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.1  1997/07/29 16:58:14  pekangas
 * Initial revision
 *
*/

