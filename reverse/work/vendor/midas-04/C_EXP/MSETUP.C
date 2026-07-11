/*      MSETUP.C
 *
 * MIDAS Sound System configuration program
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


int main(void)
{
    int         configured;


    midasSetDefaults();                 /* set MIDAS defaults */

    /* Run MIDAS Sound System configuration: */
    configured = midasConfig();

    /* Reset display mode: */
    vgaSetMode(0x03);

    if ( configured )
    {
        /* Configuration succesful - save configuration file: */
        midasSaveConfig("MIDAS.CFG");
        puts("Configuration written to MIDAS.CFG");
    }
    else
    {
        /* Configuration unsuccessful: */
        puts("Configuration NOT saved");
    }

    return 0;
}
