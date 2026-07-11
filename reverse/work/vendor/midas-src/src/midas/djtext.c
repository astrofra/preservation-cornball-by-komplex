/*      djtext.c
 *
 * VGA text output routines for MIDAS DJGPP port
 *
 * $Id: djtext.c,v 1.2 1997/07/31 10:56:36 pekangas Exp $
 *
 * Copyright 1996,1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include <sys/farptr.h>
#include <go32.h>
#include "lang.h"
#include "mtypes.h"
#include "vgatext.h"
#include "dpmi.h"


static unsigned scrWidth = 160;         /* screen width in bytes */

static char hexTable[] = "0123456789ABCDEF";

static dpmiRealCallRegs regs;



static unsigned StartAddr(unsigned x, unsigned y)
{
    return 0xB8000 + scrWidth * (y - 1) + 2 * (x - 1);
}




/****************************************************************************\
*
* Function:     void vgaSetWidth(int width);
*
* Description:  Sets the screen width used by text output routines
*
* Input:        int width               screen width in characters
*
\****************************************************************************/

void CALLING vgaSetWidth(int width)
{
    scrWidth = 2 * ((unsigned) width);
}




/****************************************************************************\
*
* Function:     void vgaWriteText(int x, int y, char *txt);
*
* Description:  Writes text on the screen
*
* Input:        int x                   X coordinate of string (up-left
*                                       corner is (1,1))
*               int y                   Y coordinate
*               char *txt               pointer to null-terminated text
*                                       string, which may contain also the
*                                       following special characters:
*                                           \xFF - next char is attribute
*                                           \x7F - next char is RLE count for
*                                               the character following it
*
\****************************************************************************/

void CALLING vgaWriteText(int x, int y, char *txt)
{
    unsigned    addr = StartAddr(x, y);
    uchar       attr = 0x07;
    uchar       c;
    uchar       *cptr = (uchar*) txt;
    uchar       count;
    ushort      v;

    while ( *cptr != 0 )
    {
        c = *(cptr++);
        if ( c == 0 )
            break;

        if ( c == 0xFF )
        {
            attr = *(cptr++);
            continue;
        }

        if ( c == 0x7F )
        {
            count = *(cptr++);
            c = *(cptr++);
            v = (((ushort) attr) << 8) | c;
            while ( count )
            {
                _farpokew(_dos_ds, addr, v);
                addr += 2;
                count--;
            }
            continue;
        }

        _farpokew(_dos_ds, addr, (((ushort) attr) << 8) | c);
        addr += 2;
    }
}



/****************************************************************************\
*
* Function:     void vgaWriteStr(int x, int y, char *str, char attr,
*               int txtlen);
*
* Description:  Writes a string on the screen
*
* Input:        int x                   X coordinate of the string
*               int y                   Y coordinate
*               char *str               pointer to a ASCIIZ string
*               char attr               attribute for the string
*               int txtlen              number of characters to be printed on
*                                       screen - padded with spaces
*
\****************************************************************************/

void CALLING vgaWriteStr(int x, int y, char *str, char attr, int txtlen)
{
    unsigned    addr = StartAddr(x, y);
    ushort      v = ((ushort) attr) << 8;

    while ( (txtlen) && (*str != 0) )
    {
        v = (v & 0xFF00) | ((uchar) *(str++));
        _farpokew(_dos_ds, addr, v);
        addr += 2;
        txtlen--;
    }

    v = (v & 0xFF00) | ' ';
    while ( txtlen )
    {
        _farpokew(_dos_ds, addr, v);
        addr += 2;
        txtlen--;
    }
}




/****************************************************************************\
*
* Function:     void vgaWriteByte(int x, int y, uchar byte, char attr);
*
* Description:  Writes a hex byte on the screen
*
* Input:        int x                   X coordinate
*               int y                   Y coordinate
*               uchar byte              byte to be written
*               char attr               attribute for the byte
*
\****************************************************************************/

void CALLING vgaWriteByte(int x, int y, uchar byte, char attr)
{
    unsigned    addr = StartAddr(x, y);
    ushort      v = ((ushort) attr) << 8;

    _farpokew(_dos_ds, addr, v | hexTable[byte >> 4]);
    _farpokew(_dos_ds, addr + 2, v | hexTable[byte & 0x0F]);
}




/****************************************************************************\
*
* Function:     void vgaFillRect(int x1, int y1, int x2, int y2, char attr);
*
* Description:  Draws a filled rectangle on the screen
*
* Input:        int x1                  X-coordinate of upper left corner
*               int y1                  Y-coordinate of upper left corner
*               int x2                  X-coordinate of lower left corner
*               int y2                  Y-coordinate of lower left corner
*               char attr               rectangle attribute
*
\****************************************************************************/

void CALLING vgaFillRect(int x1, int y1, int x2, int y2, char attr)
{
    unsigned    addr = StartAddr(x1, y1);
    ushort      v = (((ushort) attr) << 8) | ' ';
    unsigned    numRows = y2 - y1 + 1;
    unsigned    numColumns = x2 - x1 + 1;
    unsigned    colCount;
    unsigned    rowSkip = scrWidth - (2 * numColumns);

    while ( numRows )
    {
        colCount = numColumns;

        while ( colCount )
        {
            _farpokew(_dos_ds, addr, v);
            addr += 2;
            colCount--;
        }
        addr += rowSkip;
        numRows--;
    }
}



/****************************************************************************\
*
* Function:     void vgaDrawChar(int x, int y, char ch, char attr);
*
* Description:  Draws a single character on the screen
*
* Input:        int x                   character X-coordinate
*               int y                   character Y-coordinate
*               char ch                 character
*               char attr               character attribute
*
\****************************************************************************/

void CALLING vgaDrawChar(int x, int y, char ch, char attr)
{
    _farpokew(_dos_ds, StartAddr(x, y), (((ushort) attr) << 8) | ((uchar) ch));

}




/****************************************************************************\
*
* Function:     void vgaSetMode(int mode)
*
* Description:  Sets a VGA BIOS display mode
*
* Input:        int mode                BIOS mode number
*
\****************************************************************************/

void CALLING vgaSetMode(int mode)
{
    regs.rSS = regs.rSP = 0;
    regs.rEAX = (mode & 0xFF);
    dpmiRealModeInt(0x10, &regs);
}




/****************************************************************************\
*
* Function:     void vgaMoveCursor(int x, int y);
*
* Description:  Moves the text mode cursor to a new location
*
* Input:        int x                   cursor X-coordinate
*               int y                   cursor Y-coordinate
*
\****************************************************************************/

void CALLING vgaMoveCursor(int x, int y)
{
    regs.rSS = regs.rSP = 0;
    regs.rEAX = 0x0200;
    regs.rEBX = 0;
    regs.rEDX = ((x - 1) & 0xFF) | (((unsigned) ((y - 1) & 0xFF)) << 8);
    dpmiRealModeInt(0x10, &regs);
}




/****************************************************************************\
*
* Function:     void vgaDrawChars(int x, int y, char ch, char attr, int num);
*
* Description:  Draws many charactersr on the screen
*
* Input:        int x                   character X-coordinate
*               int y                   character Y-coordinate
*               char ch                 character
*               char attr               character attribute
*               int num                 number characters to draw
*
\****************************************************************************/

void CALLING vgaDrawChars(int x, int y, char ch, char attr, int num)
{
    unsigned    addr = StartAddr(x, y);
    ushort      v = (((ushort) attr) << 8) | ((uchar) ch);

    while ( num )
    {
        _farpokew(_dos_ds, addr, v);
        addr += 2;
        num--;
    }
}



/*
 * $Log: djtext.c,v $
 * Revision 1.2  1997/07/31 10:56:36  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.1  1997/02/27 15:58:55  pekangas
 * Initial revision
 *
*/
