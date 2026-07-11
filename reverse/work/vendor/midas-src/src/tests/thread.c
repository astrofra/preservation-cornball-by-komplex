/*      thread.c
 *
 * MIDAS thread test
 *
 * This program misuses the MIDAS DLL API from different threads as badly
 * as possible, attempting to break everything. It shouldn't.
 *
 * $Id: thread.c,v 1.7 1997/08/18 16:51:07 pekangas Exp $
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
#include <conio.h>
#include <process.h>
#include "midas.h"
#include "midasdll.h"

#define MODULE1 "f:\\music\\wrlddrgs.xm"
#define MODULE2 "f:\\music\\effects.s3m"
#define SAMPLE1 "..\\..\\samples\\data\\explosi1.pcm"
#define SAMPLE2 "e:\\powerups.sw"
#define STREAM1 "e:\\fable-stereo.wav"

/*
#define MODULE1 "e:\\music\\wrlddrgs.xm"
#define MODULE2 "e:\\music\\effects.s3m"
#define SAMPLE1 "..\\..\\samples\\data\\explosi1.pcm"
#define SAMPLE2 "e:\\treeinfo.ncd"
#define STREAM1 "e:\\track1.mp2"
*/

#define BUFFERSIZE 32768


MIDASmodule module1;
MIDASmodulePlayHandle playHandle1;
MIDASmodule module2;
MIDASmodulePlayHandle playHandle2;
MIDASsample sample1;
MIDASsample sample2;
MIDASstreamHandle stream1;

unsigned feedCount = 0, feedMin = 1000000, feedMax = 0, feedTotal = 0;
unsigned pollCount = 0, pollMin = 1000000, pollMax = 0, pollTotal = 0;

#ifdef __WATCOMC__

unsigned GetCycleCount(void);
#pragma aux GetCycleCount = \
    ".586" \
    "rdtsc" \
    value [eax] \
    modify exact [eax edx];

#else

unsigned GetCycleCount(void)
{
    return 0;
}

#endif



void MIDASerror(void)
{
    printf("MIDAS error: %s\n", MIDASgetErrorMessage(MIDASgetLastError()));

    MIDASclose();

#ifdef DEBUG
    errPrintList();
#endif

    exit(EXIT_FAILURE);
}



void Error(char *msg)
{
    printf("Error: %s\n", msg);

    MIDASclose();

    exit(EXIT_FAILURE);
}



void RandomPriority(void)
{
    switch ( rand() % 7 )
    {
        case 0:
            SetThreadPriority(GetCurrentThread(),
                              THREAD_PRIORITY_TIME_CRITICAL);
            break;
                    
        case 1:
            SetThreadPriority(GetCurrentThread(),
                              THREAD_PRIORITY_NORMAL);
            break;
                    
        case 2:
            SetThreadPriority(GetCurrentThread(),
                              THREAD_PRIORITY_LOWEST);
            break;
                    
        case 3:
            SetThreadPriority(GetCurrentThread(),
                              THREAD_PRIORITY_IDLE);
            break;
                    
        case 4:
            SetThreadPriority(GetCurrentThread(),
                              THREAD_PRIORITY_HIGHEST);
            break;

        case 5:
            SetThreadPriority(GetCurrentThread(),
                              THREAD_PRIORITY_BELOW_NORMAL);
            break;
                    
        case 6:
            SetThreadPriority(GetCurrentThread(),
                              THREAD_PRIORITY_ABOVE_NORMAL);
            break;
    }
}



HWND        consoleHwnd = NULL;


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    char txt[256];

    if ( GetWindowText(hwnd, txt, 255) )
    {
        if ( !strcmp(txt, "DirectSound TestiKala") )
        {
            consoleHwnd = hwnd;
            return FALSE;
        }
    }

    return TRUE;
}






volatile int stopPoll = 0;


#ifdef _MSC_VER
static void __cdecl PollThread(void *dummy)
#else
void PollThread(void *dummy)
#endif
{
    unsigned c;
    
    dummy = dummy;

    while ( !stopPoll )
    {
        c = GetCycleCount();
        if ( !MIDASpoll() )
            MIDASerror();
        c = GetCycleCount() - c;
        if ( pollCount > 100 )
        {
            if ( c < pollMin )
                pollMin = c;
            if ( c > pollMax )
                pollMax = c;
            pollTotal += c;
        }
        pollCount++;
        Sleep(rand() % 40);
        if ( (rand() % 8) < 3 )
            RandomPriority();
    }

    stopPoll = 0;

    _endthread();
}




int main(int argc, char *argv[])
{
    char        oldTitle[256];
    unsigned    i;
/*    char        *foo;*/

    MIDASstartup();
    
    /* Keep the test somewhat reproducable: */
    srand(17);

    setbuf(stdout, NULL);

    puts("MIDAS stream test\n\n"
         "The sound output may sound strange, but the program should _NOT_ "
         "crash");

    puts("Trying to get window handle");

    /* Hack - try to figure out our console window's window handle: */
    GetConsoleTitle(oldTitle, 255);
    SetConsoleTitle("DirectSound TestiKala");
    Sleep(250);
    EnumWindows(&EnumWindowsProc, (LPARAM) NULL);
    SetConsoleTitle(oldTitle);

    printf("%P, %u\n", consoleHwnd, sizeof(HWND));

    if ( consoleHwnd == NULL )
        Error("Couldn't find window handle");

    puts("Trying to use DirectSound primary buffer mode");
    MIDASsetOption(MIDAS_OPTION_DSOUND_HWND, (DWORD) consoleHwnd);
    MIDASsetOption(MIDAS_OPTION_DSOUND_MODE, MIDAS_DSOUND_PRIMARY);

    /* Initialize MIDAS and start background playback: */
    if ( !MIDASinit() )
        MIDASerror();
/*    if ( !MIDASstartBackgroundPlay(0) )
        MIDASerror();*/

#ifdef _MSC_VER   
    if ( _beginthread(PollThread, 4096, NULL) == -1 )
        Error("Unable to create polling thread");
#else    
    if ( _beginthread(PollThread, NULL, 4096, NULL) == -1 )
        Error("Unable to create polling thread");
#endif    

    /* Open enough channels: */
    if ( !MIDASopenChannels(32) )
        MIDASerror();

    /* Allocate some channels for automatic effects: */
    if ( !MIDASallocAutoEffectChannels(12) )
        MIDASerror();

    /* Load the initial module and sample: */
    puts("Loading modules");
    if ( (module1 = MIDASloadModule(MODULE1)) == NULL )
        MIDASerror();
    if ( (module2 = MIDASloadModule(MODULE2)) == NULL )
        MIDASerror();
    puts("Loading samples");
    if ( (sample1 = MIDASloadRawSample(SAMPLE1, MIDAS_SAMPLE_8BIT_MONO, FALSE))
         == 0 )
        MIDASerror();
    if ( (sample2 = MIDASloadRawSample(SAMPLE2, MIDAS_SAMPLE_16BIT_MONO,
                                       FALSE)) == 0 )
         MIDASerror();

    /* Swap stuff out: */
    /*
    puts("Touching a LOT of memory");
    if ( (foo = malloc(64*1024*1024)) == NULL )
        Error("Out of memory");
    for ( i = 0; i < (64*1024*1024); i++ )
        foo[i] = (i & 255);
        */

    puts("Playing stream");
    if ( (stream1 = MIDASplayStreamWaveFile(STREAM1, 500, TRUE)) == NULL )
        MIDASerror();

    /* Play the module: */
    puts("Playing module");
    if ( (playHandle1 = MIDASplayModule(module1, TRUE)) == 0 )
        MIDASerror();

    /* Play effects for a while: */
    puts("Playing samples");
    for ( i = 0; i < 1000; i++ )
    {
        switch ( rand() & 8 )
        {
            case 0:
                if ( MIDASplaySample(sample1, MIDAS_CHANNEL_AUTO, 0, 22050 +
                                     ((rand() % 30000) - 15000), rand() % 24,
                                     (rand() % 128) - 64) == 0 )
                    MIDASerror();
                break;

            case 8:
                if ( MIDASplaySample(sample2, MIDAS_CHANNEL_AUTO, 0, 22050 +
                                     ((rand() % 30000) - 15000), rand() % 24,
                                     (rand() % 128) - 64) == 0 )
                    MIDASerror();
                break;
        }
                
        Sleep(rand() % 27);
        if ( (rand() % 8) < 2 )
            RandomPriority();

        printf("%u\n", MIDASgetStreamBytesBuffered(stream1));
    }

    puts("Stopping");

    stopPoll = 1;
    while ( stopPoll )
        Sleep(10);

    if ( !MIDASstopStream(stream1) )
        MIDASerror();
    /* Stop playback and uninitialize MIDAS: */
/*    if ( !MIDAScloseChannels() )
        MIDASerror();*/
/*    if ( !MIDASstopBackgroundPlay() )
        MIDASerror();*/
    if ( !MIDASclose() )
        MIDASerror();

    printf("Poll total %u, min %u, max %u, avg %u, count %u\n", pollTotal,
           pollMin, pollMax, pollTotal/(pollCount-100), pollCount);

    return 0;
}


/*
 * $Log: thread.c,v $
 * Revision 1.7  1997/08/18 16:51:07  pekangas
 * Fixed file names
 *
 * Revision 1.6  1997/07/31 10:56:59  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.5  1997/07/26 14:15:59  pekangas
 * Fixed to work (somewhat) with Visual C
 *
 * Revision 1.4  1997/05/30 18:24:07  pekangas
 * Initial revision
 *
 * Revision 1.3  1997/05/20 15:18:49  pekangas
 * Fixed to work with updated API
 *
 * Revision 1.2  1997/04/08 19:17:25  pekangas
 * Initial revision
 *
*/
