/*      thread.c
 *
 * MIDAS music channel fade test
 *
 * $Id: musfade.c,v 1.4 1997/07/31 10:56:58 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <process.h>
#include "midasdll.h"



void MIDASerror(void)
{
    printf("MIDAS error: %s\n", MIDASgetErrorMessage(MIDASgetLastError()));

    MIDASclose();

    exit(EXIT_FAILURE);
}



void Error(char *msg)
{
    printf("Error: %s\n", msg);

    MIDASclose();

    exit(EXIT_FAILURE);
}



int main(int argc, char *argv[])
{
    MIDASmodule module;
    MIDASmodulePlayHandle playHandle;
    unsigned    numChannels, chan, fade, down;
    static MIDASmoduleInfo info;

    MIDASstartup();

    puts("MIDAS music channel fade test\n");

    if ( argc != 2 )
    {
        puts("Usage: musfade module");
        return 1;
    }
    
    /* Initialize MIDAS and start background playback: */
    if ( !MIDASinit() )
        MIDASerror();
    if ( !MIDASstartBackgroundPlay(0) )
        MIDASerror();

    if ( (module = MIDASloadModule(argv[1])) == NULL )
        MIDASerror();
    
    if ( (playHandle = MIDASplayModule(module, TRUE)) == 0 )
        MIDASerror();

    chan = 0; fade = 64; down = 1;
    if ( !MIDASgetModuleInfo(module, &info) )
        MIDASerror();
    numChannels = info.numChannels;

    while ( !kbhit() )
    {
        if ( down )
        {
            if ( fade > 0 )
            {
                fade--;
            }
            else
            {
                fade = 64;
                chan++;
                if ( chan >= numChannels )
                {
                    chan = 0;
                    fade = 0;
                    down = 0;
                }
            }
        }
        else
        {
            if ( fade < 64 )
            {
                fade++;
            }
            else
            {
                fade = 0;
                chan++;
                if ( chan >= numChannels )
                {
                    chan = 0;
                    fade = 64;
                    down = 1;
                }
                Sleep(500);
            }
        }

        if ( !MIDASfadeMusicChannel(playHandle, chan, fade) )
            MIDASerror();
    
        Sleep(10);
    }

    getch();

    if ( !MIDASstopModule(playHandle) )
        MIDASerror();
    if ( !MIDASfreeModule(module) )
        MIDASerror();
    if ( !MIDASstopBackgroundPlay() )
        MIDASerror();
    if ( !MIDASclose() )
        MIDASerror();

    return 0;
}


/*
 * $Log: musfade.c,v $
 * Revision 1.4  1997/07/31 10:56:58  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.3  1997/05/20 15:18:49  pekangas
 * Fixed to work with updated API
 *
 * Revision 1.2  1997/04/08 19:17:25  pekangas
 * Initial revision
 *
*/
