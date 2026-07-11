/*      INTRO.H
 *
 * S2 The Party '94 64kb intro
 * -- Include file for intro global functions and variables
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#ifndef __INTRO_H
#define __INTRO_H





/****************************************************************************\
*     Global variables
\****************************************************************************/


extern ushort   *sinTable;              /* pointer to sine table */
extern ushort   *cosTable;              /* pointer to cosine table */
extern uchar    music[];                /* the music as a .MM module */
extern uchar    *font[];                /* the vector font */
extern volatile ulong frameCount;       /* frame counter */
extern volatile unsigned scrStart;      /* screen start address */
extern mpInformation *info;             /* Module Player information */
extern uchar    *fadeTable;             /* Palette fading table */



/****************************************************************************\
*       Inline assembler pragmas:
\****************************************************************************/

void SetMode(int mode);
#pragma aux SetMode = \
        "int    10h" \
        parm [ax] \
        modify exact [ax];


void SetBorder(int color);
#pragma aux SetBorder = \
        "mov    dx,03DAh" \
        "in     al,dx" \
        "mov    dx,03C0h" \
        "mov    al,31h" \
        "out    dx,al" \
        "mov    al,bl" \
        "out    dx,al" \
        parm [bx] \
        modify exact [ax dx];


void ClearVGA(void);
#pragma aux ClearVGA = \
        "mov    dx,03C4h" \
        "mov    ax,0F02h" \
        "out    dx,ax" \
        "mov    ax,0A000h" \
        "mov    es,ax" \
        "xor    di,di" \
        "mov    cx,16384" \
        "xor    eax,eax" \
        "cld" \
        "rep    stosd" \
        modify exact [ax cx dx es di];


void SetPalette(uchar *pal, ushort firstCol, ushort numCols);
#pragma aux SetPalette = \
        "mov    dx,03C8h" \
        "out    dx,al" \
        "imul   cx,bx,3" \
        "inc    dx" \
"lp:    mov     al,es:[si]" \
        "inc    si" \
        "out    dx,al" \
        "loop   lp" \
        parm [es si] [ax] [bx] \
        modify exact [ax cx dx si];


void FadePalette(uchar *pal, ushort firstCol, ushort numCols,
    ushort brightness, uchar *ftable);
#pragma aux FadePalette = \
        "mov    dx,03C8h" \
        "out    dx,al" \
        "imul   cx,cx,3" \
        "inc    dx" \
        "mov    bh,bl" \
"lp:    mov     bl,es:[si]" \
        "inc    si" \
        "mov    al,[di+bx]" \
        "out    dx,al" \
        "loop   lp" \
        parm [es si] [ax] [cx] [bx] [ds di] \
        modify exact [ax bx cx dx si];


void SetScrStart(unsigned address);
#pragma aux SetScrStart = \
        "mov    dx,03D4h" \
        "mov    al,0Ch" \
        "mov    ah,bh" \
        "out    dx,ax" \
        "mov    al,0Dh" \
        "mov    ah,bl" \
        "out    dx,ax" \
        parm [bx] \
        modify exact [ax dx];


void WaitVR(void);
#pragma aux WaitVR = \
        "mov    dx,03DAh" \
"vr:    in      al,dx" \
        "test   al,8" \
        "jz     vr" \
        modify exact [ax dx];


void WaitNotVR(void);
#pragma aux WaitNotVR = \
        "mov    dx,03DAh" \
"vr:    in      al,dx" \
        "test   al,8" \
        "jnz    vr" \
        modify exact [ax dx];


void WaitDE(void);
#pragma aux WaitDE = \
        "mov    dx,03DAh" \
"de:    in      al,dx" \
        "test   al,1" \
        "jnz    de" \
        modify exact [ax dx];




/****************************************************************************\
*     Global functions
\****************************************************************************/



/****************************************************************************\
*
* Function:     int MakeTrigTables(void);
*
* Description:  Initializes and allocates the trigonometrical tables
*               (sinTable and cosTable)
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int cdecl MakeTrigTables(void);



/****************************************************************************\
*
* Function:     void WaitFrame(void)
*
* Description:  Waits for a Vertical Retrace
*
\****************************************************************************/

void WaitFrame(void);



/****************************************************************************\
*
* Function:     void UpdInfo(void)
*
* Description:  Updates module player information structure
*
\****************************************************************************/

void UpdInfo(void);



/****************************************************************************\
*
* Function:     int iSin(int angle);
*
* Description:  Gets a sine value for an angle from the sine table.
*
* Input:        int angle               angle, in degrees
*
* Returns:      Sine value for the angle, multiplied by 16384.
*
\****************************************************************************/

int cdecl iSin(int angle);




/****************************************************************************\
*
* Function:     int iCos(int angle);
*
* Description:  Gets a cosine value for and angle from the cosine table
*
* Input:        int angle               angle, in degrees
*
* Returns:      Cosine value for the angle, multiplied by 16384.
*
\****************************************************************************/

int cdecl iCos(int angle);




/****************************************************************************\
*
* Function:     int iSinMult(int angle, int mult);
*
* Description:  Calculates a sine value multiplied by a number.
*
* Input:        int angle               angle, in degrees
*               int mult                multiplier
*
* Returns:      mult * sin(angle)
*
\****************************************************************************/

int cdecl iSinMult(int angle, int mult);




/****************************************************************************\
*
* Function:     int iCosMult(int angle, int mult);
*
* Description:  Calculates a cosine value multiplied by a number.
*
* Input:        int angle               angle, in degrees
*               int mult                multiplier
*
* Returns:      mult * cos(angle)
*
\****************************************************************************/

int cdecl iCosMult(int angle, int mult);




/****************************************************************************\
*
* Function:     void Error(char *msg);
*
* Description:  Prints an error message and exits to DOS.
*
* Input:        char *msg               error message
*
\****************************************************************************/

void cdecl Error(char *msg);




/****************************************************************************\
*
* Function:     void vgaSetNC4(void);
*
* Description:  Sets up 320x200x256 Non Chain Four mode
*
\****************************************************************************/

void cdecl vgaSetNC4(void);




/****************************************************************************\
*
* Function:     void vgaSetC4(void);
*
* Description:  Sets up 320x200x256 Chain Four mode
*
\****************************************************************************/

void cdecl vgaSetC4(void);




/****************************************************************************\
*
* Function:     void ShadebobZoomer(void)
*
* Description:  The Shadedbob-zoomer part
*
\****************************************************************************/

void ShadebobZoomer(void);




/****************************************************************************\
*
* Function:     void PolygonTunnel(void)
*
* Description:  The Polygon tunnel part
*
\****************************************************************************/

void PolygonTunnel(void);




/****************************************************************************\
*
* Function:     void Writer(void)
*
* Description:  The Writer part (beginning)
*
\****************************************************************************/

void Writer(void);




/****************************************************************************\
*
* Function:     void Water(void)
*
* Description:  The water part
*
\****************************************************************************/

void Water(void);



/****************************************************************************\
*
* Function:     void LineTunnel(void)
*
* Description:  The line tunnel part
*
\****************************************************************************/

void LineTunnel(void);



/****************************************************************************\
*
* Function:     void EndText(void)
*
* Description:  The end text screen
*
\****************************************************************************/

void EndText(void);




/***************************************************************************\
*
* Function:     void DrawLine(int x1, int y1, int x2, int y2, int color);
*
* Description:  Draws a line to the screen
*
* Input:        int x1                  x-coordinate for first endpoint
*               int y1                  y-coordinate for first endpoint
*               int x2                  x-coordinate for second endpoint
*               int y2                  y-coordinate for second endpoint
*
\***************************************************************************/

void cdecl DrawLine(int x1, int y1, int x2, int y2);
void cdecl DrawLine2(int x1, int y1, int x2, int y2);





#endif
