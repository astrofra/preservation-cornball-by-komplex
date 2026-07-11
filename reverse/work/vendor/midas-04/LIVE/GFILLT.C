/*      GFILLT.C
 *
 * S2 The Party '94 64kb intro
 * -- Polygon tunnel gouraud shaded triangle filler
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

/* Note! This is SLOW! The tunnel part could be speeded up at least three
   times by switching it to normal 320x200x256 graphics mode and
   rewriting this filler.
*/


#include <dos.h>
#include <midas.h>
#include "intro.h"


#define SCRWIDTH 320
#define SCRHEIGHT 200


extern unsigned actSeg;                 /* active screen page segment */


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





void WritePlane(int plane);
#pragma aux WritePlane = \
        "mov    dx,03C4h" \
        "mov    ah,1" \
        "shl    ah,cl" \
        "mov    al,2" \
        "out    dx,ax" \
        parm [cx] \
        modify exact [ax dx];

void FillColumn(void far *start, int col1, int dcol, int height);
#pragma aux FillColumn = \
"lp:    mov     es:[bx],ah" \
        "add    ax,dx" \
        "add    bx,80" \
        "dec    cx" \
        "jnz    lp" \
        parm [es bx] [ax] [dx] [cx] \
        modify exact [ax bx cx];


/****************************************************************************\
*
* Function:     void DrawGouraud(int x, int y1, int color1, int y2,
*                   int color2)
*
* Description:  Draws a Gouraud-shaded filled polygon column
*
* Input:        int         x           x coordinate
*               int         y1          start y coordinate
*               int         color1      start color, 8.8 fixed point
*               int         y2          end y coordinate
*               int         color2      end color, 8.8 fixed point
*
\****************************************************************************/

void DrawGouraud(int x, int y1, int color1, int y2, int color2)
{
    int         dcol;

    if ( (y1 <= (SCRHEIGHT-1)) && (y2 >= 0) && (y2 >= y1) )
    {
        if ( y1 == y2 )
        {
            WritePlane(x & 3);
            *((uchar*) MK_FP(actSeg, 80*y1 + (x >> 2))) = color1 >> 8;
        }
        else
        {
            dcol = (color2 - color1) / (y2-y1);

            if ( y1 < 0 )
            {
                color1 += (-y1) * dcol;
                y1 = 0;
            }

            if ( y2 >= SCRHEIGHT)
            {
                y2 = SCRHEIGHT-1;
            }

            WritePlane(x & 3);
            FillColumn(MK_FP(actSeg, 80*y1 + (x >> 2)), color1, dcol,
                (y2-y1)+1);
        }
    }
}


int NextVert(int n);
#pragma aux NextVert = \
        "inc    ax" \
        "cmp    ax,3" \
        "jb     ok" \
        "xor    ax,ax" \
"ok:"\
        parm [ax] \
        modify exact [ax] \
        value [ax];

int PrevVert(int n);
#pragma aux PrevVert = \
        "dec    ax" \
        "jns    ok" \
        "mov    ax,2" \
"ok:" \
        parm [ax] \
        modify exact [ax] \
        value [ax];


long DeltaY(long y, int desty, int width);
#pragma aux DeltaY = \
        "shl    edx,16" \
        "shl    ebx,16" \
        "mov    dx,ax" \
        "movsx  ecx,cx" \
        "sub    ebx,edx" \
        "mov    eax,ebx" \
        "cdq" \
        "idiv   ecx" \
        "mov    edx,eax" \
        "shr    edx,16" \
        parm [dx ax] [bx] [cx] \
        modify [bx cx];





/****************************************************************************\
*
* Function:     void FillTriangle(PolyVertex *vertices)
*
* Description:  Fills a Gouraud-shaded triangle
*
* Input:        PolyVertex *vertices    triangle vertices
*
\****************************************************************************/

void FillTriangle(PolyVertex *vertices)
{
    long        y1, y2, dy1, dy2;
    int         x, y, w1, w2, maxx;
    int         p1, p2, np1, np2;
    int         color1, color2;
    int         dcol1, dcol2;

    if ( ((vertices[0].x < 0) && (vertices[1].x < 0) && (vertices[2].x < 0))
        || ((vertices[0].x >= SCRWIDTH) && (vertices[1].x >= SCRWIDTH) &&
        (vertices[2].x >= SCRWIDTH)) )
    {
        return;
    }

    if ( (vertices[0].x == vertices[1].x) &&
        (vertices[1].x == vertices[2].x) )
    {
        /* find the uppermost vertex (p1): */
        y = vertices[0].y; p1 = 0;
        if ( vertices[1].y < y )
            p1 = 1;
        if ( vertices[2].y < y )
            p1 = 2;

        /* find the lowest vertex (p2): */
        y = vertices[0].y; p2 = 0;
        if ( vertices[1].y > y )
            p2 = 1;
        if ( vertices[2].y > y )
            p2 = 2;

        DrawGouraud(vertices[0].x, vertices[p1].y, vertices[p1].color,
            vertices[p2].y, vertices[p2].color);
    }
    else
    {
        /* All vertices are NOT in the same column */

        /* Find the leftmost vertex: (p1) */
        x = vertices[0].x; p1 = 0;
        if ( vertices[1].x < x )
        {
            p1 = 1;
            x = vertices[1].x;
        }
        if ( vertices[2].x < x )
        {
            p1 = 2;
            x = vertices[2].x;
        }

        /* Find maximum x value: */
        maxx = vertices[0].x;
        if ( vertices[1].x > maxx )
            maxx = vertices[1].x;
        if ( vertices[2].x > maxx )
            maxx = vertices[2].x;

        /* Check if there are two vertices with the same x-coordinate: */
        if ( vertices[NextVert(p1)].x == x )
        {
            p2 = p1;
            p1 = NextVert(p1);
        }
        else
        {
            if ( vertices[PrevVert(p1)].x == x )
                p2 = PrevVert(p1);
            else
                p2 = p1;
        }

        np1 = NextVert(p1);
        np2 = PrevVert(p2);

        while ( vertices[np1].x == x )
            np1 = NextVert(np1);
        while ( vertices[np2].x == x )
            np1 = PrevVert(np2);

        w1 = vertices[np1].x - x;
        w2 = vertices[np2].x - x;

        if ( (w1 == 0) || (w2 == 0) )
            return;

        y1 = ((long) vertices[p1].y) << 16;
        y2 = ((long) vertices[p2].y) << 16;
//        dy1 = ((((long) vertices[np1].y) << 16) - y1) / w1;
        dy1 = DeltaY(y1, vertices[np1].y, w1);
//        dy2 = ((((long) vertices[np2].y) << 16) - y2) / w2;
        dy2 = DeltaY(y2, vertices[np2].y, w2);

        color1 = vertices[p1].color;
        color2 = vertices[p2].color;
        dcol1 = (vertices[np1].color - color1) / w1;
        dcol2 = (vertices[np2].color - color2) / w2;

        while ( 1 )
        {
            if ( (x >= maxx) || (x >= SCRWIDTH) )
                break;

            if ( x >= 0 )
            {
                DrawGouraud(x, (y1 >> 16), color1, (y2 >> 16), color2);
            }

            x++;
            y1 += dy1;
            y2 += dy2;
            color1 += dcol1;
            color2 += dcol2;
            w1--;
            w2--;

            if ( w1 <= 0 )
            {
                p1 = np1;
                np1 = NextVert(p1);
                w1 = vertices[np1].x - vertices[p1].x;
                y1 = ((long) vertices[p1].y) << 16;
                color1 = vertices[p1].color;
                if ( w1 != 0 )
                {
                    // dy1 = ((((long) vertices[np1].y) << 16) - y1) / w1;
                    dy1 = DeltaY(y1, vertices[np1].y, w1);
                    dcol1 = (vertices[np1].color - color1) / w1;
                }
                else
                {
                    dy1 = 32767;
                    dcol1 = 32767;
                }
            }

            if ( w2 <= 0 )
            {
                p2 = np2;
                np2 = PrevVert(p2);
                w2 = vertices[np2].x - vertices[p2].x;
                y2 = ((long) vertices[p2].y) << 16;
                color2 = vertices[p2].color;
                if ( w2 != 0 )
                {
                    // dy2 = ((((long) vertices[np2].y) << 16) - y2) / w2;
                    dy2 = DeltaY(y2, vertices[np2].y, w2);
                    dcol2 = (vertices[np2].color - color2) / w2;
                }
                else
                {
                    dy2 = 32767;
                    dcol2 = 32767;
                }
            }
        }
    }
}
