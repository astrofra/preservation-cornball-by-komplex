/*      mprof.c
 *
 * MIDAS internal profiling support
 *
 * $Id: mprof.c,v 1.2 1997/07/31 10:56:53 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include <string.h>
#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#include "mmem.h"
#include "mprof.h"


mProfSection *mProfSections;
static void *mProfSectionMem;



/****************************************************************************\
*
* Function:     int mProfInit(void)
*
* Description:  Initializes profiling support
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mProfInit(void)
{
    int error;
    unsigned i;
    mProfSection *s;
    
    /* Allocate memory for the section structures and align them at a 64-byte
       boundary: */
    if ( (error = memAlloc(M_PROF_NUM_SECTIONS * sizeof(mProfSection) + 63,
                           &mProfSectionMem)) != OK )
        PASSERROR(ID_mProfInit);
    mProfSections = (mProfSection*)((((unsigned) mProfSectionMem) + 63) &
                                    (~63));

    /* Clear them: (zero is a reasonable default value for most of the
       fields) */
    memset(mProfSections, 0, M_PROF_NUM_SECTIONS * sizeof(mProfSection));

    /* Reset the rest to better defaults: */
    s = mProfSections;
    i = M_PROF_NUM_SECTIONS;
    while ( i )
    {
        s->minRounds = 0xFFFFFFFF;
        s->minCyclesPerRound = 0xFFFFFFFF;
        i--;
        s++;
    }

    return OK;
}




/****************************************************************************\
*
* Function:     int mProfClose(void)
*
* Description:  Uninitializes profiling support
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mProfClose(void)
{
    int error;
    
    if ( (error = memFree(mProfSectionMem)) != OK )
        PASSERROR(ID_mProfClose);
    mProfSections = NULL;

    return OK;
}


/*
 * $Log: mprof.c,v $
 * Revision 1.2  1997/07/31 10:56:53  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.1  1997/07/23 17:26:49  pekangas
 * Initial revision
 *
*/










