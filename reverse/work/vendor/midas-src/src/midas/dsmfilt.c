/*      dsmfilt.c
 *
 * Digital Sound Mixer, output filters
 *
 * $Id: dsmfilt.c,v 1.6.2.2 1997/09/08 16:54:17 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#include "mglobals.h"
#include "sdevice.h"
#include "mprof.h"
#include "dsm.h"

RCSID(const char *dsmfilt_rcsid = "$Id: dsmfilt.c,v 1.6.2.2 1997/09/08 16:54:17 pekangas Exp $";)


#ifdef M_X86_ASSEMBLER
extern void CALLING dsmFilterFloatMono1(void *data, unsigned numSamples);
#endif

void CALLING dsmFilterFloatMono(void *data, unsigned numSamples, void *dummy)
{
    static float d1l = 0.0f, d2l = 0.0f;
    float s;
    float *p = (float*) data;
    int filt;

    M_PROF_START(DSM_PROF_FILTER_FLOAT_MONO, numSamples);

    dummy = dummy;

    if ( mOutputFilterMode == 3 )
        filt = dsmAutoFilterType;
    else
        filt = mOutputFilterMode;
    
    switch ( filt )
    {
        case 1:
#ifdef M_X86_ASSEMBLER
            dsmFilterFloatMono1(data, numSamples);
#else
            while ( numSamples )
            {
                s = (d1l + 2.0f * (*p)) * (1.0f/3.0f);
                d1l = *p;
                *(p++) = s;
                numSamples--;
            }
#endif            
            break;

        case 2:
            while ( numSamples )
            {
                s = (d1l + 3.0f * d2l + 4.0f * (*p)) * (1.0f/8.0f);
                d1l = d2l; d2l = *p;
                *(p++) = s;
                numSamples--;
            }
            break;
    }

    M_PROF_END(DSM_PROF_FILTER_FLOAT_MONO);
}


#ifdef M_X86_ASSEMBLER
extern void CALLING dsmFilterFloatStereo1(void *data, unsigned numSamples);
#endif
    
void CALLING dsmFilterFloatStereo(void *data, unsigned numSamples, void *dummy)
{
    static float d1l = 0.0f, d1r = 0.0f, d2l = 0.0f, d2r = 0.0f;
    float s;
    float *p = (float*) data;
    int filt;

    M_PROF_START(DSM_PROF_FILTER_FLOAT_STEREO, numSamples);

    dummy = dummy;

    if ( mOutputFilterMode == 3 )
        filt = dsmAutoFilterType;
    else
        filt = mOutputFilterMode;
    
    switch ( filt )
    {
        case 1:
#ifdef M_X86_ASSEMBLER
            dsmFilterFloatStereo1(data, numSamples);
#else
            while ( numSamples )
            {
                s = (d1l + 2.0f * (*p)) * (1.0f/3.0f);
                d1l = *p;
                *(p++) = s;
                s = (d1r + 2.0f * (*p)) * (1.0f/3.0f);
                d1r = *p;
                *(p++) = s;
                numSamples--;
            }
#endif            
            break;

        case 2:
            while ( numSamples )
            {
                s = (d1l + 3.0f*d2l + 4.0f*(*p)) * (1.0f/8.0f);
                d1l = d2l; d2l = *p;
                *(p++) = s;
                s = (d1r + 3.0f*d2r + 4.0f*(*p)) * (1.0f/8.0f);
                d1r = d2r; d2r = *p;
                *(p++) = s;
                numSamples--;
            }
            break;
    }

    M_PROF_END(DSM_PROF_FILTER_FLOAT_STEREO);
}


#ifdef M_X86_ASSEMBLER
extern void CALLING dsmFilterIntMono1(void *data, unsigned numSamples);
#endif

void CALLING dsmFilterIntMono(void *data, unsigned numSamples, void *dummy)
{
    static int d1l = 0, d2l = 0;
    int s;
    int *p = (int*) data;
    int filt;

    M_PROF_START(DSM_PROF_FILTER_INT_MONO, numSamples);

    dummy = dummy;

    if ( mOutputFilterMode == 3 )
        filt = dsmAutoFilterType;
    else
        filt = mOutputFilterMode;
    
    switch ( filt )
    {
        case 1:
#ifdef M_X86_ASSEMBLER
            dsmFilterIntMono1(data, numSamples);
#else
            while ( numSamples )
            {
                /*s = (d1 + 2 * (*p)) */
                s = d1l + ((*p) << 1);
                s = (s >> 2) + (s >> 3) - (s >> 5);
                d1l = *p;
                *(p++) = s;
                /* ok, not quite it, but close enough */
                    
                numSamples--;
            }
#endif            
            break;

        case 2:
            while ( numSamples )
            {               
                /*s = (d1l + 3.0f*d2l + 4.0f*(*p)) * (1.0f/8.0f);*/
                s = (d1l + d2l + (d2l << 1) + ((*p) << 2)) >> 3;
                d1l = d2l; d2l = *p;
                *(p++) = s;
                numSamples--;
            }
            break;
    }

    M_PROF_END(DSM_PROF_FILTER_INT_MONO);
}


#ifdef M_X86_ASSEMBLER
extern void CALLING dsmFilterIntStereo1(void *data, unsigned numSamples);
#endif


void CALLING dsmFilterIntStereo(void *data, unsigned numSamples, void *dummy)
{
    static int d1l = 0, d1r = 0, d2l = 0, d2r = 0;
    int s;
    int *p = (int*) data;
    int filt;

    M_PROF_START(DSM_PROF_FILTER_INT_STEREO, numSamples);

    dummy = dummy;

    if ( mOutputFilterMode == 3 )
        filt = dsmAutoFilterType;
    else
        filt = mOutputFilterMode;
    
    switch ( filt )
    {
        case 1:
#ifdef M_X86_ASSEMBLER
            dsmFilterIntStereo1(data, numSamples);
#else
            while ( numSamples )
            {
                /*s = (d1 + 2 * (*p)) */
                s = d1l + ((*p) << 1);
                s = (s >> 2) + (s >> 3) - (s >> 5);
                d1l = *p;
                *(p++) = s;
                /* ok, not quite it, but close enough */

                s = d1r + ((*p) << 1);
                s = (s >> 2) + (s >> 3) - (s >> 5);
                d1r = *p;
                *(p++) = s;
                    
                numSamples--;
            }
#endif            
            break;

        case 2:
            while ( numSamples )
            {               
                /*s = (d1l + 3.0f*d2l + 4.0f*(*p)) * (1.0f/8.0f);*/
                s = (d1l + d2l + (d2l << 1) + ((*p) << 2)) >> 3;
                d1l = d2l; d2l = *p;
                *(p++) = s;
                s = (d1r + d2r + (d2r << 1) + ((*p) << 2)) >> 3;
                d1r = d2r; d2r = *p;
                *(p++) = s;
                numSamples--;
            }
            break;
    }
    
    M_PROF_END(DSM_PROF_FILTER_INT_STEREO);
}





/*
 * $Log: dsmfilt.c,v $
 * Revision 1.6.2.2  1997/09/08 16:54:17  pekangas
 * Added hand-optimized versions of floating point filter 1 functions
 *
 * Revision 1.6.2.1  1997/08/20 12:48:04  pekangas
 * Manually merged fixes from the main trunk (oops)
 *
 * Revision 1.7  1997/08/19 17:22:02  pekangas
 * Added assembler versions of the default filtering routines
 *
 * Revision 1.6  1997/07/31 10:56:39  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.5  1997/07/25 13:47:55  pekangas
 * Added automatic filtering and made it the default
 *
 * Revision 1.4  1997/07/25 09:58:09  pekangas
 * Added integer filters
 *
 * Revision 1.3  1997/07/10 18:40:22  pekangas
 * Added echo effect support
 *
 * Revision 1.2  1997/06/20 10:08:53  pekangas
 * Fixed to compile with Visual C
 *
 * Revision 1.1  1997/06/11 14:35:53  pekangas
 * Initial revision
 *
*/
