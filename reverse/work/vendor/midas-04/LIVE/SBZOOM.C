/*      SBZOOM.C
 *
 * S2 The Party '94 64kb intro
 * -- Shadebob zoomer
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <dos.h>
#include <midas.h>
#include "intro.h"



/****************************************************************************\
*
* Function:     void PutShadebob(ushort bufSeg, int x, int y);
*
* Description:  Puts a shadebob into a buffer
*
* Input:        ushort bufSeg           buffer segment
*               int x                   x-coordinate
*               int y                   y-coordinate
*
\****************************************************************************/

extern void cdecl PutShadebob(ushort bufSeg, int x, int y);



/****************************************************************************\
*
* Function:     void ZoomBuffer(ushort srcSeg, ushort destSeg,
*                   int xOffset int yOffset);
*
* Description:  Zoom data from one buffer to another and to screen
*
* Input:        ushort srcSeg           source buffer segment
*               ushort destSeg          destination buffer segment
*               int xOffset             zooming center X offset
*               int yOffset             zooming center Y offset
*
\****************************************************************************/

extern void cdecl ZoomBuffer(ushort srcSeg, ushort destSeg, int xOffset,
    int yOffset);



extern uchar    sbzPalette[];           /* palette for the zoomer */




/****************************************************************************\
*
* Function:     void ShadebobZoomer(void)
*
* Description:  The Shadedbob-zoomer part
*
\****************************************************************************/

void ShadebobZoomer(void)
{
    uchar       *buf[2];
    ushort      bufSeg[2];
    int         bufNum;
    int         a;
    int         i, aa, oa;
    int         error;
    int         leaving = 0, leave = 0;
    ulong       lfFrame;
    int         b;

    /* Allocate memory for zoomer buffers: */
    if ( (error = memAlloc(64016, (void**) &buf[0])) != OK )
        Error(errorMsg[error]);
    if ( (error = memAlloc(64016, (void**) &buf[1])) != OK )
        Error(errorMsg[error]);

    /* Set buffer segments to point to buffers: */
    bufSeg[0] = FP_SEG(buf[0]) + (FP_OFF(buf[0]) + 15) / 16;
    bufSeg[1] = FP_SEG(buf[1]) + (FP_OFF(buf[1]) + 15) / 16;

    /* Clear buffers: */
    memset(buf[0], 0, 64000);
    memset(buf[1], 0, 64000);

    a = 0;
    oa = 0;
    bufNum = 0;                         /* reset buffer flip/flop */

    scrStart = 0;

    /* Clear display memory: */
    ClearVGA();

    /* Wait until the clear display page must be visible: */
    scrStart = 0; lfFrame = 0;
    WaitFrame();

    /* Set the correct RGB palette: */
    SetPalette(&sbzPalette[0], 0, 256);

    while ( !leave )
    {
        UpdInfo();
        WaitFrame();

        if ( leaving )
        {
            b = frameCount - lfFrame;
            if ( b < 64 )
                FadePalette(&sbzPalette[0], 0, 256, 64-b, fadeTable);
            else
            {
                FadePalette(&sbzPalette[0], 0, 256, 0, fadeTable);
                leave = 1;
            }
        }

        /* Draw the shadebobs into first buffer (buf[bufNum]): */
        aa = 0;
        for ( i = 0; i < 16; i++ )
        {
            PutShadebob(bufSeg[bufNum], 160 - 8 + iSinMult((a+aa) * 5,
                30), 100 - 8 + iCosMult(a+2*aa, 20));
            aa += 173;
            if ( aa > 360 )
                aa -= 360;
        }

        a = a + 7;
        if ( a > 360 )
            a -= 360;

        /* Zoom the buffer contents to another buffer and screen: */
        ZoomBuffer(bufSeg[bufNum], bufSeg[bufNum ^ 1],
            iSinMult(oa, 2), iCosMult(oa, 2));

        oa += 3;
        if ( oa > 360 )
            oa -= 360;

        /* Flip buffer number: */
        bufNum ^= 1;

        if ( (info->pos == 0x1B) && (info->row > 0x29) )
        {
            leaving = 1;
            if ( lfFrame == 0 )
                lfFrame = frameCount;
        }
    }

    /* Deallocate zoomer buffers: */
    if ( (error = memFree(buf[0])) != OK )
        Error(errorMsg[error]);
    if ( (error = memFree(buf[1])) != OK )
        Error(errorMsg[error]);
}
