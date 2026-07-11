/*      mixsd.c
 *
 * Miscellaneous helper functions common to all mixing sound devices.
 * Technically these functions should be part of each Sound Device's internal
 * code, but are here to save some space and help maintenance.
 *
 * $Id: mixsd.c,v 1.6 1997/07/31 10:56:51 pekangas Exp $
 *
 * Copyright 1996,1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#ifdef __DJGPP__
#include <stdlib.h>
#include <sys/movedata.h>
#endif

#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#include "mmem.h"
#include "mixsd.h"
#include "sdevice.h"
#include "dsm.h"
#include "dma.h"
#include "mglobals.h"

RCSID(const char *mixsd_rcsid = "$Id: mixsd.c,v 1.6 1997/07/31 10:56:51 pekangas Exp $";)


//#define DUMPBUFFER



unsigned dmaBufferSize;                 /* DMA playback buffer size */
//static dmaBuffer buffer;                /* DMA playback buffer */
dmaBuffer buffer;
static unsigned mixMode;                /* mixing mode */
static unsigned mixRate;                /* mixing rate */
static unsigned outputMode;             /* output mode */
static unsigned updateMix;              /* number of elements to mix between
                                           two updates */
static unsigned mixLeft;                /* number of elements to mix before
                                           next update */
//static unsigned dmaPos;                 /* DMA position inside buffer */
//static unsigned mixPos;                 /* mixing position */
unsigned dmaPos, mixPos;
int             playDMA;                /* Playing through DMA -flag */

uchar    *bufferPtr;                    /* the target buffer */

static unsigned dmaElemSize;            /* DMA buffer element size */

#ifdef DUMPBUFFER
#include <stdio.h>
static FILE     *f;
#endif




/****************************************************************************\
*
* Function:     int mixsdInit(unsigned mixRate, unsigned mode, unsigned
*                   dmaChNum)
*
* Description:  Common initialization for all mixing Sound Devices.
*               Initializes DMA functions, DSM, starts DMA playback and
*               allocates memory for possible post-processing tables
*
* Input:        unsigned mixRate        mixing rate in Hz
*               unsigned mode           output mode
*               int dmaChNum            DMA channel number / -1 if not to be
*                                       played yet
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mixsdInit(unsigned _mixRate, unsigned mode, int dmaChNum)
{
    int         error;
    U8          *p, val;
    unsigned    i;

    mixRate = _mixRate;
    outputMode = mode;
    mixPos = 0;

    /* Calculate required DMA buffer size: (1/25th of a second) */
    dmaBufferSize = mixRate / DMABUFLEN;
//    dmaBufferSize = mixRate / 10;
    dmaElemSize = 1;

    /* Multiply by 2 if stereo: */
    if ( mode & sdStereo )
    {
        dmaBufferSize *= 2;
        dmaElemSize *= 2;
    }

    /* Multiply by 2 if 16-bit: */
    if ( mode & sd16bit )
    {
        dmaBufferSize *= 2;
        dmaElemSize *= 2;
    }

    /* Make buffer length a multiple of 32 (helps with GUS): */
    dmaBufferSize = (dmaBufferSize + 31) & (~31);

    /* Allocate DMA buffer: */
    if ( (error = dmaAllocBuffer(dmaBufferSize, &buffer)) != OK )
        PASSERROR(ID_mixsdInit)

#ifdef __DJGPP__
    /* Get a separate buffer where we'll mix first, and copy to the real
       DMA buffer afterwards (accessing the DMA buffer is painful with
       DJGPP) */
    if ( (error = memAlloc(dmaBufferSize, (void**) &bufferPtr)) != OK )
        PASSERROR(ID_mixsdInit)
#else
    bufferPtr = buffer.dataPtr;
#endif

    /* Now clear the DMA buffer to avoid nasty clicks */

    /* Check zero value: */
    if ( mode & sd8bit )
        val = 0x80;
    else
        val = 0;

    /* And clear it: */
    for ( p = (U8*) bufferPtr, i = 0; i < dmaBufferSize; i++,
        *(p++) = val );

#ifdef __DJGPP__
    dosmemput(bufferPtr, dmaBufferSize, buffer.startAddr);
#endif

    /* Check correct mixing mode: */
    if ( mode & sdStereo )        
        mixMode = dsmMixStereo;
    else
        mixMode = dsmMixMono;
    if ( mode & sd8bit )
        mixMode |= dsmMix8bit;
    else
        mixMode |= dsmMix16bit;
    if ( mOversampling )
        mixMode |= dsmMixInterpolation;

    if ( (error = dsmInit(mixRate, mixMode)) != OK )
        PASSERROR(ID_mixsdInit);

    if ( dmaChNum != -1 )
    {
        /* Start playing the DMA buffer: */
        if ( (error = dmaPlayBuffer(&buffer, dmaChNum, 1)) != OK )
            PASSERROR(ID_mixsdInit)
        playDMA = 1;
    }
    else
    {
        playDMA = 0;
    }

    /* Set update rate to 50Hz: */
    if ( (error = mixsdSetUpdRate(5000)) != OK )
        PASSERROR(ID_mixsdInit)

#ifdef DUMPBUFFER
    f = fopen("dmabuffer.smp", "wb");
#endif

    return OK;
}




/****************************************************************************\
*
* Function:     int CALLING mixsdClose(void)
*
* Description:  Common uninitialization code for all mixing Sound Devices.
*               Uninitializes DMA playback and DSM and deallocates memory.
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mixsdClose(void)
{
    int         error;

    if ( playDMA == 1 )
    {
        /* Stop DMA playback: */
        if ( (error = dmaStop(buffer.channel)) != OK )
            PASSERROR(ID_mixsdClose)
    }

    /* Uninitialize Digital Sound Mixer: */
    if ( (error = dsmClose()) != OK )
        PASSERROR(ID_mixsdClose)

    /* Deallocate DMA buffer: */
    if ( (error = dmaFreeBuffer(&buffer)) != OK )
        PASSERROR(ID_mixsdClose)

#ifdef __DJGPP__
    /* Deallocate our dummy buffer: */
    if ( (error = memFree(bufferPtr)) != OK )
        PASSERROR(ID_mixsdClose)
#endif

#ifdef DUMPBUFFER
    fclose(f);
#endif

    return OK;
}




/****************************************************************************\
*
* Function:     int mixsdGetMode(unsigned *mode)
*
* Description:  Reads the current output mode
*
* Input:        unsigned *mode          pointer to output mode
*
* Returns:      MIDAS error code. Output mode is written to *mode.
*
\****************************************************************************/

int CALLING mixsdGetMode(unsigned *mode)
{
    *mode = outputMode;

    return OK;
}




/****************************************************************************\
*
* Function:     int mixsdSetUpdRate(unsigned updRate);
*
* Description:  Sets the channel value update rate (depends on song tempo)
*
* Input:        unsigned updRate        update rate in 100*Hz (eg. 50Hz
*                                       becomes 5000).
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mixsdSetUpdRate(unsigned updRate)
{
    /* Calculate number of elements to mix between two updates: (even) */
    mixLeft = updateMix = ((unsigned) ((100L * (ulong) mixRate) /
        ((ulong) updRate)) + 1) & 0xFFFFFFFE;

    return OK;
}




/****************************************************************************\
*
* Function:     int mixsdStartPlay(void)
*
* Description:  Prepares for playing - reads DMA playing position. Called
*               once before the Sound Device and music player polling loop.
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mixsdStartPlay(void)
{
    int         error;

    /* Read DMA playing position: */
    if ( (error = dmaGetPos(&buffer, &dmaPos)) != OK )
        PASSERROR(ID_mixsdStartPlay)

    return OK;
}




/****************************************************************************\
*
* Function:     int mixsdPlay(int *callMP);
*
* Description:  Plays the sound - mixes the correct amount of data with DSM
*               and copies it to DMA buffer with post-processing.
*
* Input:        int *callMP             pointer to music player calling flag
*
* Returns:      MIDAS error code. If enough data was mixed for one updating
*               round and music player should be called, 1 is written to
*               *callMP, otherwise 0 is written there. Note that if music
*               player can be called, mixsdPlay() should be called again
*               with a new check for music playing to ensure the DMA buffer
*               gets filled with new data.
*
\****************************************************************************/

int CALLING mixsdPlay(int *callMP)
{
    int         error;
    int         numElems, mixNow;
    int         dmaBufLeft;

    /* Calculate number of bytes that fits in the buffer: */
    if ( dmaPos >= mixPos )
        numElems = dmaPos - mixPos - 16;
    else
        numElems = dmaBufferSize - mixPos + dmaPos - 16;

    /* Do not mix less than 16 bytes: */
    if ( numElems < 16 )
    {
        *callMP = 0;
        return OK;
    }

    /* Calculate actual number of elements: */
    if ( outputMode & sdStereo )
        numElems >>= 1;
    if ( outputMode & sd16bit )
        numElems >>= 1;

    /* Make sure number of elements is even: */
    numElems = numElems & 0xFFFFFFFE;

    /* Check that we won't mix more elements than there is left before next
       update: */
    if ( numElems > mixLeft )
        numElems = mixLeft;

    /* Decrease number of elements before next update: */
    mixLeft -= numElems;

    /* Mix all the data and post-process it to the DMA buffer, making sure
       DSM mixing buffer will not overflow: */
    while ( numElems )
    {
        /* Calculate the number of elements left in DMA buffer before
           buffer end, and don't mix more than that: */
        mixNow = numElems;
        dmaBufLeft = dmaBufferSize - mixPos;
        if ( outputMode & sdStereo )
            dmaBufLeft >>= 1;
        if ( outputMode & sd16bit )
            dmaBufLeft >>= 1;
        if ( mixNow > dmaBufLeft )
            mixNow = dmaBufLeft;

        /* Decrease number of elements left: */
        numElems -= mixNow;

        /* Mix it! */
        if ( (error = dsmMixData(mixNow, bufferPtr + mixPos)) != OK )
            PASSERROR(ID_mixsdPlay);

#ifdef __DJGPP__
        dosmemput(bufferPtr + mixPos, dmaElemSize * mixNow,
                  buffer.startAddr + mixPos);
#endif

#ifdef DUMPBUFFER
        fwrite(bufferPtr + mixPos, dmaElemSize * mixNow, 1, f);
#endif

        mixPos += dmaElemSize * mixNow;

        if ( mixPos >= dmaBufferSize )
            mixPos = 0;
    }

    /* Check if music player should be called: */
    if ( mixLeft == 0 )
    {
        mixLeft = updateMix;
        *callMP = 1;
    }
    else
    {
        *callMP = 0;
    }

    return OK;
}


/*
 * $Log: mixsd.c,v $
 * Revision 1.6  1997/07/31 10:56:51  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.5  1997/07/29 13:16:54  pekangas
 * Fixed to compile with DJGPP without warnings
 *
 * Revision 1.4  1997/06/20 10:08:22  pekangas
 * Fixed to work with new mixing routines
 *
 * Revision 1.3  1997/02/27 16:04:37  pekangas
 * Added DJGPP support, small fixes for new GUS software mixing SD
 *
 * Revision 1.2  1997/01/16 18:41:59  pekangas
 * Changed copyright messages to Housemarque
 *
 * Revision 1.1  1996/05/22 20:49:33  pekangas
 * Initial revision
 *
*/
