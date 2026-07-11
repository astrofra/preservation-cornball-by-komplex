/*      testutil.h
 *
 * MIDAS test utility functions
 *
 * $Id: testutil.h,v 1.4 1997/07/31 10:56:59 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
 * Copyright 1996 Petteri Kangaslampi
*/


#ifndef __print_h
#define __print_h


extern int failed;
extern int testVerbose;

void FatalError(char *str);
void FatalMidasError(int error);
void StartPrint(char *logFileName, int verbose);
void EndPrint(void);
void TestPrintf(char *format, ...);
void TestPrintfDisp(char *format, ...);

void CheckHeap(void);
unsigned MemUsed(void);


/*
PASS: kalakala.c, 123    - HummaHummaJee
NOTE: kalakala.c, 123    - This shouldn't fail ever
*/

#define TEST_PASS(message) \
    { \
        TestPrintf("PASS: %12s, %-6i - ", __FILE__, __LINE__); \
        TestPrintf message; \
        TestPrintf("\n"); \
    }

#define TEST_FAIL(message) \
    { \
        TestPrintfDisp("FAIL: %12s, %-6i - ", __FILE__, __LINE__); \
        TestPrintfDisp message; \
        TestPrintfDisp("\n"); \
        failed = 1; \
    }

#define TEST_NOTE(message) \
    { \
        TestPrintfDisp("NOTE: %12s, %-6i - ", __FILE__, __LINE__); \
        TestPrintfDisp message; \
        TestPrintfDisp("\n"); \
    }

#define TEST_NOTEV(message) \
    { \
        TestPrintf("NOTE: %12s, %-6i - ", __FILE__, __LINE__); \
        TestPrintf message; \
        TestPrintf("\n"); \
    }

#endif


/*
 * $Log: testutil.h,v $
 * Revision 1.4  1997/07/31 10:56:59  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.3  1997/07/30 14:18:03  pekangas
 * Cleaned up stdout output in non-verbose mode
 * Made all messages go to the log file, even in non-verbose mode
 *
 * Revision 1.2  1997/07/24 23:14:45  pekangas
 * Added support for verbose on/off
 *
 * Revision 1.1  1997/05/30 18:24:07  pekangas
 * Initial revision
 *
*/
