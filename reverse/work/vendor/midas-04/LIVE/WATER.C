/*      WATER.C
 *
 * S2 The Party '94 64kb intro
 * -- "Water"
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
* Function:     void WaveWater(ushort bufSeg, char *sini, ushort angle,
*                              ushort inc1, ushort inc2);
*
* Description:  "Waves" the water in buffer
*
*
\****************************************************************************/

void cdecl WaveWater(ushort bufSeg, ushort sinSeg, ushort angle, ushort angle2,
                     ushort inc1, ushort inc2, ushort inc3);



/****************************************************************************\
*
* Function:     void DrawWater(ushort bufSeg, ushort pageNum);
*
* Description:  Draws the water from buffer
*
*
\****************************************************************************/

void cdecl  DrawWater(ushort bufSeg, ushort pageNum);

extern  char    waterPalette[];



/****************************************************************************\
*
* Function:     void Water(void)
*
* Description:  The water part
*
\****************************************************************************/

void Water(void)
{
    signed char *buf;
    ushort      bufSeg;
    ushort      r, a, a2;
    long        c, b;
    int         error;
    char        *sini;
    char        *sini2;
    ushort      sinSeg;


    /* Allocate memory for water buffer: */
    if ( (error = memAlloc(64016, (void**) &buf)) != OK )
        Error(errorMsg[error]);

    /* Set buffer segment to point to buffer: */
    bufSeg = FP_SEG(buf) + (FP_OFF(buf) + 15) / 16;

    /* Clear buffers: */
    memset(buf, 0, 64016);

    /* Allocate memory for sine table: */
    if ( (error = memAlloc(512+16, (void**) &sini)) != OK )
        Error(errorMsg[error]);

    /* Set sine segment to point to sine table: */
    sinSeg = FP_SEG(sini) + (FP_OFF(sini) + 15) / 16;

    sini2 = MK_FP(sinSeg,0);

    for (c = 0; c < 512; c++)
    {
        b = 360*c/256;
        sini2[c] = (char)iSinMult(b, 10);
    }

    /* Reset screen buffer flip flop */
    r = 0;
    a = 47;
    a2 = 17;


    vgaSetNC4();

    /* Set the correct RGB palette: */
    SetPalette(&waterPalette[0], 0, 256);

    while ( !kbhit() )
    {
        WaitFrame();

        SetBorder(0);

        WaveWater(bufSeg, sinSeg, a, a2, 3, 7, 1);

        DrawWater(bufSeg, r);

        SetScrStart(r*80*200);

        /* Flip flop */

        r ^= 1;

        a += 69;
        if ( a > 256*16 ) a -= 256*16;

        a2 += 3;
        if ( a2 > 256 ) a2 -= 256;

        SetBorder(255);
    }

    getch();

    /* Deallocate sine table: */
    if ( (error = memFree(sini)) != OK )
        Error(errorMsg[error]);

    /* Deallocate screen buffers: */
    if ( (error = memFree(buf)) != OK )
        Error(errorMsg[error]);
}
