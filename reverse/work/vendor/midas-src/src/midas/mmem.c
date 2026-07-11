/*      MMEM.C
 *
 * MIDAS Digital Audio System memory handling routines
 *
 * $Id: mmem.c,v 1.8.2.1 1997/09/22 14:48:34 pekangas Exp $
 *
 * Copyright 1996,1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "lang.h"
#include "errors.h"
#include "mmem.h"

RCSID(const char *mmem_rcsid = "$Id: mmem.c,v 1.8.2.1 1997/09/22 14:48:34 pekangas Exp $";)


/****************************************************************************\
*
* Function:     int memAlloc(unsigned len, void **blk);
*
* Description:  Allocates a block of conventional memory
*
* Input:        unsigned len            Memory block length in bytes
*               void **blk              Pointer to memory block pointer
*
* Returns:      MIDAS error code.
*               Pointer to allocated block stored in *blk, NULL if error.
*
\****************************************************************************/

#ifdef DEBUG    
int CALLING _memAlloc(unsigned len, void **blk, char *file, unsigned line)
#else
int CALLING memAlloc(unsigned len, void **blk)
#endif    
{
#ifdef DEBUG
    unsigned    cnt, *b;
    U8          *d;
#endif

    /* check that block length is not zero: */
    if ( len == 0 )
    {
        ERROR(errInvalidBlock, ID_memAlloc);
        return errInvalidBlock;
    }

#ifdef DEBUG
    len = (len + 2*sizeof(U32) + 16 + 3) & 0xFFFFFFFC;
    cnt = len >> 2;
#endif

    /* allocate memory: */
    *blk = malloc(len);

    if ( *blk == NULL )
    {
        /* Memory allocation failed - check if heap is corrupted. If not,
           assume out of memory: */
#if (!defined(__UNIX__)) && (!defined(__DJGPP__))
#if defined(__WATCOMC__) || defined(__VC32__)
        if ( _heapchk() != _HEAPOK )
#else
        if ( heapcheck() != _HEAPOK )
#endif
        {
            ERROR(errHeapCorrupted, ID_memAlloc);
            return errHeapCorrupted;
        }
        else
#endif
        {
            ERROR(errOutOfMemory, ID_memAlloc);
            return errOutOfMemory;
        }
    }

#ifdef DEBUG
    b = (unsigned*) *blk;
    while ( cnt )
    {
        *(b++) = 0xDEADBEEF;
        cnt--;
    }

    d = ((U8*) *blk) + len - sizeof(unsigned);
    *((U32*)d) = 0x31337F00;
    d -= sizeof(unsigned);
    *((U32*)d) = line;
    d -= 16;
    strncpy(d, file, 15);
    *(d+15) = 0;
#endif

    /* memory allocated successfully */
    return OK;
}



/****************************************************************************\
*
* Function:     int memFree(void *blk);
*
* Description:  Deallocates a memory block allocated with memAlloc()
*
* Input:        void *blk               Memory block pointer
*
* Returns:      MIDAS error code.
*
\****************************************************************************/

int CALLING memFree(void *blk)
{
    /* Check that block pointer is not NULL: */
    if ( blk == NULL )
    {
        ERROR(errInvalidBlock, ID_memFree);
        return errInvalidBlock;
    }

    /* deallocate block: */
    free(blk);

    /* deallocation successful */
    return OK;
}


/*
 * $Log: mmem.c,v $
 * Revision 1.8.2.1  1997/09/22 14:48:34  pekangas
 * Added debug-allocation routines
 *
 * Revision 1.8  1997/07/31 10:56:52  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.7  1997/06/02 00:54:16  jpaana
 * Changed most __LINUX__ defines to __UNIX__ for generic *nix porting
 *
 * Revision 1.6  1997/02/27 16:04:54  pekangas
 * Added DJGPP support
 *
 * Revision 1.5  1997/02/06 12:43:57  pekangas
 * Fixed to work with lang.h, cleaned include up
 *
 * Revision 1.4  1997/01/16 18:41:59  pekangas
 * Changed copyright messages to Housemarque
 *
 * Revision 1.3  1996/07/13 18:21:05  pekangas
 * Fixed to compile with Visual C
 *
 * Revision 1.2  1996/05/24 16:20:36  jpaana
 * Fixed for Linux
 *
 * Revision 1.1  1996/05/22 20:49:33  pekangas
 * Initial revision
 *
*/
