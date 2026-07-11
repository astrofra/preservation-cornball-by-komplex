/*      LTUNNEL.C
 *
 * S2 The Party '94 64kb intro
 * -- Line tunnel
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


#define LINESPERCIRCLE      16
#define NUMCIRCLES          24
#define NUMDELAYS           8


int         linTab[LINESPERCIRCLE * 4 * NUMCIRCLES * NUMDELAYS];
char        palette[256*3*NUMDELAYS];
uchar       lineTab[LINESPERCIRCLE * 2 * NUMCIRCLES];

extern  char    WaterPalette[];
extern  char    LineColor;

void doPalette(char *palette, char *delayColors, char delayNum);
#pragma aux doPalette = \
        "mov    ch,1" \
        "mov    al,ch" \
        "shl    al,cl" \
        "mov    cl,al" \
    "l1: mov    al,cl" \
        "push   si" \
    "l2: test   ch,al" \
        "jnz    found" \
        "ror    al,1" \
        "add    si,3" \
        "jmp    l2" \
    "found:" \
        "movsw" \
        "movsb" \
        "pop    si" \
        "inc    ch" \
        "jnz    l1" \
        parm [es di] [ds si] [cl] \
        modify exact [cx al di];


void SetPaletteEntry(uchar color, uchar r, uchar g, uchar b);
#pragma aux SetPaletteEntry = \
        "mov    dx,03C8h" \
        "out    dx,al" \
        "inc    dx" \
        "mov    al,bl" \
        "out    dx,al" \
        "mov    al,cl" \
        "out    dx,al" \
        "mov    al,ch" \
        "out    dx,al" \
        parm [al] [bl] [cl] [ch] \
        modify exact [ax dx];



/****************************************************************************\
*
* Function:     void LineTunnel(void)
*
* Description:  The Line tunnel part
*
\****************************************************************************/

void LineTunnel(void)
{
    int         error;
    int         *sini;
    ushort      sinSeg;
    int         *sini2;
    int         *transTab;
    int         i, c, d, l, o;
    int         x1, y1, x2, y2;
    long        e, b;
    int         zTab[NUMCIRCLES+1];
    int         cSin[NUMCIRCLES+1];
    int         starting = 1;
    int         stopping = 0;
    int         stop = 0;
    int         lineCount = 0;
    int         oframeCnt;
    int         delay = 0;
    int         z, dx, dy, z2, dx2, dy2;
    int         direction = 4;
    long        ff;
    int         f;

    char        delays[8*3] = {
        53,53,63,
        26,26,36,
        17,17,22,
        12,12,17,
        10,10,15,
        7,7,13,
        5,5,11,
        1,0,13 };


    scrStart = 0;
    vgaSetC4();

    /* Allocate memory for sine table: */
    if ( (error = memAlloc(512*2+16, (void**) &sini)) != OK )
        Error(errorMsg[error]);

    if ( (error = memAlloc(1024 * sizeof(int), (void**) &transTab)) != OK )
        Error(errorMsg[error]);

    /* Clear line table */

    memset(&linTab[0], 0, sizeof(int) * LINESPERCIRCLE * 4 * NUMCIRCLES
            * NUMDELAYS);

    /* Set sine segment point to sine table: */
    sinSeg = FP_SEG(sini) + (FP_OFF(sini) + 15) / 16;

    sini2 = MK_FP(sinSeg,0);

    for (e = 0; e < 512; e++)
    {
        b = 360*e/256;
        sini2[e] = iSinMult(b, 339*32);            // 200
    }

    /* Do translation table */

    for (i = 0; i < 1024; i++)
        transTab[i] = i * 5 / 6;

    /* Initialize circles */

    c = 0;
    while ( c < NUMCIRCLES * 2 * LINESPERCIRCLE )
    {
        for (i = 0; i < LINESPERCIRCLE; i++)
        {
            lineTab[c] = i * 16;
            lineTab[c+1] = (i + 1) * 16;
            c += 2;
        }
    }

    for (c = 0; c < NUMCIRCLES; c++)
    {
        zTab[c] = 666;
    }

    zTab[NUMCIRCLES] = zTab[0];

    for (c = 0; c < NUMCIRCLES; c++)
    {
        cSin[c] = c * (360/NUMCIRCLES);
    }
    cSin[NUMCIRCLES] = cSin[0];

    for (d = 0; d < NUMDELAYS; d++)
       doPalette(&palette[d*256*3], &delays[0], d);

    d = 0;

    oframeCnt = frameCount;

    /* Main loop */

//    while ( info->pos < 0x16 )

    ff = frameCount;

    while ( !stop )
    {
        UpdInfo();
        if ( info->pos >= 0x15 )
            direction = -4;
        if ( (info->pos >= 0x15) && (info->row >=0x10) )
            stopping = 1;
        if ( info->pos >= 0x16 )
            stop = 1;

        WaitFrame();

        f = frameCount - ff;
        if ( f > 64 )
            f = 64;

        SetPaletteEntry(0, fadeTable[(f << 8) + 1], 0,
            fadeTable[(f << 8) + 0x0D]);

        /* Set the correct RGB palette: */
        SetPalette(&palette[d*256*3], 1, 255);

//        SetBorder(255);

        for (; oframeCnt < frameCount; oframeCnt++)
        {
            if (starting)
            {
                if ( delay == 0 )
                {
                    zTab[lineCount] = NUMCIRCLES * 32;
                    delay = 8;
                    if (++lineCount == NUMCIRCLES + 1) starting = 0;
                }
                else
                {
                    delay--;
                }
            }

            for ( c = 0; c < NUMCIRCLES + 1; c++)
            {
                if (zTab[c] != 666)
                {
                    zTab[c] -= 4;
                    if (zTab[c] < 32)
                    {
                        if(!stopping)
                        {
                            zTab[c] += NUMCIRCLES * 32;
                        }
                        else
                        {
                            zTab[c] = 666;
                        }
                    }
                }
                cSin[c] += direction;
            }
        }

        if ( ++d == NUMDELAYS ) d -= NUMDELAYS;

        LineColor = (1 << d) ^ 0xff;

        c = d * NUMCIRCLES * LINESPERCIRCLE * 4;
        for (i = 0; i < NUMCIRCLES*LINESPERCIRCLE; i++)
        {
            DrawLine(linTab[c], linTab[c+1], linTab[c+2], linTab[c+3]);
            c += 4;
        }

        c = d * NUMCIRCLES * LINESPERCIRCLE * 4;
        l = 0;
        o = 0;
        LineColor = 1 << d;

        while ( c < ( d + 1 ) * NUMCIRCLES * 4 * LINESPERCIRCLE )
        {
            z = zTab[o];
            if ( z != 666 )
            {
                z = z << 4;
                z2 = z + (32 * 16);

                dx = iSinMult(cSin[o], z);
                dy = iCosMult(cSin[o], z);
                dx2 = iSinMult(cSin[o+1], z2);
                dy2 = iCosMult(cSin[o+1], z2);
                z = z >> 4;
                z2 = z2 >> 4;

                for ( i = 0; i < LINESPERCIRCLE; i++)
                {
                    x1 = y1 = (int)lineTab[l];
                    x2 = y2 = (int)lineTab[l+1];

                    linTab[c] = (sini2[x1 + 64] + dx) / z + 160;
                    linTab[c+1] = transTab[(sini2[y1] + dy) / z + 512] - 320;
                    linTab[c+2] = (sini2[x2 + 64] + dx2) / z2 + 160;
                    linTab[c+3] = transTab[(sini2[y2] + dy2) / z2 + 512] - 320;
                    DrawLine2(linTab[c], linTab[c+1], linTab[c+2], linTab[c+3]);
                    c += 4;
                    l += 2;
                }
            }
            else
            {
                for ( i = 0; i < LINESPERCIRCLE; i++)
                {
                    linTab[c] = -1;
                    linTab[c+1] = -1;
                    linTab[c+2] = -1;
                    linTab[c+3] = -1;
                    c += 4;
                    l += 2;
                }
            }
            o++;
        }


//        SetBorder(0);
    }


    /* Deallocate sine table: */
    if ( (error = memFree(sini)) != OK )
        Error(errorMsg[error]);

    if ( (error = memFree(transTab)) != OK )
        Error(errorMsg[error]);
}



