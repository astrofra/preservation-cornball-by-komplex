{*      MPARSER.PAS
 *
 * MIDAS Sound System command line parser. Meant to be used with the
 * simplified MIDAS API, MIDAS.C
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*}


unit mParser;


interface



{****************************************************************************\
*
* Function:     mParserError(msg : Pchar);
*
* Description:  Prints a parser error message to stderr and exits to DOS.
*
* Input:        msg : Pchar             error message
*
\****************************************************************************}

procedure mParserError(msg : Pchar);




{****************************************************************************\
*
* Function:     midasParseOption(option : Pchar);
*
* Description:  Parses one MIDAS command line option.
*
* Input:        option : Pchar          Command line option string WITHOUT
*                                       the leading '-' or '/'.
*
* Recognized options:
*       -sx     Force Sound Device x (1 = GUS, 2 = PAS, 3 = WSS, 4 = SB,
*               5 = No Sound)
*       -pxxx   Force I/O port xxx (hex) for Sound Device
*       -ix     Force IRQ x for Sound Device
*       -dx     Force DMA channel x for Sound Device
*       -cx     Force sound card type x for Sound Device
*       -mxxxx  Set mixing rate to xxxx Hz
*       -oxxx   Force output mode (8 = 8-bit, 1 = 16-bit, s = stereo,
*               m = mono)
*       -e      Disable EMS usage
*       -t      Disable ProTracker BPM tempos
*       -u      Enable Surround sound
*       -v      Disable real VU-meters
*       -b      Disable Virtual DMA usage
*       -ax     Force amplification level x
*
\****************************************************************************}

procedure midasParseOption(option : Pchar);



{****************************************************************************\
*
* Function:     midasParseEnvironment;
*
* Description:  Parses the MIDAS environment string, which has same format
*               as the command line options.
*
\****************************************************************************}

procedure midasParseEnvironment;



implementation


uses MIDAS, mUtils, mMem, mGlobals, Errors;


procedure mParserError(msg : Pchar); external;
procedure midasParseOption(option : Pchar); external;
procedure midasParseEnvironment; external;
{$L MPARSER.OBJ}


END.
