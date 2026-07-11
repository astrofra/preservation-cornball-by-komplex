/*      module.c
 *
 * A minimal module playing example with the DLL API
 *
 * Copyright 1996,1997 Housemarque Inc.
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
#include <dos.h>
#include "midas.h"
#include "midasdll.h"


static volatile int stopPolling = 0;

void PollaaMIDAS(void)
{
    int         error;
    int         callMP;

    if ( !midasSDInit )
        return;

    if ( (error = midasSD->StartPlay()) != OK )
        midasError(error);
    do
    {
        if ( (error = midasSD->Play(&callMP)) != OK )
            midasError(error);
        if ( callMP )
        {
            if ( midasGMPInit )
            {
                if ( (error = gmpPlay()) != OK )
                    midasError(error);
            }
        }
    } while ( callMP && (midasSD->tempoPoll == 0) && !stopPolling);
}




int main(void)
{
    MIDASmodule module;

    malloc(70000);
    
    /* Error checking has been removed for clarity - see other API examples */

    /* Initialize MIDAS and start background playback: */
    MIDASstartup();

    MIDASconfig();
    
    MIDASinit();
/*    MIDASstartBackgroundPlay(0); */

    /* Load the module and start playing: */
    module = MIDASloadModule("e:/music/effects.s3m");
    MIDASplayModule(module, 0);

    puts("Playing - press any key");

    while ( !kbhit() )
    {
/*        PollaaMIDAS();*/
        delay(10);
    }
    
    getch();

    /* Stop playing and deallocate module: */
    MIDASstopModule(module);
    MIDASfreeModule(module);

    /* Stop background playback and uninitialize MIDAS: */
/*    MIDASstopBackgroundPlay();*/
    MIDASclose();

    return 0;
}
