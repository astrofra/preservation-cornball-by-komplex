/*      wave.c
 *
 * RIFF WAVE reader test
 *
 * $Id: wave.c,v 1.3 1997/07/31 10:56:59 pekangas Exp $
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
#include "midas.h"
#include "midasdll.h"



void MIDASerror(int error)
{
    printf("MIDAS error: %s\n", MIDASgetErrorMessage(error));

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
    int error;
    static mWaveState *state;

    MIDASstartup();

    puts("MIDAS RIFF WAVE reader test\n");

    if ( argc != 2 )
    {
        puts("Usage: musfade wavefile");
        return 1;
    }

    puts("Opening file and reading info");
    if ( (error = mWaveOpen(argv[1], &state)) != OK )
        MIDASerror(error);

    printf("Sample type = %u\n", state->sampleType);
    printf("Sample rate = %u\n", state->sampleRate);
    printf("Data length = %u bytes\n", state->dataLength);
    printf("Data offset = %u\n", state->dataStart);

    puts("Closing file");
    if ( (error = mWaveClose(state)) != OK )
        MIDASerror(error);

    return 0;
}


/*
 * $Log: wave.c,v $
 * Revision 1.3  1997/07/31 10:56:59  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.2  1997/07/28 13:19:20  pekangas
 * Removed unnecessary #include <windows.h>
 *
 * Revision 1.1  1997/05/20 15:19:03  pekangas
 * Initial revision
 *
*/
