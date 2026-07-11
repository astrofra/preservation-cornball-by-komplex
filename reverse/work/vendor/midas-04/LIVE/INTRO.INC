;*      INTRO.INC
;*
;* S2 The Party '94 64kb intro
;* -- Include file for intro global functions and variables
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
;*     Global variables
;\***************************************************************************/


GLOBAL  sinTable : dword                ; pointer to sine table
GLOBAL  cosTable : dword                ; pointer to cosine table




;/***************************************************************************\
;*     Global functions
;\***************************************************************************/

;/***************************************************************************\
;*
;* Function:     int MakeTrigTables(void);
;*
;* Description:  Initializes and allocates the trigonometrical tables
;*               (sinTable and cosTable)
;*
;* Returns:      MIDAS error code
;*
;\***************************************************************************/

GLOBAL  MakeTrigTables : far




;/***************************************************************************\
;*
;* Function:     int iSin(int angle);
;*
;* Description:  Gets a sine value for an angle from the sine table.
;*
;* Input:        int angle               angle, in degrees
;*
;* Returns:      Sine value for the angle, multiplied by 16384.
;*
;\***************************************************************************/

GLOBAL  iSin : far




;/***************************************************************************\
;*
;* Function:     int iCos(int angle);
;*
;* Description:  Gets a cosine value for and angle from the cosine table
;*
;* Input:        int angle               angle, in degrees
;*
;* Returns:      Cosine value for the angle, multiplied by 16384.
;*
;\***************************************************************************/

GLOBAL  iCos : far




;/***************************************************************************\
;*
;* Function:     int iSinMult(int angle, int mult);
;*
;* Description:  Calculates a sine value multiplied by a number.
;*
;* Input:        int angle               angle, in degrees
;*               int mult                multiplier
;*
;* Returns:      mult * sin(angle)
;*
;\***************************************************************************/

GLOBAL  iSinMult : far




;/***************************************************************************\
;*
;* Function:     int iCosMult(int angle, int mult);
;*
;* Description:  Calculates a cosine value multiplied by a number.
;*
;* Input:        int angle               angle, in degrees
;*               int mult                multiplier
;*
;* Returns:      mult * cos(angle)
;*
;\***************************************************************************/

GLOBAL  iCosMult : far
