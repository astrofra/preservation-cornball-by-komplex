/*      echo.c
 *
 * MIDAS echo effect engine test
 *
 * $Id: echotest.c,v 1.3 1997/07/31 10:56:58 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "midasdll.h"
#include "midas.h"
#include "mecho.h"



void MIDASerror(void)
{
    printf("MIDAS error: %s\n", MIDASgetErrorMessage(MIDASgetLastError()));

    MIDASclose();

#ifdef DEBUG
    errPrintList();
#endif

    exit(EXIT_FAILURE);
}



void Error(char *msg)
{
    printf("Error: %s\n", msg);

    MIDASclose();

#ifdef DEBUG
    errPrintList();
#endif    

    exit(EXIT_FAILURE);
}


void mError(int error)
{
    printf("MIDAS error: %s\n", MIDASgetErrorMessage(error));

    MIDASclose();

#ifdef DEBUG
    errPrintList();
#endif

    exit(EXIT_FAILURE);
}


mEcho simpleEchos[] =
{
    { 250*65536, 0x4000, 0 }
};

mEchoSet simpleEcho =
{
    0x1000,
    0x10000,
    1,
    simpleEchos
};


mEcho reverseEchos[] =
{
    { 250*65536, 0x4000, 1 }
};

mEchoSet reverseEcho =
{
    0x1000,
    0x10000,
    1,
    reverseEchos
};



int main(int argc, char *argv[])
{
    int error;
    MIDASmodule module;
    MIDASmodulePlayHandle playHandle;
    static mEchoHandle eh;

    MIDASstartup();

    puts("MIDAS echo effect engine test\n");

    if ( argc != 2 )
    {
        puts("Usage: echo module");
        return 1;
    }

    MIDASsetOption(MIDAS_OPTION_MIXING_MODE, MIDAS_MIX_HIGH_QUALITY);
    if ( !MIDASinit() )
        MIDASerror();
    if ( !MIDASstartBackgroundPlay(0) )
        MIDASerror();

    if ( (error = mEchoInit()) != OK )
        mError(error);

    if ( (module = MIDASloadModule(argv[1])) == NULL )
        MIDASerror();
    
    if ( (playHandle = MIDASplayModule(module, TRUE)) == 0 )
        MIDASerror();

    
    if ( (error = mEchoAddEffect(&simpleEcho, &eh)) != OK )
        mError(error);

    puts("Simple echo - press any key");
    getch();

    if ( (error = mEchoRemoveEffect(eh)) != OK )
        mError(error);

    if ( (error = mEchoAddEffect(&reverseEcho, &eh)) != OK )
        mError(error);

    puts("Channel-reverse echo - press any key");
    getch();

    if ( (error = mEchoRemoveEffect(eh)) != OK )
        mError(error);

    puts("No echo - press any key");
    getch();

    if ( !MIDASstopModule(playHandle) )
        MIDASerror();
    if ( !MIDASfreeModule(module) )
        MIDASerror();

    if ( (error = mEchoClose()) != OK )
        mError(error);
    
    if ( !MIDASstopBackgroundPlay() )
        MIDASerror();
    if ( !MIDASclose() )
        MIDASerror();

    return 0;
}


/*
 * $Log: echotest.c,v $
 * Revision 1.3  1997/07/31 10:56:58  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.2  1997/07/28 13:18:25  pekangas
 * Fixed to compile in DOS, both with Watcom C and GCC
 *
 * Revision 1.1  1997/07/26 14:15:39  pekangas
 * Initial revision
 *
 * Revision 1.3  1997/05/20 15:18:49  pekangas
 * Fixed to work with updated API
 *
 * Revision 1.2  1997/04/08 19:17:25  pekangas
 * Initial revision
 *
*/
