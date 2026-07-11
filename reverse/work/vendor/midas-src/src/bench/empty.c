/*      empty.c
 *
 * Benchmarks polling without sound
 *
 * $Id: empty.c,v 1.2 1997/07/31 10:56:32 pekangas Exp $
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
#include "bench.h"
#include "midasdll.h"


int main(void)
{
    benchTime   totalTime, pollTime, diff;
    int         c;

    MIDASstartup();
    MIDASinit();


    while ( !kbhit() )
    {
        totalTime = benchGetTime();
        pollTime = 0;

        for ( c = 0; c < 100; c++ )
        {
            diff = benchGetTime();
            MIDASpoll();
            pollTime += benchGetTime() - diff;
            Sleep(20);
        }
        totalTime = benchGetTime() - totalTime;

        printf("total %f, poll %f - %f%% in poll\n", totalTime, pollTime,
            100.0 * pollTime / totalTime);
    }

    getch();

    MIDASclose();

    return 0;
}