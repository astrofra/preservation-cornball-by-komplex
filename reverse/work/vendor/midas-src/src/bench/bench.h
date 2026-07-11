/*      bench.h
 *
 * Benchmarking helper functions
 *
 * $Id: bench.h,v 1.3.2.1 1997/08/25 19:38:29 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#ifndef __bench_h
#define __bench_h



/* The time type: */
typedef double benchTime;


#ifdef __WATCOMC__

#define HAVE_CYCLE_COUNT

unsigned benchGetCycleCount(void);
#pragma aux benchGetCycleCount = \
    ".586" \
    "rdtsc" \
    value [eax] \
    modify exact [eax edx];

#endif


#ifdef _MSC_VER

#define HAVE_CYCLE_COUNT

__inline unsigned benchGetCycleCount(void)
{
    unsigned foo;
    
    __asm {
        __emit 0x0f
        __emit 0x31
        mov foo,eax
    }

    return foo;
}

#endif



#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************\
*
* Function:     benchTime benchGetTime(void);
*
* Description:  Reads the current time, in microseconds.
*
* Returns:      Current time in microseconds
*
\****************************************************************************/

benchTime benchGetTime(void);


#ifdef __cplusplus
}
#endif



#endif


/*
 * $Log: bench.h,v $
 * Revision 1.3.2.1  1997/08/25 19:38:29  pekangas
 * Added Visual C support
 *
 * Revision 1.3  1997/07/31 10:56:32  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.2  1997/05/30 18:22:25  pekangas
 * Added cycle counter reading
 *
 * Revision 1.1  1997/02/12 19:07:35  pekangas
 * Initial revision
 *
*/
