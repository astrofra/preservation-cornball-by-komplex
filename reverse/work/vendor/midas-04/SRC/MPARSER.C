/*      MPARSER.C
 *
 * MIDAS Sound System command line parser. Meant to be used with the
 * simplified MIDAS API, MIDAS.C
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/


#include "midas.h"



/****************************************************************************\
*
* Function:     void mParserError(char *msg)
*
* Description:  Prints a parser error message to stderr and exits to DOS.
*
* Input:        char *msg               error message
*
\****************************************************************************/

void CALLING mParserError(char *msg)
{
    char        errmsg[70];

    mStrCopy(&errmsg[0], "MIDAS parser error: ");
    mStrAppend(&errmsg[0], msg);
    errErrorExit(&errmsg[0]);           /* print error message */
}




/****************************************************************************\
*
* Function:     void midasParseOption(char *option)
*
* Description:  Parses one MIDAS command line option.
*
* Input:        char *option            Command line option string WITHOUT
*                                       the leading '-' or '/'.
*
* Recognized options:
*       -sx     Force Sound Device x (1 = GUS, 2 = PAS, 3 = WSS, 4 = SB,
*               5 = No Sound)
*       -pxxx   Force I/O port xxx (hex) for Sound Device
*       -ix     Force IRQ x for Sound Device
*       -dx     Force DMA channel x for Sound Device
*       -cx     Force sound card type x for Sound Device
*       -mxxxx  Set mixing rate to xxxx Hz
*       -oxxx   Force output mode (8 = 8-bit, 1 = 16-bit, s = stereo,
*               m = mono)
*       -e      Disable EMS usage
*       -t      Disable Protracker BPM tempos
*       -g      Disable Protracker panning commands
*       -x      Enable extended octaves
*       -u      Enable Surround sound
*       -b      Disable Virtual DMA usage
*       -ax     Force amplification level x
*
\****************************************************************************/

void CALLING midasParseOption(char *option)
{
    int         c;
    char        *opt;

    opt = &option[1];
    switch ( option[0] )
    {
        /* -sx     Force Sound Device x */
        case 's':
            midasSDNumber = mDec2Long(opt) - 1;
            if ( (midasSDNumber >= NUMSDEVICES) || (midasSDNumber < 0) )
                mParserError("Illegal Sound Device number");
            break;

        /* -pxxx   Force I/O port xxx (hex) for Sound Device */
        case 'p':
            midasSDPort = mHex2Long(opt);
            if ( midasSDPort == -1 )
                mParserError("Illegal Sound Device port");
            break;

        /* -ix     Force IRQ x for Sound Device */
        case 'i':
            midasSDIRQ = mDec2Long(opt);
            if ( midasSDIRQ == -1 )
                mParserError("Illegal Sound Device IRQ");
            break;

        /* -dx     Force DMA channel x for Sound Device */
        case 'd':
            midasSDDMA = mDec2Long(opt);
            if ( midasSDDMA == -1 )
                mParserError("Illegal Sound Device DMA");
            break;

        /* -cx     Force sound card type x for Sound Device */
        case 'c':
            midasSDCard = mDec2Long(opt);
            if ( midasSDCard == -1 )
                mParserError("Illegal sound card number");
            break;

        /* -mxxxx  Set mixing rate to xxxx Hz */
        case 'm':
            midasMixRate = mDec2Long(opt);
            if ( midasMixRate < 1 )
                mParserError("Invalid mixing rate");
            break;

        /* -e      Disable EMS usage */
        case 'e':
            midasDisableEMS = 1;
            break;

        /* -t      Disable ProTracker BPM tempos */
        case 't':
            ptTempo = 0;
            break;

        /* -u      Enable Surround sound */
        case 'u':
            surround = 1;
            break;

        /* -g      Disable Protracker panning commands */
        case 'g':
            usePanning = 0;
            break;

        /* -x      Enable extended octaves */
        case 'x':
            extendedOctaves = 17;
            break;

        /* -b      Disable Virtual DMA usage */
        case 'b':
            useVDS = 0;
            break;

        /* -oxxx   Force output mode */
        case 'o':
            for ( c = 0; c < mStrLength(opt); c++ )
            {
                switch( opt[c] )
                {
                    /* Output mode '8' - 8-bit */
                    case '8':
                        midasOutputMode |= sd8bit;
                        midasOutputMode &= 0xFFFF ^ sd16bit;
                        break;

                    /* Output mode '1' - 16-bit */
                    case '1':
                        midasOutputMode |= sd16bit;
                        midasOutputMode &= 0xFFFF ^ sd8bit;
                        break;

                    /* Output mode 'm' - mono */
                    case 'm':
                        midasOutputMode |= sdMono;
                        midasOutputMode &= 0xFFFF ^ sdStereo;
                        break;

                    /* Output mode 's' - stereo */
                    case 's':
                        midasOutputMode |= sdStereo;
                        midasOutputMode &= 0xFFFF ^ sdMono;
                        break;

                    default:
                        mParserError("Invalid output mode character");
                        break;
                }
            }
            break;

        /* -ax     Force amplification level x */
        case 'a':
            midasAmplification = mDec2Long(opt);
            if ( midasAmplification < 1 )
                mParserError("Invalid amplification level");
            break;

        default:
            mParserError("Unknown option character");
            break;
    }
}




/****************************************************************************\
*
* Function:     void midasParseEnvironment(void)
*
* Description:  Parses the MIDAS environment string, which has same format
*               as the command line options.
*
\****************************************************************************/

void CALLING midasParseEnvironment(void)
{
    char        *envs, *midasenv, *opt;
    int         spos, slen, stopparse, error;

    /* try to get pointer to MIDAS environment string: */
    envs = mGetEnv("MIDAS");

    if ( envs != NULL )
    {
        slen = mStrLength(envs);
        /* allocate memory for a copy of the environment string: */
        if ( (error = memAlloc(slen+1, (void**) &midasenv)) != OK )
            midasError(error);

        /* copy environment string to midasenv: */
        mStrCopy(midasenv, envs);

        spos = 0;                       /* search position = 0 */
        opt = NULL;                     /* current option string = NULL */
        stopparse = 0;

        /* parse the whole environment string: */
        while ( !stopparse )
        {
            switch ( midasenv[spos] )
            {
                case ' ':
                    /* current character is space - change it to '\0' and
                       parse this option string if it exists*/
                    midasenv[spos] = 0;
                    if ( opt != NULL )
                        midasParseOption(opt);

                    opt = NULL;         /* no option string */
                    spos++;             /* next character */
                    break;

                case 0:
                    /* Current character is '\0' - end. Parse option string
                       if it exists and stop parsing. */
                    if ( (opt != NULL) && (*opt != 0) )
                        midasParseOption(opt);
                    stopparse = 1;
                    break;

                case '-':
                case '/':
                    /* Current character is '-' or '/' - option string starts
                       from next character */
                    spos++;
                    opt = &midasenv[spos];
                    break;

                default:
                    /* some normal character - continue parsing from next
                       character */
                    spos++;
            }
        }

        if ( (error = memFree(midasenv)) != OK )
            midasError(error);
    }
}
