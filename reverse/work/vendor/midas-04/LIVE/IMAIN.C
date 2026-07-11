/*      IMAIN.C
 *
 * S2 The Party '94 64kb intro
 * -- Main program file
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
#include <midas.h>
#include "intro.h"

// #define NOMUSIC


mpModule        *module;


volatile ulong      frameCount;
volatile unsigned   scrStart;

mpInformation       *info;

uchar           *fadeTable;



/****************************************************************************\
*
* Function:     void Error(char *msg)
*
* Description:  Prints an error message and exits to DOS.
*
* Input:        char *msg               error message
*
\****************************************************************************/

void cdecl Error(char *msg)
{
    midasClose();
    /* set normal text mode: */
    errErrorExit(msg);
}



/****************************************************************************\
*
* Function:     void preVR(void)
*
* Description:  preVR() function for screen timer. Note that for this to
*               always work, you must use the command line option "-zu",
*               to ensure that Watcom won't try to access the variables
*               here through the stack segment, as it might not be what it
*               expects (we are in an interrupt here). It might be useful
*               not to use that command line option with other modules though,
*               as it causes Watcom to always load the DS segment when it
*               needs to reference a global variable
*
\****************************************************************************/

void CALLING preVR(void)
{
    SetScrStart(scrStart);
    frameCount++;
}




/****************************************************************************\
*
* Function:     void WaitFrame(void)
*
* Description:  Waits for a Vertical Retrace
*
\****************************************************************************/

void WaitFrame(void)
{
    ulong       temp = frameCount;

#ifdef NOMUSIC
    WaitDE();
    SetScrStart(scrStart);
    WaitVR();
    frameCount++;
#else
    while ( frameCount == temp );
#endif
}



/****************************************************************************\
*
* Function:     void UpdInfo(void)
*
* Description:  Updates module player information structure
*
\****************************************************************************/

void UpdInfo(void)
{
    int         error;
#ifndef NOMUSIC
    if ( (error = midasMP->GetInformation(&info)) != OK )
        midasError(error);
#endif
}



int main(void)
{
    int         error;
    ushort      scrSync;
    int         a, b;

    if ( (error = MakeTrigTables()) != OK )
        Error(errorMsg[error]);

    if ( (error = memAlloc(65*256, (void**) &fadeTable)) != OK )
        Error(errorMsg[error]);

    for ( a = 0; a < 65; a++ )
    {
        for ( b = 0; b < 256; b++ )
        {
            fadeTable[(a << 8) + b] = (uchar) ((int) (a * b) / 64);
        }
    }

    /* Initialize MIDAS Sound System: */
    midasSetDefaults();
    midasConfig();
    midasDisableEMS = 1;

    /* Set up 320x200x256 non Chain Four display mode: */
    vgaSetC4();

    scrStart = 0;

#ifndef NOMUSIC
    if ( (error = tmrGetScrSync(&scrSync)) != OK )
        Error(errorMsg[error]);

    midasInit();
#endif

#ifndef NOMUSIC
    /* Start playing the music: */
    module = midasPrepareMM(&music[0], &mpMOD);
    midasPlayModule(module, 0);

    /* Synchronize timer to the screen: */
    if ( (error = tmrSyncScr(scrSync, (void CALLING (*)())&preVR, NULL,
        NULL)) != OK )
        Error(errorMsg[error]);
#endif

    scrStart = 0;
    frameCount = 0;


    Writer();
    PolygonTunnel();
    LineTunnel();
    ShadebobZoomer();
    EndText();

    do
        UpdInfo();
    while ( (info->pos != 0x1D) || (info->row < 0x3D) );

#ifndef NOMUSIC
    /* Stop music: */
    midasStopModule(module);
    midasFreeMM(module);

    /* Stop timer screen synchronization: */
    if ( (error = tmrStopScrSync()) != OK )
        Error(errorMsg[error]);

    /* Uninitialize MIDAS Sound System: */
    midasClose();
#endif

    /* Set normal text mode: */
    SetMode(0x03);

    if ( (error = memFree(sinTable)) != OK )
        Error(errorMsg[error]);

    if ( (error = memFree(fadeTable)) != OK )
        Error(errorMsg[error]);

    return 0;
}
