/*      PLAYMM.C
 *
 * PLAYMM v1.10 .MM module player
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
#include <string.h>
#include <conio.h>

#ifdef __WATCOMC__
#include <malloc.h>
#else
#include <alloc.h>
#endif

#include "midas.h"


char *title =
"PLAYMM v1.10 .MM module player\n"
"Copyright 1995 Petteri Kangaslampi and Jarno Paananen\n";

char *usage =
"Usage:\tPLAYMM\t<filename> [MIDAS options]";


    /* pointers to all Module Players: */
ModulePlayer    *modulePlayers[NUMMPLAYERS] =
    { &mpS3M,
      &mpMOD,
      &mpMTM };

    /* module type strings: */
char            *moduleTypeStr[NUMMPLAYERS] = {
    { "Scream Tracker ]I[" },
    { "Protracker" },
    { "Multitracker" } };




/****************************************************************************\
*
* Function:     void Error(char *msg)
*
* Description:  Prints an error message to stderr and exits to DOS
*
* Input:        char *msg               error message
*
\****************************************************************************/

void Error(char *msg)
{
    fprintf(stderr, "Error: %s\n", msg);
    midasClose();
    exit(EXIT_FAILURE);
}



int main(int argc, char *argv[])
{
    int         i;
    int         error;
    long        mmSize;
    fileHandle  f;
    uchar       huge *mm;
    mpModule    *module;
    mpInformation *info;
#ifdef __BORLANDC__
    ulong       free1;
#endif
    ModulePlayer *MP;
    int         mpID;

#ifdef __BORLANDC__
    free1 = coreleft();
#endif

    puts(title);
    if ( argc < 2 )
    {
        puts(usage);
        exit(EXIT_SUCCESS);
    }

    midasSetDefaults();
    midasConfig();
    vgaSetMode(3);

    midasDisableEMS = 1;

    for ( i = 2; i < argc; i++ )
        midasParseOption(&argv[i][1]);

    midasInit();

    if ( (error = fileOpen(argv[1], fileOpenRead, &f)) != OK )
        Error(errorMsg[error]);
    if ( (error = fileGetSize(f, &mmSize)) != OK )
        Error(errorMsg[error]);

#ifdef __WATCOMC__
    mm = (uchar huge*) halloc(mmSize, 1);
#else
    mm = (uchar huge*) farmalloc(mmSize);
#endif
    if ( mm == NULL )
        Error("Out of memory");

    if ( (error = fileRead(f, mm, mmSize)) != OK )
        Error(errorMsg[error]);
    if ( (error = fileClose(f)) != OK )
        Error(errorMsg[error]);

    module = (mpModule*) mm;
    mpID = ((mpModule*) mm)->IDnum;
    if ( mpID >= NUMMPLAYERS )
        Error("Invalid module file");
    MP = modulePlayers[mpID];

    module = midasPrepareMM((uchar*) mm, MP);

    midasPlayModule(module, 0);

    puts("Playing - Press any key to stop");
    printf("Song \"%s\", originally a %s module\n", &module->songName[0],
        moduleTypeStr[mpID]);
    while ( !kbhit() )
    {
        if ( (error = midasMP->GetInformation(&info)) != OK )
            midasError(error);
        printf("%02X %02X %02X\r", (unsigned) info->pos,
            (unsigned) info->pattern, (unsigned) info->row);
    }

    getch();
    midasStopModule(module);
    midasFreeMM(module);

#ifdef __WATCOMC__
    hfree(mm);
#else
    farfree(mm);
#endif

    midasClose();

#ifdef DEBUG
    errPrintList();
#endif

#ifdef __BORLANDC__
    printf("Done. Memory lost: %lu bytes\n", free1 - coreleft());
#endif

    return 0;
}
