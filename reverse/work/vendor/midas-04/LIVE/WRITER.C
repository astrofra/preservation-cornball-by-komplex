/*      WRITER.C
 *
 * S2 The Party '94 64kb intro
 * -- 3D vector font writer
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


#define MAXLETTERS 256
#define MAXVERTS 8

#define SCRWIDTH 320
#define SCRHEIGHT 200

#define LETTERSIZE 100

#define DELAY1 10
#define TIME1 100
#define SPEED1 10
#define NUMFR1 300

#define TIME2 70

#define TIME3 150
#define DELAY3 7
#define T3 45

#define TIME4 100

#define SPEED5 10

/* Perspective projection multiplier: */
#define PROJMULT -2*160

/* Aspect ratio: (xx/100) */
#define ASPECTRATIO 83


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
* Function:     void DrawWater(ushort bufSeg, ushort pageNum, ushort height);
*
* Description:  Draws the water from buffer
*
*
\****************************************************************************/

void cdecl  DrawWater(ushort bufSeg, ushort pageNum, ushort height);

extern  char    waterPalette[];


/****************************************************************************\
*
* Function:     int MultDiv(int a, int b, int c)
*
* Description:  Calculates (a*b) / c using 32-bit temporary value
*
* Input:        int a                   multiplicant
*               int b                   multiplier
*               int c                   divisor
*
* Returns:      (a * b) / c
*
\****************************************************************************/

int MultDiv(int a, int b, int c);
#pragma aux MultDiv = \
        "imul   dx" \
        "idiv   bx" \
        parm [ax] [dx] [bx] \
        modify exact [ax dx] \
        value [ax];



/****************************************************************************\
*       struct Letter
*       -------------
* Description:  One writer letter
\****************************************************************************/

typedef struct
{
    uchar       *charData;              /* pointer to character font data */
    int         color;                  /* palette index */
    int         x, y, z;
    int         r, g, b;
    int         rotation;
} Letter;


extern void cdecl FillPoly(int numpnts, void *poly, unsigned pagestart,
    char color, int *lineTable);




static unsigned actSeg;                 /* active screen page segment */
static int      page;
static int      numLetters;
static Letter   *letters;
static int      *fillDest;
static int      rot1;
static ulong    frames1;





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
* Function:     void SetLetterPalette(Letter *letter)
*
* Description:  Sets the palette entry of a letter
*
* Input:        Letter *letter          Writer letter
*
\****************************************************************************/

void SetLetterPalette(Letter *letter)
{
    SetPaletteEntry(letter->color, letter->r, letter->g, letter->b);
}



/****************************************************************************\
*
* Function:     void DrawLetter(Letter *letter)
*
* Description:  Draws a 3D writer letter to the screen
*
* Input:        Letter *letter          Writer letter
*
\****************************************************************************/

void DrawLetter(Letter *letter)
{
    int         cx, cy;
    int         z = letter->z;
    uchar       *data = letter->charData;
    uchar       numPolys;
    uchar       numVerts;
    uchar       vertex;
    int         poly[2*MAXVERTS];
    int         x, y, ry, rz;
    int         polyn, vert;
    int         rotation = letter->rotation;
    int         color = letter->color;

    cx = letter->x;
    cy = letter->y;

    numPolys = *(data++);

    for ( polyn = 0; polyn < numPolys; polyn++ )
    {
        numVerts = *(data++);

        for ( vert = 0; vert < numVerts; vert++ )
        {
            vertex = *(data++);
            x = LETTERSIZE * (((int) (vertex >> 4)) - 3);
            y = LETTERSIZE * (((int) (vertex & 15)) - 5);
            ry = iCosMult(rotation, y);
            rz = iSinMult(rotation, y) + z;

            poly[2*vert] = (SCRWIDTH/2) + MultDiv(PROJMULT, x+cx, rz);
            poly[2*vert+1] = (SCRHEIGHT/2) + MultDiv(PROJMULT, ry+cy, rz);
        }

        FillPoly(numVerts, poly, 16000 * page, color, fillDest);
    }
}



/****************************************************************************\
*
* Function:     void DrawLetter2D(int x, int y, char ch, int size, int color)
*
* Description:  Draws a 2D vector character to the screen
*
* Input:        int x                   character upper left corner x coord.
*               int y                   character upper left corner y coord.
*               char ch                 character
*               int size                character size
*               int color               character color
*
\****************************************************************************/

void DrawLetter2D(int x, int y, char ch, int size, int color)
{
    uchar       *data;
    uchar       numPolys;
    uchar       numVerts;
    uchar       vertex;
    int         poly[2*MAXVERTS];
    int         polyn, vert;

    data = font[ch-32];

    numPolys = *(data++);

    for ( polyn = 0; polyn < numPolys; polyn++ )
    {
        numVerts = *(data++);

        for ( vert = 0; vert < numVerts; vert++ )
        {
            vertex = *(data++);
            poly[2*vert] = x + size * ((int) vertex >> 4);
            poly[2*vert+1] = y + size * ((int) vertex & 15);
        }

        FillPoly(numVerts, poly, 16000 * page, color, fillDest);
    }
}



/****************************************************************************\
*
* Function:     void AddLetter(...)
*
* Description:  Adds a letter to the screen letter list
*
\****************************************************************************/

void AddLetter(int x, int y, int z, int r, int g, int b, int color, char ch)
{
    Letter      *letter = &letters[numLetters];

    letter->x = x; letter->y = y; letter->z = z; letter->r = r; letter->g = g;
    letter->b = b; letter->color = color;
    letter->charData = font[ch - 32];
    letter->rotation = 0;
    numLetters++;
}



void PutString(int x, int y, int z, int r, int g, int b, int fcolor, int cadd,
    char *str)
{
    int         color = fcolor;
    int         i;
    int         slen = mStrLength(str);

    for ( i = 0; i < slen; i++ )
    {
        AddLetter(x, y, z, r, g, b, color, str[i]);
        x += 8 * LETTERSIZE;
        color += cadd;
    }
}



void DrawString2D(int x, int y, int size, char *str, int color)
{
    int         i;
    int         slen = mStrLength(str);

    for ( i = 0; i < slen; i++ )
    {
        DrawLetter2D(x, y, str[i], size, color);
        x += 8 * size;
    }
}



void DrawLetters(void)
{
    int         i;

    for ( i = 0; i < numLetters; i++ )
        DrawLetter(&letters[i]);
}



void Writer1(void)
{
    int         i;
    ulong       f;

    if ( frames1 < ((numLetters+1) * TIME1) )
    {
        f = frames1;

        for ( i = 0; i < numLetters; i++ )
        {
            if ( (f < TIME1) && (f > 0) )
            {
                if ( i > 5 )
                    letters[i].y += 2*SPEED1;
                else
                    letters[i].y += SPEED1;

                letters[i].z -= 5 * SPEED1;
            }

            f -= DELAY1;
        }
    }

    for ( i = 0; i < numLetters; i++ )
        letters[i].rotation = rot1;

//    rot1 += 2;

    frames1++;
}



void Writer2(void)
{
    int         i;
    long        f;
    Letter      *letter;

    f = frames1;

    for ( i = 0; i < 13; i++ )
    {
        letter = &letters[i];
        if ( f > 0 )
        {
            if ( f > T3 )
            {
                letter->r = 1;
                letter->g = 0;
                letter->b = 0x0D;
                letter->rotation = 90;
            }
            else
            {
                letter->rotation = 2*f;
                letter->r = 1 + MultDiv(53, T3-f, T3);
                letter->g = MultDiv(56, T3-f, T3);
                letter->b = 0x0D + MultDiv(50, T3-f, T3);
            }
        }
        else
        {
            letter->r = 54;
            letter->g = 56;
            letter->b = 63;
        }

        f -= DELAY3;
    }

    letters[numLetters-1].r = 1 + MultDiv(53, TIME3-frames1, TIME3);
    letters[numLetters-1].g = MultDiv(56, TIME3-frames1, TIME3);
    letters[numLetters-1].b = 0x0D + MultDiv(50, TIME3-frames1, TIME3);

    frames1++;
}



void ClearScreen(unsigned segment);
#pragma aux ClearScreen = \
        "mov    es,ax" \
        "mov    dx,03C4h" \
        "mov    ax,0F02h" \
        "out    dx,ax" \
        "xor    di,di" \
        "mov    cx,4000" \
        "xor    eax,eax" \
        "cld" \
        "rep    stosd" \
        parm [ax] \
        modify exact [ax cx dx es di];




/****************************************************************************\
*
* Function:     void Writer(void)
*
* Description:  The Writer part (beginning)
*
\****************************************************************************/

void Writer(void)
{
    Letter      *letter;
    int         error, i, f, oo;
    ulong       ofCount, skipFrames;
    signed char *buf;
    ushort      bufSeg;
    ushort      a, a2;
    long        c, b;
    char        *sini;
    char        *sini2;
    ushort      sinSeg;
    uchar       *pal;
    ushort      waterHeight = 0;

    /* Set up 320x200x256 Non Chain Four display mode: */
    vgaSetNC4();

    ClearVGA();

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
    a = 47;
    a2 = 17;


    page = 0; actSeg = 0xA000; scrStart = 0;
    ClearScreen(actSeg);
    SetPaletteEntry(0, 0x01, 0x00, 0x0D);
    WaitFrame();

    /* Allocate memory for letter data: */
    if ( (error = memAlloc(MAXLETTERS * sizeof(Letter), (void**) &letters))
        != OK )
        Error(errorMsg[error]);
    numLetters = 0;

    /* Allocate memory for polygon filler destination: */
    if ( (error = memAlloc((2*SCRHEIGHT+2) * sizeof(int), (void**) &fillDest))
        != OK )
        Error(errorMsg[error]);

    numLetters = 0;
    PutString(-5 * 4 * LETTERSIZE, -26 * LETTERSIZE / 2 - 1000, -1000, 54, 56,
        63, 1, 1, "SAHARA");
    PutString(-3 * 8 * LETTERSIZE, -2000, -1000, 54, 56, 63, 7, 1,
        "SURFERS");

    ofCount = frameCount;

    rot1 = 0; frames1 = 0;

    while ( frames1 < 300 )
    {
        scrStart = 16000 * page;
        WaitFrame();

        for ( i = 0; i < numLetters; i++ )
            SetLetterPalette(&letters[i]);

        page ^= 1;
        actSeg = 0xA000 + (16000/16) * page;

        skipFrames = frameCount - ofCount;
        ofCount = frameCount;

        while ( skipFrames )
        {
            Writer1();
            skipFrames--;
        }

        ClearScreen(actSeg);
        DrawLetters();
    }

    PutString(-3 * 8* LETTERSIZE, 40 * LETTERSIZE / 2, -9000, 54, 56, 63, 14,
        0, "PRESENT");

    do
        UpdInfo();
    while (info->pos < 1);

    SetPaletteEntry(14, 1, 0, 0x0D);
    ClearScreen(actSeg);
    DrawLetters();

    frames1 = 0;
    while ( frames1 < TIME2 )
    {
        scrStart = 16000 * page;
        WaitFrame();
        if ( frames1 < 65 )
        {
            SetPaletteEntry(14, 1 + fadeTable[(frames1 << 8) + 53],
                fadeTable[(frames1 << 8) + 56], 0x0D +
                fadeTable[(frames1 << 8) + 50]);
        }
        frames1++;
    }

    do
        UpdInfo();
    while (info->row < 0x20);


    frames1 = 0;

    ofCount = frameCount;

    while ( frames1 < TIME3 )
    {
        scrStart = 16000 * page;
        WaitFrame();

        for ( i = 0; i < numLetters; i++ )
            SetLetterPalette(&letters[i]);

        page ^= 1;
        actSeg = 0xA000 + (16000/16) * page;

        skipFrames = frameCount - ofCount;
        ofCount = frameCount;

        while ( skipFrames )
        {
            Writer2();
            skipFrames--;
        }

        ClearScreen(actSeg);
        DrawLetters();
    }

    numLetters = 0;

    page = 0;
    scrStart = 0;
    actSeg = 0xA000;
    WaitFrame();
    ClearVGA();
    SetPaletteEntry(1, 1, 0, 0x0D);
    DrawString2D(136, 40, 2, "FOR", 1);
    DrawString2D(48, 68, 2, "THE PARTY 1994", 1);
    DrawString2D(88, 96, 2, "64K INTRO", 1);
    DrawString2D(72, 124, 2, "COMPETITION", 1);

    do
        UpdInfo();
    while ( info->pos < 2 );

    frames1 = 0;
    while ( frames1 < 65 )
    {
        WaitFrame();
        SetPaletteEntry(1, 1 + fadeTable[(frames1 << 8) + 53],
            fadeTable[(frames1 << 8) + 56], 0x0D +
            fadeTable[(frames1 << 8) + 50]);
        frames1++;
    }

    do
        UpdInfo();
    while ( info->pos < 3 );

    frames1 = 0;
    while ( frames1 < 65 )
    {
        scrStart = 16000 * page;
        WaitFrame();
        if ( frames1 < 65 )
        {
            SetPaletteEntry(1, 1 + fadeTable[((64-frames1) << 8) + 53],
                fadeTable[((64-frames1) << 8) + 56], 0x0D +
                fadeTable[((64-frames1) << 8) + 50]);
        }
        frames1++;
    }

    numLetters = 0;

    PutString(-3 * 4 * LETTERSIZE, 0, -4000, 54, 56, 63,
        255, 0, "LIVE");

    scrStart = 16000*page;
    WaitFrame();
    SetPaletteEntry(255, 1, 0, 0x0D);

    page ^= 1;
    actSeg = 0xA000 + (16000/16) * page;
    ClearScreen(actSeg);
    DrawLetters();
    scrStart = 16000*page;
    WaitFrame();
    page ^= 1;
    actSeg = 0xA000 + (16000/16) * page;
    ClearScreen(actSeg);
    DrawLetters();
    scrStart = 16000*page;
    WaitFrame();

    do
        UpdInfo();
    while ( info->row < 0x20 );

    frames1 = 0;
    while ( frames1 < TIME4 )
    {
        scrStart = 16000 * page;
        WaitFrame();
        if ( frames1 < 65 )
        {
            SetPaletteEntry(255, 1 + fadeTable[(frames1 << 8) + 53],
                fadeTable[(frames1 << 8) + 56], 0x0D +
                fadeTable[(frames1 << 8) + 50]);
        }
        frames1++;
    }

    do
        UpdInfo();
    while ( info->pos < 4 );

    frames1 = 0;
    ofCount = frameCount; oo = 0;

    while ( (info->pos < 7) || ((info->pos == 7) && (info->row < 0x3E)) )
    {
        UpdInfo();

        scrStart = 16000 * page;
        WaitFrame();
        page ^= 1;
        actSeg = 0xA000 + (16000/16) * page;

        if ( frames1 < 129 )
        {
            pal = &waterPalette[3];
            f = frames1 >> 1;
            for ( i = 1; i < 201; i++ )
            {
                SetPaletteEntry(i, 1 + (f * (((int) (*pal)) - 1) / 64),
                    (f * ((int) pal[1])) / 64,
                    13 + (f * (((int) pal[2]) - 13)) / 64);
                pal += 3;
            }
        }
        else
        {
            if ( (info->pos == 7) && (info->row >= 0x30) )
            {
                if ( oo == 0 )
                    oo = frameCount;
                f = frameCount - oo;
                if ( f > 64 )
                    f = 64;
                FadePalette(&waterPalette[0], 0, 201, 64 - f, fadeTable);
            }
            else
            {
                SetPalette(&waterPalette[0], 0, 201);
            }
        }

        skipFrames = frameCount - ofCount;
        ofCount = frameCount;
        while ( skipFrames )
        {
            if ( info->pos >= 6 )
            {
                for ( i = 0; i < numLetters; i++ )
                {
                    letter = &letters[i];
                    if ( letter->z < -500 )
                    {
                        letter->z += 5*SPEED5;
                        letter->y -= 2*SPEED5;
                    }
                }
            }
            a += 23;
            if ( a > 256*16 )
                a -= 256*16;
            a2 += 1;
            if ( a2 > 256 )
                a2 -= 256;

            if ( (info->pos == 7) && (info->row >= 0x20) )
                waterHeight++;

            frames1++;
            skipFrames--;
        }



        ClearScreen(actSeg);

        WaveWater(bufSeg, sinSeg, a, a2, 3, 7, 1);
        DrawWater(bufSeg, page, waterHeight);
        DrawLetters();
    }

    /* Deallocate letter data: */
    if ( (error = memFree(letters)) != OK )
        Error(errorMsg[error]);

    /* Deallocate polygon filler destination: */
    if ( (error = memFree(fillDest)) != OK )
        Error(errorMsg[error]);
}



/****************************************************************************\
*
* Function:     void EndText(void)
*
* Description:  The end text screen
*
\****************************************************************************/

void EndText(void)
{
    int         error;

    /* Set up 320x200x256 Non Chain Four display mode: */
    vgaSetNC4();

    page = 0; actSeg = 0xA000; scrStart = 0;
    ClearVGA();
    SetPaletteEntry(0, 0, 0, 0);
    SetPaletteEntry(1, 0, 0, 0);
    WaitFrame();

    /* Allocate memory for polygon filler destination: */
    if ( (error = memAlloc((2*SCRHEIGHT+2) * sizeof(int), (void**) &fillDest))
        != OK )
        Error(errorMsg[error]);

    DrawString2D(128, 40, 2, "LIVE", 1);
    DrawString2D(48, 96, 2, "COPYRIGHT 1994", 1);
    DrawString2D(48, 124, 2, "SAHARA SURFERS", 1);

    do
        UpdInfo();
    while ( info->pos < 0x1C );

    frames1 = 0;
    while ( frames1 < 65 )
    {
        WaitFrame();
        SetPaletteEntry(1, fadeTable[(frames1 << 8) + 51],
            fadeTable[(frames1 << 8) + 50],
            fadeTable[(frames1 << 8) + 53]);
        frames1++;
    }

    do
        UpdInfo();
    while ( (info->pos < 0x1D) || (info->row < 0x30) );

    frames1 = 0;
    while ( frames1 < 65 )
    {
        scrStart = 16000 * page;
        WaitFrame();
        if ( frames1 < 65 )
        {
            SetPaletteEntry(1, fadeTable[((64-frames1) << 8) + 51],
                fadeTable[((64-frames1) << 8) + 50],
                fadeTable[((64-frames1) << 8) + 53]);
        }
        frames1++;
    }

    /* Deallocate letter data: */
    if ( (error = memFree(letters)) != OK )
        Error(errorMsg[error]);
}
