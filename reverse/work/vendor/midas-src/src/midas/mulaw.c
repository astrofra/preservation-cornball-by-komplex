/*      mulaw.c
 *
 * u-law encoding/decoding routines
 *
 * $Id: mulaw.c,v 1.2 1997/07/31 10:56:53 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

/* Adapted from two simple conversion routines by
   Craig Reese (IDA/Supercomputing Research Center) and
   Joe Campbell (Department of Defense) */

#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#include "mulaw.h"


/* Exponent look-up table: */
static signed char expTable[256] =
{
    0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};

/* Decode exponent table: */
static int      decExpTable[8] =
    { 0,132,396,924,1980,4092,8316,16764 };


#define BIAS 0x84
#define CLIP 32635


/****************************************************************************\
*
* Function:     int mUlawEncode(short *input, unsigned inputLength,
*                   uchar *output)
*
* Description:  Encodes raw 16-bit signed sample data to 8-bit u-law.
*
* Input:        short *input            pointer to input data
*               unsigned inputLenth     length of input data, in bytes
*               uchar *output           pointer to output buffer, length
*                                       inputLength/2.
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mUlawEncode(short *input, unsigned inputLength, uchar *output)
{
    int         sign, exponent, mantissa;
    int         sample;
    unsigned char ulawbyte;

    /* Convert it all: */
    while ( inputLength )
    {
        /* Get sample: */
        sample = *(input++);
        inputLength -= 2;

        /* Convert sample into a sign & magnitude -pair and clip: */
        sign = (sample >> 8) & 0x80;    /* sign to bit 7 */
        if ( sign != 0 )
            sample = -sample;
        if ( sample > CLIP )
            sample = CLIP;

        /* Convert to u-law: */
        sample = sample + BIAS;
        exponent = expTable[(sample >> 7) & 0xFF];
        mantissa = (sample >> (exponent + 3)) & 0x0F;
        ulawbyte = ~(sign | (exponent << 4) | mantissa);

        /* Write output: */
        *(output++) = ulawbyte;
    }

    return OK;
}




/****************************************************************************\
*
* Function:     int mUlawDecode(uchar *input, unsigned inputLength,
*                   short *output)
*
* Description:  Decodes 8-bit u-law data to 16-bit signed raw samples
*
* Input:        uchar *input            pointer to input data
*               unsigned inputLength    amount of input data in bytes
*               short *output           pointer to output buffer, size
*                                       2*inputLength
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mUlawDecode(uchar *input, unsigned inputLength, short *output)
{
    int         sign, exponent, mantissa, sample;
    uchar       ulaw;

    while ( inputLength )
    {
        /* Get input: */
        ulaw = *(input++);
        inputLength--;

        /* Convert it: */
        ulaw = ~ulaw;
        sign = (ulaw & 0x80);
        exponent = (ulaw >> 4) & 0x07;
        mantissa = ulaw & 0x0F;
        sample = decExpTable[exponent] + (mantissa << (exponent + 3));
        if ( sign != 0 )
            sample = -sample;

        /* And write output: */
        *(output++) = (short) sample;
    }

    return OK;
}


/*
 * $Log: mulaw.c,v $
 * Revision 1.2  1997/07/31 10:56:53  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.1  1997/02/20 19:48:22  pekangas
 * Initial revision
 *
*/