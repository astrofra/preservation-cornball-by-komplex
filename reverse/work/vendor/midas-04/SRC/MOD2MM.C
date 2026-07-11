/*      MOD2MM.C
 *
 * MOD2MM v1.10 .MM module converter
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
#ifdef __BORLANDC__
#include <alloc.h>
#endif
#include "midas.h"

char            *title =
"MOD2MM v1.10 .MM module converter for MIDAS Sound System v" MVERSTR "\n"
"Copyright 1995 Petteri Kangaslampi and Jarno Paananen\n";

char            *usage =
"Usage:\tMOD2MM\t<Sourcefile> <MMfile> [options]\n"
"    <Sourcefile>    Name of source module file\n"
"    <MMfile>        Name of destination .MM module file\n"
"Options:\n"
"    -q              Be quiet - no messages\n";

fileHandle      outf;                   /* output file handle */
int             ofOpen = 1;             /* 1 if output file is open */

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
    if ( ofOpen == 1 )
        fileClose(outf);
    exit(EXIT_FAILURE);
}




/****************************************************************************\
*
* Function:     int DetectModuleType(char *fileName)
*
* Description:  Detects the type of a module file
*
* Input:        char *fileName          module file name
*
* Returns:      Module type ID - 0 for S3M, 1 for MOD and 2 for MTM module.
*
\****************************************************************************/

int DetectModuleType(char *fileName)
{
    uchar       *header;
    fileHandle  f;
    int         error, mpNum, recognized, found;

    /* allocate memory for module header: */
    if ( (error = memAlloc(MPHDRSIZE, (void**) &header)) != OK )
        midasError(error);

    /* open module file: */
    if ( (error = fileOpen(fileName, fileOpenRead, &f)) != OK )
        midasError(error);

    /* read MPHDRSIZE bytes of module header: */
    if ( (error = fileRead(f, header, MPHDRSIZE)) != OK )
        midasError(error);

    if ( (error = fileClose(f)) != OK )
        midasError(error);

    /* Search through all Module Players to find one that recognizes the
       file header: */
    mpNum = 0; found = 0;
    while ( (mpNum < NUMMPLAYERS) && (!found) )
    {
        if ( (error = modulePlayers[mpNum]->Identify(header, &recognized))
            != OK )
            midasError(error);
        if ( recognized )
            found = 1;
        else
            mpNum++;
    }

    /* deallocate module header: */
    if ( (error = memFree(header)) != OK )
        midasError(error);

    if ( found == 0 )
        Error("Unknown module format");

    /* return module type ID: */
    return mpNum;
}




/****************************************************************************\
*
* Function:     void SaveMM(char *fileName, mpModule *module)
*
* Description:  Saves a .MM module
*
* Input:        char *fileName          module file name
*               mpModule *module        module to be saved
*
\****************************************************************************/

void SaveMM(char *fileName, mpModule *module)
{
    int         error;
    int         i;
    unsigned    a;
    uchar       b, d;

    /* open output file: */
    if ( (error = fileOpen(fileName, fileOpenWrite, &outf)) != OK )
        Error(errorMsg[error]);
    ofOpen = 1;

    /* write module header: */
    if ( (error = fileWrite(outf, module, sizeof(mpModule))) != OK )
        Error(errorMsg[error]);

    /* write pattern playing orders: */
    if ( (error = fileWrite(outf, module->orders, module->songLength))
        != OK )
        Error(errorMsg[error]);

    /* write instrument headers: */
    if ( (error = fileWrite(outf, module->insts,  module->numInsts *
        sizeof(mpInstrument))) != OK )
        Error(errorMsg[error]);

    /* write pattern data: */
    for ( i = 0; i < module->numPatts; i++ )
    {
        if ( (fileWrite(outf, module->patterns[i],
            module->patterns[i]->length + sizeof(mpPattern))) != OK )
        {
            Error(errorMsg[error]);
        }
    }

    /* write instruments used table: */
    if ( (error = fileWrite(outf, module->instsUsed, module->numInsts))
        != OK )
        Error(errorMsg[error]);

    /* write sample data for used instruments: */
    for ( i = 0; i < module->numInsts; i++ )
    {
        if ( (module->instsUsed[i] == 1) && (module->insts[i].length != 0) )
        {
            /* convert sample data to delta format: */
            b = 0;
            for ( a = 0; a < module->insts[i].length; a++ )
            {
                d = module->insts[i].sample[a] - b;
                b = b + d;
                module->insts[i].sample[a] = d;
            }

            /* write converted sample: */
            if ( (fileWrite(outf, module->insts[i].sample,
                module->insts[i].length)) != OK )
                Error(errorMsg[error]);
        }
    }

    /* close output file: */
    if ( (error = fileClose(outf)) != OK )
        Error(errorMsg[error]);
    ofOpen = 0;
}



int main(int argc, char *argv[])
{
    int         error;
    mpModule    *module;
#ifdef __BORLANDC__
    ulong       free1;
#endif
    int         moduleType;
    ModulePlayer  *MP;
    int         quiet = 0;

    /* EMS MUST be disable when using .MM modules: */
    useEMS = 0;

#ifdef __BORLANDC__
    /* Save the size of largest available free block to detect memory leaks:*/
    free1 = coreleft();
#endif

    /* Check if quiet mode should be used: */
    if ( (argc == 4) )
    {
        if ( ((argv[3][0] == '-') || (argv[3][0] == '/')) &&
             ((argv[3][1] == 'q') || (argv[3][1] == 'Q')) )
            quiet = 1;
    }

    if ( !quiet )
        puts(title);

    if ( (argc < 3) || (argc > 4) )
    {
        puts(usage);
        exit(EXIT_SUCCESS);
    }

    /* Detect module file type: */
    moduleType = DetectModuleType(argv[1]);
    MP = modulePlayers[moduleType];

    if ( !quiet )
        printf("Loading %s module %s\n", moduleTypeStr[moduleType], argv[1]);

    /* Load the module without adding instruments to a Sound Device: */
    if ( (error = MP->LoadModule(argv[1], NULL, NULL, &module)) != OK )
        midasError(error);

    if ( !quiet )
    printf("Saving .MM module %s\n", argv[2]);

    /* save .MM module: */
    SaveMM(argv[2], module);

    /* deallocate module: */
    if ( (error = MP->FreeModule(module, NULL)) != OK )
        Error(errorMsg[error]);

#ifdef __BORLANDC__
    if ( !quiet )
        printf("Done. Memory lost: %lu bytes\n", free1 - coreleft());
#endif

    return 0;
}
