/*      proftest.c
 *
 * MIDAS music playback profiler
 *
 * $Id: playprof.c,v 1.2.2.1 1997/08/25 19:38:56 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#if defined(__WINDOWS__) || defined(__NT__) || defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define _WIN32_
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "midasdll.h"
#include "midas.h"
#include "mprof.h"



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

#ifdef DEBUG
    errPrintList();
#endif    

    exit(EXIT_FAILURE);
}


void mError(int error)
{
    printf("MIDAS error: %s\n", MIDASgetErrorMessage(error));

    MIDASclose();

#ifdef DEBUG
    errPrintList();
#endif

    exit(EXIT_FAILURE);
}



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




int main(int argc, char *argv[])
{
    MIDASmodule module;
    MIDASmodulePlayHandle playHandle;

    MIDASstartup();

#ifdef _WIN32_
    SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#endif    

    puts("MIDAS playback profiler\n");

    if ( argc != 3 )
    {
        puts("Usage: mode filename\n        mode = (f)loat/(i)nteger\n");
        return EXIT_FAILURE;
    }

    MIDASconfig();

    switch ( argv[1][0] )
    {
        case 'i':
            mPreferredMixingMode = M_MIX_INTEGER;
            break;

        case 'f':
            mPreferredMixingMode = M_MIX_FLOAT;
            break;
    }

    if ( !MIDASinit() )
        MIDASerror();
    if ( !MIDASstartBackgroundPlay(0) )
        MIDASerror();
    
    if ( (module = MIDASloadModule(argv[2])) == NULL )
        MIDASerror();
    if ( (playHandle = MIDASplayModule(module, TRUE)) == 0 )
        MIDASerror();

    printf("Playing - press any\n");
    getch();

    if ( !MIDASstopModule(playHandle) )
        MIDASerror();
    if ( !MIDASfreeModule(module) )
        MIDASerror();

    if ( !MIDASstopBackgroundPlay() )
        MIDASerror();

    PrintResults();
    
    if ( !MIDASclose() )
        MIDASerror();

    
    return 0;    
}


/*
 * $Log: playprof.c,v $
 * Revision 1.2.2.1  1997/08/25 19:38:56  pekangas
 * Fixed to work again
 *
 * Revision 1.2  1997/07/31 10:56:32  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.1  1997/07/26 14:18:02  pekangas
 * Initial revision
 *
*/
