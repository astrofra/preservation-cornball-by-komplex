/*      bench.c
 *
 * Benchmarking helper functions
 *
 * $Id: bench.c,v 1.2 1997/07/31 10:56:32 pekangas Exp $
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
#include <mmsystem.h>
#include <stdio.h>
#include "bench.h"


static double   pcFreq = 0.0;
static int      usePerfCount = 2;       /* don't know yet :) */


/****************************************************************************\
*
* Function:     double LIdouble(LARGE_INTEGER *li)
*
* Description:  Converts a Win32 large integer to a double
*
* Input:        LARGE_INTEGER *li       pointer to the large integer structure
*                                       (assumed to be positive)
*
* Returns:      double with the same value as li
*
\****************************************************************************/

double LIdouble(LARGE_INTEGER *li)
{
    double      t;

    /* Hack, works on little-endian 32-bit systems (such as the x86): */
    t = ((double) (*((unsigned long*) li))) + 4294967296.0 *
        ((double) (*((unsigned long*)(((unsigned char*)li) + 4))));

    return t;
}


/****************************************************************************\
*
* Function:     benchTime benchGetTime(void);
*
* Description:  Reads the current time, in microseconds.
*
* Returns:      Current time in microseconds
*
\****************************************************************************/

benchTime benchGetTime(void)
{
    LARGE_INTEGER li;
    benchTime   btime;

    /* If we don't know the performance counter frequency yet, get it: */
    if ( (usePerfCount == 2) && (pcFreq == 0.0) )
    {
        if ( QueryPerformanceFrequency(&li) )
        {
            pcFreq = LIdouble(&li);
            usePerfCount = 1;
            printf("pcFreq: %f\n", pcFreq);
        }
        else
        {
            printf("Using timeGetTime()");
            usePerfCount = 0;
        }
    }

    /* Get the time: */
    if ( usePerfCount )
    {
        QueryPerformanceCounter(&li);
        btime = 1000000.0 * LIdouble(&li) / pcFreq;
    }
    else
    {
        btime = timeGetTime() * 1000.0;
    }

    return btime;
}