/*      testprt.c
 *
 * Common test utility functions
 *
 * $Id: testutil.c,v 1.5 1997/07/31 10:56:59 pekangas Exp $
 *
 * Copyright 1996 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
 * Copyright 1996 Petteri Kangaslampi
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>
#include "midas.h"
#include "midasdll.h"
#include "testutil.h"


static FILE     *logFile = NULL;

int failed = 0;
int testVerbose = 0;



void FatalError(char *str)
{
    if ( logFile != NULL )
        fclose(logFile);
    fprintf(stderr, "FATAL ERROR: %s\n", str);
    MIDASclose();
    exit(EXIT_FAILURE);
}


void FatalMidasError(int error)
{
#ifdef DEBUG
    errPrintList();
#endif
    FatalError(errorMsg[error]);
}


void StartPrint(char *logFileName, int verbose)
{
    testVerbose = verbose;
    if ( (logFile = fopen(logFileName, "wt")) == NULL )
        FatalError("Unable to open log file");
    TestPrintf("--- Log file \"%s\" opened ---\n", logFileName);
}


void EndPrint(void)
{
    if ( logFile == NULL )
        FatalError("EndPrint: Log file not open");
    TestPrintf("--- Closing log file ---\n");
    fclose(logFile);
    logFile = NULL;
}



void TestPrintf(char *format, ...)
{
    static va_list arglist;

    if ( logFile == NULL )
        FatalError("TestPrintf: Log file not open");

    va_start(arglist, format);
    vfprintf(logFile, format, arglist);
    va_end(arglist);
    if ( testVerbose )
    {
        va_start(arglist, format);
        vprintf(format, arglist);
        va_end(arglist);
    }
}


void TestPrintfDisp(char *format, ...)
{
    static va_list arglist;

    if ( logFile == NULL )
        FatalError("TestPrintf: Log file not open");

    va_start(arglist, format);
    vfprintf(logFile, format, arglist);
    va_end(arglist);
    va_start(arglist, format);
    vprintf(format, arglist);
    va_end(arglist);
}



void CheckHeap(void)
{
#if defined(__WC32__) || defined(__VC32__)    
    int         result = _heapchk();

    if ( (result != _HEAPOK) && (result != _HEAPEMPTY) )
        FatalError("HEAP CORRUPTED!");
#else
#endif
}


unsigned MemUsed(void)
{
#if defined(__WC32__) || defined(__VC32__)
    static struct _heapinfo h_info;
    int         heap_status;
    ulong       used = 0;

    h_info._pentry = NULL;

    while ( 1 )
    {
        heap_status = _heapwalk(&h_info);
        if ( heap_status != _HEAPOK )
            break;
        if ( h_info._useflag == _USEDENTRY )
            used += h_info._size;
    }

    if ( (heap_status != _HEAPEND) && (heap_status != _HEAPEMPTY) )
        FatalError("HEAP CORRUPTED!");

    return used;
#else
    return 0;
#endif
}



/*
 * $Log: testutil.c,v $
 * Revision 1.5  1997/07/31 10:56:59  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.4  1997/07/30 14:18:03  pekangas
 * Cleaned up stdout output in non-verbose mode
 * Made all messages go to the log file, even in non-verbose mode
 *
 * Revision 1.3  1997/07/28 13:18:25  pekangas
 * Fixed to compile in DOS, both with Watcom C and GCC
 *
 * Revision 1.2  1997/07/24 23:14:45  pekangas
 * Added support for verbose on/off
 *
 * Revision 1.1  1997/05/30 18:24:07  pekangas
 * Initial revision
 *
*/
