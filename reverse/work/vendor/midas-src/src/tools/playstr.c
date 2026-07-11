/*      playstr.c
 *
 * MIDAS stream file player
 *
 * $Id: playstr.c,v 1.8.2.3 1998/02/26 20:51:53 pekangas Exp $
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
#define IS_WIN32
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "midas.h"
#include "mprof.h"
#include "midasdll.h"

#ifndef __LINUX__
#include <conio.h>
#endif

#if defined(__DOS__) || defined(__DJGPP__)
#define NO_THREADS
#define Sleep delay
#include <dos.h>
#endif

#if defined(__LINUX__)
#define Sleep(x) usleep(1000*x)
#include <unistd.h>
#endif



#define BUFFERSIZE 32768


unsigned        rate = 44100;
int             sampleType = MIDAS_SAMPLE_16BIT_MONO;
MIDASstreamHandle stream = NULL;
unsigned        offset = 0;


void MIDASerror(void)
{
    printf("MIDAS error: %s\n", MIDASgetErrorMessage(MIDASgetLastError()));

    if ( stream != NULL )
        MIDASstopStream(stream);

    MIDASclose();

#ifdef DEBUG
    errPrintList();
#endif

    exit(EXIT_FAILURE);
}



void Error(char *msg)
{
    printf("Error: %s\n", msg);

    if ( stream != NULL )
        MIDASstopStream(stream);

    MIDASclose();

    exit(EXIT_FAILURE);
}


#ifdef M_PROF

double U64double(unsigned low, unsigned high)
{
    return 4294967296.0 * ((double) high) + ((double) low);
}


void PrintResults(void)
{
    unsigned i;
    double c;
    mProfSection *s;
    
    puts("sect       cycles    calls     rounds  avgr  minr  maxr avgc/r  "
         "min  max");
    for ( i = 0; i < M_PROF_NUM_SECTIONS; i++ )
    {
        s = &mProfSections[i];
        if ( s->numCalls > 0 )
        {
            c = U64double(s->totalCyclesLow, s->totalCyclesHigh);
            printf("%04X: %11.0f %8u %10u %5u %5u %5u %6.1f %4u %4u\n", i,
                   c, s->numCalls, s->totalRounds, s->totalRounds/s->numCalls,
                   s->minRounds, s->maxRounds,
                   c / ((double) s->totalRounds), s->minCyclesPerRound,
                   s->maxCyclesPerRound);
        }
    }

}

#endif


int main(int argc, char *argv[])
{
    FILE        *f;
    long        fileLeft;
    unsigned char *buffer, *b;
    unsigned    doNow;
    unsigned    numFed;
    int wave = 0;

    setbuf(stdout, NULL);

    puts("playstr 1.0.0 - MIDAS stream player");

    if ( (argc < 2) || (argc > 5) )
    {
        puts("Usage:  playstr filename [offset] [sampletype] [rate]\n\n"
             "        filename    input file name\n"
             "        offset      byte offset to file (optional)\n"
             "        sampletype  sample type:    1 = 8-bit mono, 2 = 16-bit "
                 "mono,\n"
             "                      (optional)    3 = 8-bit stereo, 4 = "
                 "16-bit stereo,\n"
             "                                    5 = ADPCM mono, 6 = ADPCM "
                 "stereo\n"
             "                                    7 = u-law mono, 8 = u-law "
                 "stereo\n"
             "        rate        sample rate in Hz (optional)");

        return 1;
    }

    /* Try to guess the format from the file extension: */
    if ( strlen(argv[1]) > 6 )
        if ( !memcmp(&argv[1][strlen(argv[1]) - 6], ".adpcm", 6) )
            sampleType = MIDAS_SAMPLE_ADPCM_MONO;
    if ( strlen(argv[1]) > 5 )
        if ( !memcmp(&argv[1][strlen(argv[1]) - 5], ".ulaw", 5) )
            sampleType = MIDAS_SAMPLE_ULAW_MONO;

    /* Is it a wave? */
    if ( strlen(argv[1]) > 4 )
        if ( !memcmp(&argv[1][strlen(argv[1]) - 4], ".wav", 4) )
            wave = 1;

    /* Get optional options: */
    if ( argc > 2 )
        offset = atoi(argv[2]);
    if ( argc > 3 )
        sampleType = atoi(argv[3]);
    if ( argc > 4 )
        rate = atoi(argv[4]);

    /* Initialize MIDAS and start background playback: */
    MIDASstartup();
    if ( !MIDASinit() )
        MIDASerror();
    if ( !MIDASstartBackgroundPlay(0) )
        MIDASerror();

    /* Open a channel for the stream: */
    if ( !MIDASopenChannels(1) )
        MIDASerror();

    /* If the starting offset is not 0, we'll feed the stream manually,
       otherwise we'll just play the whole file. This way we can use this
       program as a test for both of the modes :) */

    /* Also, if the target system doesn't support threads, we'll need to
       feed manually always */

#ifndef NO_THREADS    
    if ( offset )
    {
#endif        
        /* Allocate space for buffer: */
        if ( (buffer = malloc(BUFFERSIZE)) == NULL )
            Error("Out of memory");

        /* Open the file and get its length: */
        if ( (f = fopen(argv[1], "rb")) == NULL )
            Error("Unable to open input file");
        fseek(f, 0, SEEK_END);
        fileLeft = ftell(f);

        /* Seek to starting offset: */
        if ( (long) offset > fileLeft )
            offset = fileLeft;
        fseek(f, offset, SEEK_SET);
        fileLeft -= offset;

        /* Start playing the stream: */
        if ( (stream = MIDASplayStreamPolling(sampleType, rate, 500))
            == NULL )
            MIDASerror();

#ifdef __LINUX__
        puts("Playing - press Ctrl-C");
        while ( 1 )
#else
        puts("Playing - press a key");  
        while ( !kbhit() )
#endif            
        {
            if ( fileLeft > BUFFERSIZE )
                doNow = BUFFERSIZE;
            else
                doNow = fileLeft;

            if ( doNow )
            {
                /* Read data from file: */
                fread(buffer, doNow, 1, f);
                b = buffer;
                fileLeft -= doNow;

                /* Feed until we manage to feed all: */
#ifdef __LINUX__
                while ( doNow )
#else                
                while ( doNow && (!kbhit()) )
#endif                    
                {
                    numFed = MIDASfeedStreamData(stream, b, doNow, FALSE);
                    doNow -= numFed;
                    b += numFed;
                    Sleep(20);
                }
                /*numFed = MIDASfeedStreamData(stream, b, doNow, TRUE);*/
            }
            else
            {
                Sleep(20);
            }
        }

#ifndef __LINUX__        
        getch();
#endif        

        fclose(f);
        free(buffer);
#ifndef NO_THREADS        
    }
    else
    {
        /* Start playing the stream: */
        if ( wave )
        {
            if ( (stream = MIDASplayStreamWaveFile(argv[1], 500, 1)) == NULL )
                MIDASerror();
        }
        else
        {
            if ( (stream = MIDASplayStreamFile(argv[1], sampleType, rate, 500,
                                               1)) == NULL )
                MIDASerror();
        }

#ifdef __LINUX__
        puts("Playing - press Enter");
        getchar();
#else
        puts("Playing - press a key");
        getch();
#endif        
    }
#endif    

    /* Stop stream: */
    if ( !MIDASstopStream(stream) )
        MIDASerror();

    /* Stop playback and uninitialize MIDAS: */
    if ( !MIDAScloseChannels() )
        MIDASerror();
    if ( !MIDASstopBackgroundPlay() )
        MIDASerror();

#ifdef M_PROF
    PrintResults();
#endif
    
    if ( !MIDASclose() )
        MIDASerror();

    return 0;
}


/*
 * $Log: playstr.c,v $
 * Revision 1.8.2.3  1998/02/26 20:51:53  pekangas
 * Fixed to compile and work in Linux
 *
 * Revision 1.8.2.2  1997/09/22 15:56:35  pekangas
 * Fixed support for non-profiling builds
 *
 * Revision 1.8.2.1  1997/08/25 19:39:25  pekangas
 * Added profiling support
 *
 * Revision 1.8  1997/07/31 10:57:00  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.7  1997/05/20 20:34:28  pekangas
 * Added wave support
 *
 * Revision 1.6  1997/05/03 15:11:05  pekangas
 * Fixed to work in DOS
 *
 * Revision 1.5  1997/05/02 13:34:13  pekangas
 * Changed to use automatic channel allocation for streams
 *
 * Revision 1.4  1997/02/20 19:47:38  pekangas
 * Added u-law decoder and updated for u-law
 *
 * Revision 1.3  1997/02/18 20:20:18  pekangas
 * Updated for new ADPCM system - the ADPCM stream now consists of frames,
 * and playing can be started from the beginning of any frame
 *
 * Revision 1.2  1997/02/12 20:39:10  pekangas
 * Fixed a Visual C warning
 *
 * Revision 1.1  1997/02/12 16:26:39  pekangas
 * Initial revision
 *
*/
