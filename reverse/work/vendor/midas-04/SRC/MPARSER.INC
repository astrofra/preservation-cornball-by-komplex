;*      MPARSER.INC
;*
;* MIDAS Sound System command line parser. Meant to be used with the
;* simplified MIDAS API, MIDAS.C
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*



;/***************************************************************************\
;*
;* Function:    void mParserError(char *msg)
;*
;* Description: Prints a parser error message to stderr and exits to DOS.
;*
;* Input:       char *msg               error message
;*
;\***************************************************************************/

GLOBAL  LANG mParserError : far




;/***************************************************************************\
;*
;* Function:    void midasParseOption(char *option)
;*
;* Description: Parses one MIDAS command line option.
;*
;* Input:       char *option            Command line option string WITHOUT
;*                                      the leading '-' or '/'.
;*
;* Recognized options:
;*      -sx     Force Sound Device x (1 = GUS, 2 = PAS, 3 = WSS, 4 = SB,
;*              5 = No Sound)
;*      -pxxx   Force I/O port xxx (hex) for Sound Device
;*      -ix     Force IRQ x for Sound Device
;*      -dx     Force DMA channel x for Sound Device
;*      -cx     Force sound card type x for Sound Device
;*      -mxxxx  Set mixing rate to xxxx Hz
;*      -oxxx   Force output mode (8 = 8-bit, 1 = 16-bit, s = stereo,
;*              m = mono)
;*      -e      Disable EMS usage
;*      -t      Disable ProTracker BPM tempos
;*      -u      Enable Surround sound
;*      -b      Disable Virtual DMA usage
;*      -ax     Force amplification level x
;*
;\***************************************************************************/

GLOBAL  LANG midasParseOption : far




;/***************************************************************************\
;*
;* Function:    void midasParseEnvironment(void)
;*
;* Description: Parses the MIDAS environment string, which has same format
;*              as the command line options.
;*
;\***************************************************************************/

GLOBAL  LANG midasParseEnvironment : far
