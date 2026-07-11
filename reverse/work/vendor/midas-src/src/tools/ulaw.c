/*      ulaw.c
 *
 * A simple u-law encoder/decoder
 *
 * $Id: ulaw.c,v 1.2 1997/07/31 10:57:00 pekangas Exp $
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
#include "midas.h"
#include "mulaw.h"


#define ULAWBUFFER 65536


FILE            *inf = NULL, *outf = NULL;
ulong           inFileSize;
int             encode;
uchar           *ulaw;
short           *samples;


void Error(char *msg)
{
    printf("Error: %s\n", msg);
    if ( inf != NULL )
        fclose(inf);
    if ( outf != NULL )
        fclose(outf);
    exit(EXIT_FAILURE);
}


void EncodeFile(void)
{
    unsigned    doNow;
    int         error;

    while ( inFileSize )
    {
        /* Check how much we should code this time: */
        if ( inFileSize > (2*ULAWBUFFER) )
            doNow = 2*ULAWBUFFER;
        else
            doNow = inFileSize;
        inFileSize -= doNow;

        /* Read the data: */
        fread(samples, doNow, 1, inf);

        /* Code it: */
        if ( (error = mUlawEncode(samples, doNow, ulaw)) != OK )
            Error(errorMsg[error]);

        /* Write back: */
        fwrite(ulaw, doNow / 2, 1, outf);
    }
}


void DecodeFile(void)
{
    unsigned    doNow;
    int         error;

    while ( inFileSize )
    {
        /* Check how much we should code this time: */
        if ( inFileSize > ULAWBUFFER )
            doNow = ULAWBUFFER;
        else
            doNow = inFileSize;
        inFileSize -= doNow;

        /* Read the data: */
        fread(ulaw, doNow, 1, inf);

        /* Decode it: */
        if ( (error = mUlawDecode(ulaw, doNow, samples)) != OK )
            Error(errorMsg[error]);

        /* And write back: */
        fwrite(samples, 2 * doNow, 1, outf);
    }
}


int main(int argc, char *argv[])
{
    puts("ulaw 1.0.0 - MIDAS u-law encoder/decoder");

    if ( (argc != 4) || ((argv[1][0] != 'e') && (argv[1][0] != 'd')) )
    {
        puts("\nUsage:  ulaw    command input output\n\n"
             "        command e = encode, d = decode\n"
             "        input   input file name\n"
             "        output  output file name");
        return 1;
    }

    if ( argv[1][0] == 'e' )
        encode = 1;
    else
        encode = 0;

    /* Open the files: */
    if ( (inf = fopen(argv[2], "rb")) == NULL )
        Error("Unable to open input file");
    if ( (outf = fopen(argv[3], "wb")) == NULL )
        Error("Unable to open output file");

    /* Get input file size: */
    fseek(inf, 0, SEEK_END);
    inFileSize = (ulong) ftell(inf);
    fseek(inf, 0, SEEK_SET);

    if ( inFileSize == 0 )
        puts("Warning: Empty input file");

    if ( encode && (inFileSize & 1) )
        Error("Input file contains a fractional number of samples");

    /* Allocate buffers: */
    if ( (ulaw = malloc(ULAWBUFFER)) == NULL )
        Error("Out of memory");
    if ( (samples = malloc(2 * ULAWBUFFER)) == NULL )
        Error("Out of memory");

    if ( encode )
        EncodeFile();
    else
        DecodeFile();

    /* Deallocate them: */
    free(samples);
    free(ulaw);

    fclose(inf);
    inf = NULL;
    fclose(outf);
    outf = NULL;

    return 0;
}


/*
 * $Log: ulaw.c,v $
 * Revision 1.2  1997/07/31 10:57:00  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.1  1997/02/20 19:47:38  pekangas
 * Added u-law decoder and updated for u-law
 *
*/