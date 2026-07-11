/*      PTUNNEL.C
 *
 * S2 The Party '94 64kb intro
 * -- Polygon tunnel
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

/* SLOW! Besides, there appears to be a bug somewhere, which causes the
   filler to fill a polygon with something like 32767x32767 dimensions
   completely outside the screen. At least the occasional pauses suggest
   something like that.
*/

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <dos.h>
#include <midas.h>
#include "intro.h"


#define VZSPEED -8
#define VXSPEED 2
#define VYSPEED 3


#define SCRWIDTH 320
#define SCRHEIGHT 200

//#define ELEMSIZE 256
#define ELEMSIZE 320
#define ELEMANG 36
#define ELEMANG2 (90-ELEMANG)
#define ELEMDIST 256
#define VIEWDIST 16
#define MAXDETAIL 1

/* Perspective projection multiplier: */
#define PROJMULT -2*160

/* Aspect ratio: (xx/100) */
#define ASPECTRATIO 83

/* NOTE! ALL 2D ROTATION IS COUNTERCLOCKWISE! */




/****************************************************************************\
*       struct Element
*       --------------
* Description:  One tunnel element
\****************************************************************************/

typedef struct
{
    int         x, y, z;                /* world coordinates for element
                                           center */
    int         visible;                /* 1 if the element is visible */
    int         rotation;               /* rotation angle (around z axis) */
    int         color;                  /* element base color */
    int         scrX[4];                /* corner screen X-coordinates */
    int         scrY[4];                /* corner screen Y-coordinates */
} Element;



/****************************************************************************\
*       struct PolyVertex
*       -----------------
* Description:  One Gouraud-shaded polygon vertex
\****************************************************************************/

typedef struct
{
    int         x, y;                   /* vertex coordinates */
    int         color;                  /* vertex color, in 8.8 fixed point
                                           format */
} PolyVertex;



void FillTriangle(PolyVertex *vertices);





static Element  *tunnel;                /* all tunnel elements */
static int      numElements;            /* number of elements in tunnel */
static int      viewX, viewY, viewZ;    /* viewer coordinates */
static int      vZrot;                  /* viewer rotation angle about z
                                           axis */
static int      elemZ;                  /* z coordinate for next tunnel
                                           element */
static int      elemColor;              /* color for next tunnel element */
static int      elemRot;                /* rotation for next tunnel element */
unsigned        actSeg;                 /* active screen page segment */
static int      movePhase;              /* movement phase */

extern uchar    ptPalette[];            /* palette for the part */
static int      aang1 = 0;
static int      px, py;


#define NUMPHASES 1






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





void Add1(int num)
{
    Element     *element;

    element = &tunnel[num];
    element->x = iSinMult(aang1 / 2, ELEMSIZE);
    element->y = iSinMult(aang1, ELEMSIZE);
    element->z = elemZ;
    element->rotation = elemRot;
    element->color = 128 * elemColor;

    elemZ -= ELEMDIST;
    elemColor ^= 1;

    UpdInfo();

    if ( (info->pos >= 0x09) )
    {
        aang1 += 16;
        if ( aang1 > 360 )
            aang1 -= 360;
    }

    if ( (info->pos >= 0x0B) )
    {
        elemRot += 5;
        if ( elemRot > 360 )
            elemRot -= 360;
    }
}



void Move1(void)
{
    viewZ += VZSPEED;
}




/****************************************************************************\
*
* Function:     void MoveTunnel(void)
*
* Description:  Moves the tunnel
*
\****************************************************************************/

void MoveTunnel(void)
{
    int         i, vdx, vdy;

    Move1();

    if ( viewZ <= (tunnel[0].z + 8) )
    {
        for ( i = 0; i < (numElements - 1); i++ )
            memcpy(&tunnel[i], &tunnel[i+1], sizeof(Element));
        if ( (info->pos < 0x10) || ((info->pos <= 0x10) &&
            (info->row < 0x20)) )
        {
            Add1(VIEWDIST-1);
        }
        else
        {
            if ( numElements > 0 )
                numElements--;
        }

    }

/*
 *       viewX = px + MultDiv(tunnel[0].x - px, ELEMSIZE-i, ELEMSIZE);
        viewY = py + MultDiv(tunnel[0].y - py, ELEMSIZE-i, ELEMSIZE);

    }
    else
    {
        i = viewZ - tunnel[0].z;
        viewX = tunnel[0].x + MultDiv(tunnel[1].x - tunnel[0].x,
            ELEMSIZE - i, ELEMSIZE);
        viewY = tunnel[0].y + MultDiv(tunnel[1].y - tunnel[0].y,
            ELEMSIZE - i, ELEMSIZE);
 */


    if ( (viewZ - tunnel[0].z) >= ELEMSIZE )
    {
        i = (viewZ - tunnel[0].z) - ELEMSIZE;
        vdx = (px + MultDiv(tunnel[0].x - px, ELEMSIZE-i, ELEMSIZE)) - viewX;
        vdy = (py + MultDiv(tunnel[0].y - py, ELEMSIZE-i, ELEMSIZE)) - viewY;

    }
    else
    {
        i = viewZ - tunnel[0].z;
        vdx = ( tunnel[0].x + MultDiv(tunnel[1].x - tunnel[0].x,
            ELEMSIZE - i, ELEMSIZE) ) - viewX;
        vdy = ( tunnel[0].y + MultDiv(tunnel[1].y - tunnel[0].y,
            ELEMSIZE - i, ELEMSIZE) ) - viewY;
        px = tunnel[0].x;
        py = tunnel[0].y;
    }

    if ( vdx < -VXSPEED )
        vdx = -VXSPEED;
    if ( vdx > VXSPEED )
        vdx = VXSPEED;
    if ( vdy < -VYSPEED )
        vdy = -VYSPEED;
    if ( vdy > VYSPEED )
        vdy = VYSPEED;

    viewX += vdx;
    viewY += vdy;
}



/****************************************************************************\
*
* Function:     void ProjectTunnel(void)
*
* Description:  Projects tunnel element coordinates to the screen
*
\****************************************************************************/

void ProjectTunnel(void)
{
    int         i;
    int         vx, vy, vz;             /* view space coordinates for element
                                           center */
    int         sx, sy;                 /* screen coordinates for element
                                           center */
    int         srx, sry;               /* element radius in screen coords */
    int         tempX, tempY;
    int         ang;
    Element     *element;

    for ( i = 0; i < numElements; i++ )
    {
        element = &tunnel[i];

        /* Transform element center to view space: */
        tempX = element->x - viewX;
        tempY = element->y - viewY;
        vx = iCosMult(vZrot, tempX) - iSinMult(vZrot, tempY);
        vy = iSinMult(vZrot, tempX) + iCosMult(vZrot, tempY);
        vz = element->z - viewZ;

        /* Project element only if it's in front of the viewer: */
        if ( vz < 0 )
        {
            element->visible = 1;

            /* Project element center to the screen: */
            sx = 160 + MultDiv(PROJMULT, vx, vz);
            sy = 100 + MultDiv(PROJMULT * ASPECTRATIO / 100, vy, vz);

            /* Project element radius to the screen: */
            srx = MultDiv(PROJMULT, ELEMSIZE, vz);
            sry = MultDiv(PROJMULT * ASPECTRATIO / 100, ELEMSIZE, vz);

            /* Calculate the coordinates for all of the corners of the
               element, using the projected radius and center: */
            ang = element->rotation - vZrot;
            element->scrX[0] = sx - iSinMult(ang + ELEMANG2, srx);
            element->scrX[1] = sx + iCosMult(ang + ELEMANG, srx);
            element->scrX[2] = sx + iSinMult(ang + ELEMANG2, srx);
            element->scrX[3] = sx - iCosMult(ang + ELEMANG, srx);
            element->scrY[0] = sy - iCosMult(ang + ELEMANG2, sry);
            element->scrY[1] = sy - iSinMult(ang + ELEMANG, sry);
            element->scrY[2] = sy + iCosMult(ang + ELEMANG2, sry);
            element->scrY[3] = sy + iSinMult(ang + ELEMANG, sry);
        }
        else
            element->visible = 0;
    }
}




/****************************************************************************\
*
* Function:     void DrawTunnel(void)
*
* Description:  Draws the tunnel to the screen
*
\****************************************************************************/

void DrawTunnel(void)
{
    int         i;
    int         color1, color2;
    Element     *elem1, *elem2;
    PolyVertex  face[3];
    int         vnum;
    int         x0, x1, x2, x3;
    int         y0, y1, y2, y3;

    for ( i = (numElements-1); i > 0; i-- )
    {
        elem1 = &tunnel[i-1];
        elem2 = &tunnel[i];

        if ( (elem1->visible) && (elem2->visible) )
        {
            color1 = (viewZ - elem1->z) / (ELEMDIST / 4);
            if ( color1 > 63 )
                color1 = 63;
            if ( color1 < 0 )
                color1 = 0;
            color2 = (viewZ - elem2->z) / (ELEMDIST / 4);
            if ( color2 > 63 )
                color2 = 63;
            if ( color2 < 0 )
                color2 = 0;
            color1 = (elem1->color << 8) + ((63-color1) << 8);
            color2 = (elem1->color << 8) + ((63-color2) << 8);

            for ( vnum = 0; vnum < 4; vnum++ )
            {

                x0 = elem1->scrX[vnum];
                y0 = elem1->scrY[vnum];
                x1 = elem1->scrX[(vnum+1) & 3];
                y1 = elem1->scrY[(vnum+1) & 3];
                x2 = elem2->scrX[vnum];
                y2 = elem2->scrY[vnum];
                x3 = elem2->scrX[(vnum+1) & 3];
                y3 = elem2->scrY[(vnum+1) & 3];

                if ( (((long) (x1-x0)) * ((long) (y2-y0))) >
                    (((long) (y1-y0)) * ((long) (x2-x0))) )
                {
                    face[0].x = x0;
                    face[0].y = y0;
                    face[0].color = color1;
                    face[1].x = x1;
                    face[1].y = y1;
                    face[1].color = color1;
                    face[2].x = x2;
                    face[2].y = y2;
                    face[2].color = color2;
                    FillTriangle(&face[0]);

                    face[0].x = x1;
                    face[0].y = y1;
                    face[0].color = color1;
                    face[1].x = x3;
                    face[1].y = y3;
                    face[1].color = color2;
                    face[2].x = x2;
                    face[2].y = y2;
                    face[2].color = color2;
                    FillTriangle(&face[0]);
                }

                color1 ^= 0x4000;
                color2 ^= 0x4000;
            }
        }
    }
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
* Function:     void PolygonTunnel(void)
*
* Description:  The Polygon tunnel part
*
\****************************************************************************/

void PolygonTunnel(void)
{
    int         error, i;
    int         page = 0;
    int         stop = 0;
    ulong       skipFrames, oldFrameCount;

    /* allocate memory for tunnel data: */
    if ( (error = memAlloc(MAXDETAIL * 2*VIEWDIST * sizeof(Element),
        (void**) &tunnel)) != OK )
        Error(errorMsg[error]);

    /* set starting position for viewpoint: */
    viewX = 0; viewY = 0; viewZ = 32000;

    /* build initial tunnel: */
    elemZ = viewZ - VIEWDIST * ELEMDIST;
//    elemZ = viewZ - ELEMDIST;
    elemColor = 0;
    elemRot = 0;
    numElements = VIEWDIST;
    for ( i = 0; i < VIEWDIST; i++ )
        Add1(i);

    /* Set the correct palette: */
    SetPalette(&ptPalette[0], 0, 256);

    actSeg = 0xA000;
    ClearScreen(actSeg);
    oldFrameCount = frameCount;
    movePhase = 0;

    /* run the tunnel: */
    while ( !stop )
    {
        UpdInfo();

        if ( (info->pos > 0x11) || ((info->pos == 0x11) &&
            (info->row > 0x3C)) )
            stop = 1;

        scrStart = 16000 * page;
        WaitFrame();

        page ^= 1;
        actSeg = 0xA000 + (16000/16) * page;

        skipFrames = frameCount - oldFrameCount;
        oldFrameCount = frameCount;


        // skipFrames = 1;

        while ( skipFrames )
        {
            MoveTunnel();
            skipFrames--;
        }

        ProjectTunnel();
        ClearScreen(actSeg);
        DrawTunnel();
    }

    /* deallocate tunnel data: */
    if ( (error = memFree(tunnel)) != OK )
        Error(errorMsg[error]);
}
