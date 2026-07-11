{*      MUTILS.PAS
 *
 * Miscellaneous utility functions for MIDAS Sound System used
 * by various system components.
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*}


unit mUtils;


interface



{****************************************************************************\
*
* Function:     mGetKey : integer;
*
* Description:  Waits for a keypress and returns the read key
*
* Returns:      ASCII code for the key pressed. Extended keycodes are
*               returned with bit 8 set, eg. up arrow becomes \x148.
*
\****************************************************************************}

function mGetKey : integer;



{****************************************************************************\
*
* Function:     mStrLength(str : Pchar) : integer;
*
* Description:  Calculates the length of a ASCIIZ string
*
* Input:        str : Pchar             pointer to string
*
* Returns:      String length excluding the terminating '\0'.
*
\****************************************************************************}

function mStrLength(str : Pchar) : integer;




{****************************************************************************\
*
* Function:     mStrCopy(dest : Pchar; src : Pchar);
*
* Description:  Copies an ASCIIZ string from src^ to dest^.
*
* Input:        dest : Pchar            pointer to destination string
*               src : Pchar             pointer to source string
*
\****************************************************************************}

procedure mStrCopy(dest : Pchar; src : Pchar);




{****************************************************************************\
*
* Function:     mStrAppend(dest : Pchar; src : Pchar);
*
* Description:  Appends an ASCIIZ string to the end of another.
*
* Input:        dest : Pchar            pointer to destination string
*               src : Pchar             pointer to source string
*
\****************************************************************************}

procedure mStrAppend(dest : Pchar; src : Pchar);



{****************************************************************************\
*
* Function:     mMemCopy(dest : pointer; src : pointer; numBytes : word);
*
* Description:  Copies a memory block from src^ to dest^.
*
* Input:        dest : pointer          pointer to destination
*               src : pointer           pointer to source
*               numBytes : word         number of bytes to copy
*
\****************************************************************************}

procedure mMemCopy(dest : pointer; src : pointer; numBytes : word);




{****************************************************************************\
*
* Function:     mMemEqual(m1 : pointer; m2 : pointer; numBytes : word) :
*                   integer;
*
* Description:  Compares two memory blocks.
*
* Input:        m1 : pointer            pointer to memory block #1
*               m2 : pointer            pointer to memory block #2
*               numBytes : word         number of bytes to compare
*
* Returns:      1 if the memory blocks are equal, 0 if not.
*
\****************************************************************************}

function mMemEqual(m1 : pointer; m2 : pointer; numBytes : word) : integer;




{****************************************************************************\
*
* Function:     mHex2Long(hex : Pchar) : longint;
*
* Description:  Converts a hexadecimal string to a long integer.
*
* Input:        hex : Pchar             pointer to hex string, ASCIIZ
*
* Returns:      Value of the string or -1 if conversion failure.
*
\****************************************************************************}

function mHex2Long(hex : Pchar) : longint;




{****************************************************************************\
*
* Function:     mDec2Long(dec : Pchar) : longint;
*
* Description:  Converts an unsigned decimal string to a long integer
*
* Input:        dec : Pchar             pointer to string, ASCIIZ
*
* Returns:      Value of the string or -1 if conversion failure.
*
\****************************************************************************}

function mDec2Long(dec : Pchar) : longint;




{***************************************************************************\
*
* Function:     mGetEnv(envVar : Pchar) : Pchar;
*
* Description:  Searches a string from the environment
*
* Input:        envVar                  environment variable name, ASCIIZ
*
* Returns:      Pointer to environment string value (ASCIIZ), NULL if string
*               was not found.
*
\***************************************************************************}

function mGetEnv(envVar : Pchar) : Pchar;




implementation



function mGetKey : integer; external;
function mStrLength(str : Pchar) : integer; external;
procedure mStrCopy(dest : Pchar; src : Pchar); external;
procedure mStrAppend(dest : Pchar; src : Pchar); external;
procedure mMemCopy(dest : pointer; src : pointer; numBytes : word); external;
function mMemEqual(m1 : pointer; m2 : pointer; numBytes : word) : integer;
    external;
function mHex2Long(hex : Pchar) : longint; external;
function mDec2Long(dec : Pchar) : longint; external;
function mGetEnv(envVar : Pchar) : Pchar; external;
{$L MUTILS.OBJ}


END.
