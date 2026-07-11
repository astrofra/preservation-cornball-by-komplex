/*      ERRORS.C
 *
 * MIDAS Digital Audio System error codes and error message strings
 *
 * $Id: errors.c,v 1.13 1997/07/31 10:56:40 pekangas Exp $
 *
 * Copyright 1996 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include <stdio.h>
#include <stdlib.h>
#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#ifndef __UNIX__
#include "vgatext.h"
#endif

RCSID(const char *errors_rcsid = "$Id: errors.c,v 1.13 1997/07/31 10:56:40 pekangas Exp $";)


/* maximum length of an error message is 40 characters! */

char            *errorMsg[] =
{
    "OK",
    "Undefined error",
#ifdef __REALMODE__
    "Out of conventional memory",
    "Conventional memory heap corrupted",
    "Invalid conventional memory block",
#else
    "Out of memory",
    "Heap corrupted",
    "Invalid memory block",
#endif
    "Out of EMS memory",
    "EMS memory heap corrupted",
    "Invalid EMS memory block",
    "Expanded Memory Manager failure",
    "Out of soundcard memory",
    "Soundcard memory heap corrupted",
    "Invalid soundcard memory block",
    "Out of sample handles",
    "Unable to open file",
    "Unable to read file",
    "Invalid module file",
    "Invalid instrument in module",
    "Invalid pattern data in module",
    "Invalid channel number",
    "Invalid sample handle",
    "Sound Device channels not open",
    "Sound Device hardware failure",
    "Invalid function arguments",
    "File does not exist",
    "Invalid file handle",
    "Access denied",
    "File exists",
    "Too many open files",
    "Disk full",
    "Unexpected end of file",
    "Invalid path",
    "Unable to write file",
    "Unable to lock Virtual DMA buffer",
    "Unable to use Virtual DMA Services",
    "Invalid Virtual DMA Service version",
    "DPMI failure",
    "Invalid segment descriptor",
    "Out of system resources",
    "Invalid device used",
    "Unsupported function used" ,
    "Device is not available",
    "Device is busy",
    "Unsupported output mode used",
    "Unable to lock memory",
    "DirectSound Window Handle not set",
    "Invalid ADPCM data",
    "Invalid sample type",
    "No module is being played",
    "Out of sound channels",
    "Couldn't load dsound.dll",
    "Bad WAVE file",
    "Unsupported sample data format"
};




#ifdef DEBUG

errRecord   errorList[MAXERRORS];       /* error list */
unsigned    numErrors = 0;              /* number of errors in list */



/****************************************************************************\
*
* Function:     void errAdd(int errorCode, unsigned functID);
*
* Description:  Add an error to error list
*
* Input:        int errorCode           error code
*               unsigned functID        ID for function that caused the error
*
\****************************************************************************/

void CALLING errAdd(int errorCode, unsigned functID)
{
    /* make sure that error list does not overflow */
    if ( numErrors <= MAXERRORS )
    {
        /* store error information to list: */
        errorList[numErrors].errorCode = errorCode;
        errorList[numErrors].functID = functID;

        numErrors++;
    }
}




/****************************************************************************\
*
* Function:     void errClearList(void)
*
* Description:  Clears the error list. Can be called if a error has been
*               handled without exiting the program to avoid filling the
*               error list with handled errors.
*
\****************************************************************************/

void CALLING errClearList(void)
{
    numErrors = 0;
}




/****************************************************************************\
*
* Function:     void errPrintList(void);
*
* Description:  Prints the error list to stderr
*
\****************************************************************************/

void CALLING errPrintList(void)
{
    unsigned    i;

    if ( numErrors > 0 )
        fputs("MIDAS error list:\n", stderr);

    for ( i = 0; i < numErrors; i++ )
    {
        fprintf(stderr, "%u: <%i, %u> - %s at %u\n", i,
            errorList[i].errorCode, errorList[i].functID,
            errorMsg[errorList[i].errorCode], errorList[i].functID);
    }
}





#endif


#ifdef __WATCOMC__

void SetMode3(void);
#pragma aux SetMode3 = \
        "mov    ax,0003h" \
        "int    10h" \
        modify exact [ax];

#endif


/****************************************************************************\
*
* Function:     void errErrorExit(char *msg)
*
* Description:  Set up standard text mode, print an error message and exit
*
* Input:        char *msg               pointer to error message, ASCIIZ
*
\****************************************************************************/

void CALLING errErrorExit(char *msg)
{
    /* set up standard 80x25 text mode: */
#ifdef __BORLANDC__
asm     mov     ax,0003h
asm     int     10h
#else
#ifdef __WATCOMC__
//    SetMode3();
#else
#if (!defined(__UNIX__)) && (!defined(__WIN32__))
    vgaSetMode(3);
#endif
#endif
#endif

    fputs(msg, stderr);
    fputs("\n", stderr);

#ifdef DEBUG
    errPrintList();                     /* print error list */
#endif

    /* exit to DOS: */
    exit(EXIT_FAILURE);
}


/*
 * $Log: errors.c,v $
 * Revision 1.13  1997/07/31 10:56:40  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.12  1997/06/02 00:54:14  jpaana
 * Changed most __LINUX__ defines to __UNIX__ for generic *nix porting
 *
 * Revision 1.11  1997/05/20 20:35:54  pekangas
 * Added RIFF WAVE reader and two new error messages for it
 *
 * Revision 1.10  1997/05/03 17:53:43  pekangas
 * Added a new error "Couldn't load dsound.dll"
 *
 * Revision 1.9  1997/05/02 13:17:28  pekangas
 * Added a new error: Out of sound channels
 *
 * Revision 1.8  1997/03/05 16:50:40  pekangas
 * Added a new error: No module is being played
 *
 * Revision 1.7  1997/02/18 20:20:40  pekangas
 * Added a few error messages
 *
 * Revision 1.6  1997/02/15 14:00:44  pekangas
 * Added invalid ADPCM data -error
 *
 * Revision 1.5  1997/02/06 21:07:20  pekangas
 * Added new errors messages for DirectSound, plus ID_ds
 *
 * Revision 1.4  1996/07/13 20:33:43  pekangas
 * Fixed for Visual C
 *
 * Revision 1.3  1996/05/24 20:40:12  jpaana
 * Added USSWave
 *
 * Revision 1.2  1996/05/24 16:20:36  jpaana
 * Added Linux specific stuff
 *
 * Revision 1.1  1996/05/22 20:49:33  pekangas
 * Initial revision
 *
*/
