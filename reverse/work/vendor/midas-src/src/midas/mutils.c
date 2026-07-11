/*	MUTILS.C
 *
 * Miscellaneous utility functions for MIDAS Digital Audio System used
 * by various system components.
 *
 * $Id: mutils.c,v 1.6 1997/07/31 10:56:53 pekangas Exp $
 *
 * Copyright 1996,1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/


#include "lang.h"
#include "mutils.h"
#include "string.h"
#include <stdlib.h>
#include <stdio.h>

#ifndef __UNIX__
#include <conio.h>
#endif

/****************************************************************************\
*
* Function:     int mGetKey(void)
*
* Description:  Waits for a keypress and returns the read key
*
* Returns:      ASCII code for the key pressed. Extended keycodes are
*               returned with bit 8 set, eg. up arrow becomes \x148.
*
\****************************************************************************/

int CALLING mGetKey(void)
{
    char c;
#ifdef __UNIX__
#define getch getchar
#endif

    c = getch();

    if ( !c )
	return 0x100+getch();
    else
	return c;
};

/****************************************************************************\
*
* Function:     int mStrLength(const char *str)
*
* Description:  Calculates the length of a ASCIIZ string
*
* Input:        char *str               pointer to string
*
* Returns:      String length excluding the terminating '\0'.
*
\****************************************************************************/

int CALLING mStrLength(const char *str)
{
    return strlen(str);
}

/****************************************************************************\
*
* Function:     void mStrCopy(char *dest, const char *src);
*
* Description:  Copies an ASCIIZ string from *src to *dest.
*
* Input:        char *dest              pointer to destination string
*               char *src               pointer to source string
*
\****************************************************************************/

void CALLING mStrCopy(char *dest, const char *src)
{
    strcpy(dest, src);
}

/****************************************************************************\
*
* Function:     void mStrAppend(char *dest, const char *src);
*
* Description:  Appends an ASCIIZ string to the end of another.
*
* Input:        char *dest              pointer to destination string
*               char *src               pointer to source string
*
\****************************************************************************/

void CALLING mStrAppend(char *dest, const char *src)
{
    strcat(dest, src);
}

/****************************************************************************\
*
* Function:     void mMemCopy(void *dest, const void *src, unsigned numBytes);
*
* Description:  Copies a memory block from *src to *dest.
*
* Input:        void *dest              pointer to destination
*               void *src               pointer to source
*               unsigned numBytes       number of bytes to copy
*
\****************************************************************************/

void CALLING mMemCopy(void *dest, const void *src, unsigned numBytes)
{
    memcpy(dest, src, numBytes);
}


/****************************************************************************\
*
* Function:     int mMemEqual(const void *m1, const void *m2,
*                   unsigned numBytes);
*
* Description:  Compares two memory blocks.
*
* Input:        void *m1                pointer to memory block #1
*               void *m2                pointer to memory block #2
*               unsigned numBytes       number of bytes to compare
*
* Returns:      1 if the memory blocks are equal, 0 if not.
*
\****************************************************************************/

int CALLING mMemEqual(const void *m1, const void *m2, unsigned numBytes)
{
    if ( memcmp(m1, m2, numBytes ) )
      return 0;
    else
      return 1;
}

/****************************************************************************\
*
* Function:     long mHex2Long(const char *hex)
*
* Description:  Converts a hexadecimal string to a long integer.
*
* Input:        char *hex               pointer to hex string, ASCIIZ
*
* Returns:      Value of the string or -1 if conversion failure.
*
\****************************************************************************/

long CALLING mHex2Long(const char *hex)
{
    static unsigned int l;

    if ((sscanf(hex, "%x", &l) ) > 0 )
        return l;
    else
        return -1;
}


/****************************************************************************\
*
* Function:     long mDec2Long(const char *dec)
*
* Description:  Converts an unsigned decimal string to a long integer
*
* Input:        char *dec               pointer to string, ASCIIZ
*
* Returns:      Value of the string or -1 if conversion failure.
*
\****************************************************************************/

long CALLING mDec2Long(const char *dec)
{
    static int l;

    if ( (sscanf(dec, "%d", &l)) > 0 )
        return l;
    else
        return -1;
}


/***************************************************************************\
*
* Function:     char *mGetEnv(const char *envVar);
*
* Description:  Searches a string from the environment
*
* Input:        envVar                  environment variable name, ASCIIZ
*
* Returns:      Pointer to environment string value (ASCIIZ), NULL if string
*               was not found.
*
\***************************************************************************/

char * CALLING mGetEnv(const char *envVar)
{
    return getenv(envVar);
}

/*
 * $Log: mutils.c,v $
 * Revision 1.6  1997/07/31 10:56:53  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.5  1997/06/02 00:54:16  jpaana
 * Changed most __LINUX__ defines to __UNIX__ for generic *nix porting
 *
 * Revision 1.4  1997/02/05 17:40:43  pekangas
 * Changed to new makefile structure. Removed old makefiles, library
 * command files and related junk. Fixed some double linefeeds caused
 * by RCS-CVS transition. lang.h now defined NOTIMER for Win32.
 *
 * Revision 1.3  1997/01/16 18:41:59  pekangas
 * Changed copyright messages to Housemarque
 *
 * Revision 1.2  1996/05/24 20:40:12  jpaana
 * Fixed mMemEqual
 *
 * Revision 1.1  1996/05/24 16:20:36  jpaana
 * Initial revision
 *
 *
*/
