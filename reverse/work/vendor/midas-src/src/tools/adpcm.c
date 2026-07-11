/*      adpcm.c
 *
 * A simple ADPCM encoder/decoder
 *
 * $Id: adpcm.c,v 1.4 1997/07/31 10:56:59 pekangas Exp $
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
#include "madpcm.h"


#define ADPCMBUFFER 65536


FILE            *inf = NULL, *outf = NULL;
unsigned        numChannels;
ulong           inFileSize;
int             encode;
uchar           *adpcm;
short           *samples;
mAdpcmState     *state;
unsigned        frameLength;


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
    static unsigned inputUsed, outputWritten;
    unsigned    inputLeft;
    short       *in;

    while ( inFileSize )
    {
        /* Check how much we should code this time: */
        if ( inFileSize > (4*ADPCMBUFFER) )
            doNow = 4*ADPCMBUFFER;
        else
            doNow = inFileSize;
        inFileSize -= doNow;

        /* Read the data: */
        fread(samples, doNow, 1, inf);

        /* Code it: */
        inputLeft = doNow;
        in = samples;
        while ( inputLeft )
        {
            if ( (error = mAdpcmEncode(in, inputLeft, adpcm, ADPCMBUFFER,
                &inputUsed, &outputWritten, state)) != OK )
                Error(errorMsg[error]);
            inputLeft -= inputUsed;
            in += (inputUsed / 2);
            fwrite(adpcm, outputWritten, 1, outf);
        }
    }
}


void DecodeFile(void)
{
    unsigned    doNow;
    int         error;
    static unsigned inputUsed, outputWritten;
    unsigned    inputLeft;
    uchar       *in;

    while ( inFileSize )
    {
        /* Check how much we should code this time: */
        if ( inFileSize > ADPCMBUFFER )
            doNow = ADPCMBUFFER;
        else
            doNow = inFileSize;
        inFileSize -= doNow;

        /* Read the data: */
        fread(adpcm, doNow, 1, inf);

        /* Decode it: */
        inputLeft = doNow;
        in = adpcm;
        while ( inputLeft )
        {
            if ( (error = mAdpcmDecode(in, inputLeft, samples, 4*ADPCMBUFFER,
                &inputUsed, &outputWritten, state)) != OK )
                Error(errorMsg[error]);
            inputLeft -= inputUsed;
            in += inputUsed;
            fwrite(samples, outputWritten, 1, outf);
        }
    }
}


int main(int argc, char *argv[])
{
    int         error;

    puts("adpcm 1.1.0 - MIDAS ADPCM encoder/decoder");

    if ( (argc != 6) || ((argv[1][0] != 'e') && (argv[1][0] != 'd')) )
    {
        puts("\nUsage:  adpcm   command input output channels framelength\n\n"
             "        command     e = encode, d = decode\n"
             "        input       input file name\n"
             "        output      output file name\n"
             "        channels    number of channels (1 = mono, 2 = stereo "
             "etc)\n"
             "        framelength output frame length in bytes, including "
             "frame header");
        return 1;
    }

    if ( argv[1][0] == 'e' )
        encode = 1;
    else
        encode = 0;

    if ( (numChannels = atoi(argv[4])) <= 0 )
        Error("Invalid number of channels");

    frameLength = atoi(argv[5]);

    /* Kluge: */
    if ( (frameLength < (6 + 3*numChannels)) || (frameLength > 65535) )
        Error("Illegal frame length");

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

    if ( encode && ((inFileSize % (2 * numChannels)) != 0) )
        puts("Warning: Input file size does not contain complete data for all"
             " channels");

    /* Allocate buffers: */
    if ( (adpcm = malloc(ADPCMBUFFER)) == NULL )
        Error("Out of memory");
    if ( (samples = malloc(4 * ADPCMBUFFER)) == NULL )
        Error("Out of memory");

    /* Initialize ADPCM encoder/decoder state: */
    if ( (error = mAdpcmInitState(numChannels, frameLength, &state)) != OK )
        Error(errorMsg[error]);

    if ( encode )
        EncodeFile();
    else
        DecodeFile();

    /* Deallocate buffers and ADPCM state structure: */
    free(samples);
    free(adpcm);
    if ( (error = mAdpcmFreeState(state)) != OK )
        Error(errorMsg[error]);

    fclose(inf);
    inf = NULL;
    fclose(outf);
    outf = NULL;

    return 0;
}
