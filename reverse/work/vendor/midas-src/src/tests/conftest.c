/*      conftest.c
 *
 * MIDAS config test
 *
 * $Id: conftest.c,v 1.2 1997/07/31 10:56:58 pekangas Exp $
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



void MIDASerror(void)
{
    printf("MIDAS error: %s\n", MIDASgetErrorMessage(MIDASgetLastError()));

    MIDASclose();

#ifdef DEBUG
    errPrintList();
#endif

    exit(EXIT_FAILURE);
}



int main(void)
{
/*    unsigned v;*/

    MIDASstartup();

    puts("MIDAS config test\n");

    if ( !MIDASconfig() )
    {
        puts("Cancelled\n");
        return 1;
    }

    puts("OK\n");
    return 0;
}


/*
 * $Log: conftest.c,v $
 * Revision 1.2  1997/07/31 10:56:58  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.1  1997/07/28 13:19:05  pekangas
 * Initial revision
 *
*/
