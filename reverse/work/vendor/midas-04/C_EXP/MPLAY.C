/*      MPLAY.C
 *
 * Minimal Protracker module player using MIDAS Sound System
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
#include "midas.h"

char            *usage =
"Usage:\tMPLAY\t<filename>";



int main(int argc, char *argv[])
{
    mpModule    *mod;
    int         i, error, isConfig;

    /* Check that there is only one argument - the file name: */
    if  ( argc != 2 )
    {
        puts(usage);
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
    midasInit();                        /* initialize MIDAS Sound System */
    mod = midasLoadModule(argv[1], &mpMOD, NULL);  /* load module */
    midasPlayModule(mod, 0);            /* start playing */

    puts("Playing - press any key...");
    getch();                            /* wait for a keypress */

    midasStopModule(mod);               /* stop playing */
    midasFreeModule(mod);               /* deallocate module */
    midasClose();                       /* uninitialize MIDAS */

    return 0;
}
