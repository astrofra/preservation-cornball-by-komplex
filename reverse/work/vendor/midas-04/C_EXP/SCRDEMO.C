/*      SCRDEMO.C
 *
 * Example on using screen synchronized timer for scrolling
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
#include <stdlib.h>
#include <conio.h>
#include <process.h>
#include "midas.h"


signed char     horizPan;               /* Horizontal Pixel Panning register
                                           value */
ushort          startAddr;              /* screen start address */
ushort          scrSync;                /* screen synchronization value */
int             panDir;                 /* panning direction - 0 = left,
                                           1 = right */


#ifdef __WATCOMC__
void SetStartAddr(ushort addr);
#pragma aux SetStartAddr = \
        "mov    dx,03D4h" \
        "mov    al,0Ch" \
        "mov    ah,bh" \
        "out    dx,ax" \
        "mov    al,0Dh" \
        "mov    ah,bl" \
        "out    dx,ax" \
        parm [bx] \
        modify exact [ax dx];

void SetHorizPan(uchar horizPan);
#pragma aux SetHorizPan = \
        "mov    dx,03DAh" \
        "in     al,dx" \
        "mov    dx,03C0h" \
        "mov    al,33h" \
        "out    dx,al" \
        "mov    al,bl" \
        "out    dx,al" \
        parm [bl] \
        modify exact [ax dx];
#endif



/****************************************************************************\
*
* Function:     void preVR(void)
*
* Description:  Function that is called before Vertical Retrace. Sets the
*               new screen start address
*
\****************************************************************************/

void CALLING preVR(void)
{
#ifdef __BORLANDC__
asm {
        mov     bx,startAddr            /* bx = screen start address */

        mov     dx,03D4h                /* CRTC controller */
        mov     al,0Ch                  /* Start Address High register */
        mov     ah,bh                   /* screen start address high byte */
        out     dx,ax                   /* set register value */

        mov     al,0Dh                  /* Start Address Low register */
        mov     ah,bl                   /* screen start address low byte */
        out     dx,ax                   /* set register value */
}
#endif
#ifdef __WATCOMC__
    SetStartAddr(startAddr);
#endif
}



/****************************************************************************\
*
* Function:     void immVR(void)
*
* Description:  Function that is called immediately when Vertical Retrace
*               starts. Sets the new Horizontal Pixel Panning value
*
\****************************************************************************/

void CALLING immVR(void)
{
#ifdef __BORLANDC__
asm {   mov     dx,03DAh                /* read Input Status #1 register to */
        in      al,dx                   /* reset the Attribute Controller */
                                        /* flip-flop */
        mov     dx,03C0h                /* attribute controller */
        mov     al,13h + 20h            /* Horizontal Pixel Panning register,
                                           enable VGA palette */
        out     dx,al                   /* select register */
        mov     al,horizPan             /* Horizontal Pixel Panning value */
        out     dx,al                   /* write panning value */
}
#endif
#ifdef __WATCOMC__
    SetHorizPan(horizPan);
#endif
}




/****************************************************************************\
*
* Function:     void inVR(void)
*
* Description:  Function that is called some time during Vertical Retrace.
*               Calculates new Horizontal Pixel Panning and screen start
*               address values
*
\****************************************************************************/

void CALLING inVR(void)
{
    /* Note! Although this function does not cause timer synchronization
       errors if its execution takes too long, it may still cause errors
       to playing tempo when playing with GUS. */

    if ( panDir == 0 )
    {
        /* pan display one pixel left: */

        horizPan++;                     /* next pixel */

        if ( horizPan == 9 )            /* is panning 9? */
            horizPan = 0;               /* if yes, set it to 0 */
        if ( horizPan == 8 )            /* is panning 8? */
            startAddr++;                /* if yes, move to next character */

        if ( startAddr == 80 )          /* change direction after */
            panDir = 1;                 /* scrolling one screen */
    }
    else
    {
        /* pan display one pixel right: */

        horizPan--;

        if ( horizPan == -1 )           /* is panning -1? */
            horizPan = 8;               /* if yes, set it to 8 */
        if ( horizPan == 7 )            /* is panning 7? */
            startAddr--;                /* if yes, move to next character */

        if ( (startAddr == 0) && (horizPan == 8) )  /* change direction */
            panDir = 0;                 /* after scrolling back one screen */
    }

    /* note that charaters are actually 9 pixels wide on VGA */
}




int main(int argc, char *argv[])
{
    mpModule    *mod;
    int         error, i, isConfig;

    /* Check that there is exactly one argument: */
    if  ( argc != 2 )
    {
        puts("Usage: SCRDEMO <filename>");
        exit(EXIT_SUCCESS);
    }

    /* Check that the configuration file exists: */
    if ( (error = fileExists("MIDAS.CFG", &isConfig)) != OK )
        midasError(error);
    if ( !isConfig )
    {
        puts("Configuration file not found - run MSETUP.EXE");
        exit(EXIT_FAILURE);
    }

    midasSetDefaults();                 /* set MIDAS defaults */
    midasLoadConfig("MIDAS.CFG");       /* load configuration */

    /* get Timer screen synchronization value: */
    if ( (error = tmrGetScrSync(&scrSync)) != OK )
        midasError(error);

    midasInit();                        /* initialize MIDAS Sound System */

    mod = midasLoadModule(argv[1], &mpMOD, NULL);  /* load module */
    midasPlayModule(mod, 0);            /* start playing */

    puts("Playing - type \"EXIT\" to stop.");

    horizPan = 8;
    startAddr = 0;
    panDir = 0;

    /* Synchronize Timer to screen. preVR() will be called before
       Vertical Retrace, immVR() just after Vertical Retrace starts and
       inVR() some time later during Vertical Retrace. */
    if ( (error = tmrSyncScr(scrSync, &preVR, &immVR, &inVR)) != OK )
        midasError(error);

    /* jump to DOS shell: */
    spawnl(P_WAIT, getenv("COMSPEC"), NULL);

    /* stop timer screen synchronization: */
    tmrStopScrSync();

    /* reset panning and start address: (dirty but works) */
    horizPan = 8;
    startAddr = 0;
    preVR();
    immVR();

    midasStopModule(mod);               /* stop playing */
    midasFreeModule(mod);               /* deallocate module */
    midasClose();                       /* uninitialize MIDAS */

    return 0;
}
