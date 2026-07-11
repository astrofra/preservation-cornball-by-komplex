/*      proftest.c
 *
 * MIDAS profiling support test
 *
 * $Id: proftest.c,v 1.2 1997/07/31 10:56:59 pekangas Exp $
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




int main(void)
{
#ifdef M_PROF    
    int error;
    unsigned i;
    double c;
    mProfSection *s;

    MIDASstartup();

    puts("MIDAS profiling support test\n");

    if ( (error = mProfInit()) != OK )
        mError(error);

    for ( i = 0; i < 1000; i++ )
    {
        M_PROF_START(0, 1);
        M_PROF_END(0);
    }

    puts("sect      cycles rounds  minr  maxr avgc/r  min  max");
    for ( i = 0; i < M_PROF_NUM_SECTIONS; i++ )
    {
        s = &mProfSections[i];
        if ( s->numCalls > 0 )
        {
            c = U64double(s->totalCyclesLow, s->totalCyclesHigh);
            printf("%04X: %10.0f %6u %5u %5u %6.1f %4u %4u\n", i,
                   c, s->totalRounds, s->minRounds, s->maxRounds,
                   c / ((double) s->totalRounds), s->minCyclesPerRound,
                   s->maxCyclesPerRound);
        }
    }

/*sect  cycles     rounds minr  maxr  avgc/r min  max 
  0000: 1234567890 123123 12345 12345 1234.6 1234 1234*/

    if ( (error = mProfClose()) != OK )
        mError(error);
#endif
    return 0;    
}


/*
 * $Log: proftest.c,v $
 * Revision 1.2  1997/07/31 10:56:59  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.1  1997/07/26 14:15:40  pekangas
 * Initial revision
 *
*/
