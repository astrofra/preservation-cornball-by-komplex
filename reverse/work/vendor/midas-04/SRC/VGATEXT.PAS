{*      VGATEXT.H
 *
 * VGA text output routines for MIDAS Sound System
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*}


unit vgaText;



interface



{****************************************************************************\
*
* Function:     vgaWriteText(x : integer; y : integer; txt : Pchar);
*
* Description:  Writes text on the screen
*
* Input:        x : integer             X coordinate of string (up-left
*                                       corner is (1,1))
*               y : integer             Y coordinate
*               txt : Pchar             pointer to null-terminated text
*                                       string, which may contain also the
*                                       following special characters:
*                                           \xFF - next char is attribute
*                                           \x7F - next char is RLE count for
*                                                  the character following it
*
\****************************************************************************}

procedure vgaWriteText(x : integer; y : integer; txt : Pchar);




{****************************************************************************\
*
* Function:     vgaWriteStr(x : integer; y : integer; str : Pchar;
*               attr : byte; txtlen : integer);
*
* Description:  Writes a string on the screen
*
* Input:        x : integer             X coordinate of the string
*               y : integer             Y coordinate
*               str : Pchar             pointer to a ASCIIZ string
*               attr : byte             attribute for the string
*               txtlen : integer        number of characters to be printed on
*                                       screen - padded with spaces
*
\****************************************************************************}

procedure vgaWriteStr(x : integer; y : integer; str : Pchar; attr : byte;
    txtlen : integer);




{****************************************************************************\
*
* Function:     vgaSetMode(mode : integer);
*
* Description:  Sets a VGA BIOS display mode
*
* Input:        mode : integer          BIOS mode number
*
\****************************************************************************}

procedure vgaSetMode(mode : integer);




{****************************************************************************\
*
* Function:     vgaWriteByte(x : integer; y : integer; bbyte : byte;
*                   attr : byte);
*
* Description:  Writes a hex byte on the screen
*
* Input:        x : integer             X coordinate
*               y : integer             Y coordinate
*               bbyte : byte            byte to be written
*               attr : byte             attribute for the byte
*
\****************************************************************************}

procedure vgaWriteByte(x : integer; y : integer; bbyte : byte; attr : byte);




{****************************************************************************\
*
* Function:     vgaFillRect(x1, y1, x2, y2 : integer; attr : byte);
*
* Description:  Draws a filled rectangle on the screen
*
* Input:        x1 : integer            X-coordinate of upper left corner
*               x2 : integer            Y-coordinate of upper left corner
*               x3 : integer            X-coordinate of lower left corner
*               x4 : integer            Y-coordinate of lower left corner
*               attr : byte             rectangle attribute
*
\****************************************************************************}

procedure vgaFillRect(x1, y1, x2, y2 : integer; attr : byte);




{****************************************************************************\
*
* Function:     vgaDrawChar(x, y : integer; ch : char; attr : byte);
*
* Description:  Draws a single character on the screen
*
* Input:        x : integer             character X-coordinate
*               y : integer             character Y-coordinate
*               ch : char               character
*               attr : byte             character attribute
*
\****************************************************************************}

procedure vgaDrawChar(x, y : integer; ch : char; attr : byte);




{****************************************************************************\
*
* Function:     vgaDrawChars(x, y : integer; ch : char; attr : byte;
*                   num : integer);
*
* Description:  Draws many charactersr on the screen
*
* Input:        x : integer             character X-coordinate
*               y : integer             character Y-coordinate
*               ch : char               character
*               attr : byte             character attribute
*               num : integer           number characters to draw
*
\****************************************************************************}

procedure vgaDrawChars(x, y : integer; ch : char; attr : byte; num : integer);




{****************************************************************************\
*
* Function:     vgaMoveCursor(x, y : integer);
*
* Description:  Moves the text mode cursor to a new location
*
* Input:        x : integer             cursor X-coordinate
*               y : integer             cursor Y-coordinate
*
\****************************************************************************}

procedure vgaMoveCursor(x, y : integer);




implementation



procedure vgaWriteText(x : integer; y : integer; txt : Pchar); external;
procedure vgaWriteStr(x : integer; y : integer; str : Pchar; attr : byte;
    txtlen : integer); external;
procedure vgaSetMode(mode : integer); external;
procedure vgaWriteByte(x : integer; y : integer; bbyte : byte; attr : byte);
     external;
procedure vgaFillRect(x1, y1, x2, y2 : integer; attr : byte); external;
procedure vgaDrawChar(x, y : integer; ch : char; attr : byte); external;
procedure vgaDrawChars(x, y : integer; ch : char; attr : byte; num : integer);
     external;
procedure vgaMoveCursor(x, y : integer); external;
{$L VGATEXT.OBJ}



END.
