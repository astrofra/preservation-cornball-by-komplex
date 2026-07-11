/*      mix_mfi.c
 *
 * Digital Sound Mixer, mono floating point interpolating mixing routines
 *
 * $Id: mix_mfi.c,v 1.8.2.1 1997/09/06 17:16:01 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#include "sdevice.h"
#include "dsm.h"

RCSID(const char *mix_mfi_rcsid = "$Id: mix_mfi.c,v 1.8.2.1 1997/09/06 17:16:01 pekangas Exp $";)



/* Mix 8-bit mono samples: */
static void CALLING mix8Mono(unsigned numSamples, int nextSampleOffset)
{
    float *dest = (float*) dsmMixDest;
    uchar *sample = dsmMixSample;
    int  mixPos = dsmMixSrcPos;
    float s0, s1;
    float volMul = dsmMixLeftVolFloat * ((float) (1.0 / 65536.0));

    if ( nextSampleOffset != 1 )
    {
        while ( numSamples )
        {
            s0 = dsmByteFloatTable[sample[mixPos >> 16]];
            s1 = dsmByteFloatTable[sample[(mixPos >> 16) + nextSampleOffset]];
            *(dest++) += volMul * (((float) (65536 - (mixPos & 0xFFFF))) * s0 +
                                   ((float) (mixPos & 0xFFFF)) * s1);
            mixPos += dsmMixStep;
            numSamples--;
        }
    }
    else
    {	
        while ( numSamples )
        {
            s0 = dsmByteFloatTable[sample[mixPos >> 16]];
            s1 = dsmByteFloatTable[sample[(mixPos >> 16) + 1]];
            *(dest++) += volMul * (((float) (65536 - (mixPos & 0xFFFF))) * s0 +
                                   ((float) (mixPos & 0xFFFF)) * s1);
            mixPos += dsmMixStep;
            numSamples--;
        }
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}


/* Mix 16-bit mono samples: */
static void CALLING mix16Mono(unsigned numSamples, int nextSampleOffset)
{
    float *dest = (float*) dsmMixDest;
    S16 *sample = (S16*) dsmMixSample;
    int mixPos = dsmMixSrcPos;
    float s0, s1;
    float volMul = dsmMixLeftVolFloat * ((float) (1.0 / 65536.0));

    if ( nextSampleOffset != 1 )
    {
        while ( numSamples )
        {
            s0 = (float) sample[mixPos >> 16];
            s1 = (float) sample[(mixPos >> 16) + nextSampleOffset];
            *(dest++) += volMul * (((float) (65536 - (mixPos & 0xFFFF))) * s0 +
                                   ((float) (mixPos & 0xFFFF)) * s1);
            mixPos += dsmMixStep;
            numSamples--;
        }
    }
    else
    {
        while ( numSamples )
        {
            s0 = (float) sample[mixPos >> 16];
            s1 = (float) sample[(mixPos >> 16) + 1];
            *(dest++) += volMul * (((float) (65536 - (mixPos & 0xFFFF))) * s0 +
                                   ((float) (mixPos & 0xFFFF)) * s1);
            mixPos += dsmMixStep;
            numSamples--;
        }
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;    
}



/* Mix u-law mono samples: */
static void CALLING mixUlawMono(unsigned numSamples, int nextSampleOffset)
{
    float *dest = (float*) dsmMixDest;
    uchar *sample = dsmMixSample;
    int mixPos = dsmMixSrcPos;
    float s0, s1;
    float volMul = dsmMixLeftVolFloat * ((float) (1.0 / 65536.0));

    if ( nextSampleOffset != 1 )
    {
        while ( numSamples )
        {
            s0 = dsmUlawFloatTable[sample[mixPos >> 16]];
            s1 = dsmUlawFloatTable[sample[(mixPos >> 16) + nextSampleOffset]];
            *(dest++) += volMul * (((float) (65536 - (mixPos & 0xFFFF))) * s0 +
                                   ((float) (mixPos & 0xFFFF)) * s1);
            mixPos += dsmMixStep;
            numSamples--;
        }
    }
    else
    {
        while ( numSamples )
        {
            s0 = dsmUlawFloatTable[sample[mixPos >> 16]];
            s1 = dsmUlawFloatTable[sample[(mixPos >> 16) + 1]];
            *(dest++) += volMul * (((float) (65536 - (mixPos & 0xFFFF))) * s0 +
                                   ((float) (mixPos & 0xFFFF)) * s1);
            mixPos += dsmMixStep;
            numSamples--;
        }
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}




/* Mix 8-bit stereo samples: */
static void CALLING mix8Stereo(unsigned numSamples, int nextSampleOffset)
{
    float *dest = (float*) dsmMixDest;
    uchar *sample = dsmMixSample;
    int  mixPos = dsmMixSrcPos;
    float i0, i1;
    float volMul = dsmMixLeftVolFloat * ((float) (1.0 / 65536.0));

    if ( nextSampleOffset != 1 )
    {
        while ( numSamples )
        {
            i0 = (float) (65536 - (mixPos & 0xFFFF));
            i1 = (float) (mixPos & 0xFFFF);
            *(dest++) += volMul *
                ((i0 * dsmByteFloatTable[sample[2 * (mixPos >> 16)]] +
                  i1 * dsmByteFloatTable[sample[2 * (mixPos >> 16) +
                                               2 * nextSampleOffset]]) +
                 (i0 * dsmByteFloatTable[sample[2 * (mixPos >> 16) + 1]] +
                  i1 * dsmByteFloatTable[sample[2 * (mixPos >> 16) +
                                               2 * nextSampleOffset + 1]]));
            mixPos += dsmMixStep;
            numSamples--;
        }
    }
    else
    {
        while ( numSamples )
        {
            i0 = (float) (65536 - (mixPos & 0xFFFF));
            i1 = (float) (mixPos & 0xFFFF);
            *(dest++) += volMul *
                ((i0 * dsmByteFloatTable[sample[2 * (mixPos >> 16)]] +
                  i1 * dsmByteFloatTable[sample[2 * (mixPos >> 16) + 2]]) +
                 (i0 * dsmByteFloatTable[sample[2 * (mixPos >> 16) + 1]] +
                  i1 * dsmByteFloatTable[sample[2 * (mixPos >> 16) + 3]]));
            mixPos += dsmMixStep;
            numSamples--;
        }
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}


/* Mix 16-bit stereo samples: */
static void CALLING mix16Stereo(unsigned numSamples, int nextSampleOffset)
{
    float *dest = (float*) dsmMixDest;
    S16 *sample = (S16*) dsmMixSample;
    int  mixPos = dsmMixSrcPos;
    float i0, i1;
    float volMul = dsmMixLeftVolFloat * ((float) (1.0 / 65536.0));

    if ( nextSampleOffset != 1 )
    {
        while ( numSamples )
        {
            i0 = (float) (65536 - (mixPos & 0xFFFF));
            i1 = (float) (mixPos & 0xFFFF);
            *(dest++) += volMul *
                ((i0 * ((float) sample[2 * (mixPos >> 16)]) +
                  i1 * ((float) sample[2 * (mixPos >> 16) +
                                      2 * nextSampleOffset])) +
                 (i0 * ((float) sample[2 * (mixPos >> 16) + 1]) +
                  i1 * ((float) sample[2 * (mixPos >> 16) +
                                      2 * nextSampleOffset + 1])));
            mixPos += dsmMixStep;
            numSamples--;
        }
    }
    else
    {
        while ( numSamples )
        {
            i0 = (float) (65536 - (mixPos & 0xFFFF));
            i1 = (float) (mixPos & 0xFFFF);
            *(dest++) += volMul *
                ((i0 * ((float) sample[2 * (mixPos >> 16)]) +
                  i1 * ((float) sample[2 * (mixPos >> 16) + 2])) +
                 (i0 * ((float) sample[2 * (mixPos >> 16) + 1]) +
                  i1 * ((float) sample[2 * (mixPos >> 16) + 3])));
            mixPos += dsmMixStep;
            numSamples--;
        }
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}


/* Mix u-law stereo samples: */
static void CALLING mixUlawStereo(unsigned numSamples, int nextSampleOffset)
{
    float *dest = (float*) dsmMixDest;
    uchar *sample = dsmMixSample;
    int  mixPos = dsmMixSrcPos;
    float i0, i1;
    float volMul = dsmMixLeftVolFloat * ((float) (1.0 / 65536.0));

    if ( nextSampleOffset != 1 )
    {
        while ( numSamples )
        {
            i0 = (float) (65536 - (mixPos & 0xFFFF));
            i1 = (float) (mixPos & 0xFFFF);
            *(dest++) += volMul *
                ((i0 * dsmUlawFloatTable[sample[2 * (mixPos >> 16)]] +
                  i1 * dsmUlawFloatTable[sample[2 * (mixPos >> 16) +
                                               2 * nextSampleOffset]]) +
                 (i0 * dsmUlawFloatTable[sample[2 * (mixPos >> 16) + 1]] +
                  i1 * dsmUlawFloatTable[sample[2 * (mixPos >> 16) +
                                               2 * nextSampleOffset + 1]]));
            mixPos += dsmMixStep;
            numSamples--;
        }
    }
    else
    {        
        while ( numSamples )
        {
            i0 = (float) (65536 - (mixPos & 0xFFFF));
            i1 = (float) (mixPos & 0xFFFF);
            *(dest++) += volMul *
                ((i0 * dsmUlawFloatTable[sample[2 * (mixPos >> 16)]] +
                  i1 * dsmUlawFloatTable[sample[2 * (mixPos >> 16) + 2]]) +
                 (i0 * dsmUlawFloatTable[sample[2 * (mixPos >> 16) + 1]] +
                  i1 * dsmUlawFloatTable[sample[2 * (mixPos >> 16) + 3]]));
            mixPos += dsmMixStep;
            numSamples--;
        }
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}


#ifdef M_X86_ASSEMBLER
extern void CALLING dsmMixMono8MonoAsmFI(unsigned numSamples,
                                         int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixMono8MonoAsmFIReloc;

extern void CALLING dsmMixMono16MonoAsmFI(unsigned numSamples,
                                          int nextSampleOffset);

extern void CALLING dsmMixMonoUlawMonoAsmFI(unsigned numSamples,
                                            int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixMonoUlawMonoAsmFIReloc;
#endif

dsmMixRoutineSet dsmMixMonoFloatInterp =
{
    {
        { 0, 0, NULL, NULL, NULL },                         /* no sample */
#ifdef M_X86_ASSEMBLER        
        { 1, 2, &mix8Mono, &dsmMixMono8MonoAsmFI,
          &dsmMixMono8MonoAsmFIReloc },                     /* 8-bit mono */
        { 1, 2, &mix16Mono, &dsmMixMono16MonoAsmFI, NULL }, /* 16-bit mono */
#else
        { 1, 1, &mix8Mono, &mix8Mono, NULL },               /* 8-bit mono */
        { 1, 1, &mix16Mono, &mix16Mono, NULL },             /* 16-bit mono */
#endif        
        { 1, 1, &mix8Stereo, &mix8Stereo, NULL },           /* 8-bit stereo */
        { 1, 1, &mix16Stereo, &mix16Stereo, NULL },         /* 16-bit stereo */
        { 0, 0, NULL, NULL, NULL },                         /* ADPCM mono */
        { 0, 0, NULL, NULL, NULL },                         /* ADPCM stereo */
#ifdef M_X86_ASSEMBLER
        { 1, 2, &mixUlawMono, &dsmMixMonoUlawMonoAsmFI,
          &dsmMixMonoUlawMonoAsmFIReloc },                  /* u-law mono */
#else
        { 1, 1, &mixUlawMono, &mixUlawMono, NULL },         /* u-law mono */
#endif
        { 1, 1, &mixUlawStereo, &mixUlawStereo, NULL }      /* u-law stereo */
    },

    /* No special middle or surround routines: */
    {
        { 0, 0, NULL, NULL, NULL },
        { 0, 0, NULL, NULL, NULL }, { 0, 0, NULL, NULL, NULL },
        { 0, 0, NULL, NULL, NULL }, { 0, 0, NULL, NULL, NULL },
        { 0, 0, NULL, NULL, NULL }, { 0, 0, NULL, NULL, NULL },
        { 0, 0, NULL, NULL, NULL }, { 0, 0, NULL, NULL, NULL }
    },
    {
        { 0, 0, NULL, NULL, NULL },
        { 0, 0, NULL, NULL, NULL }, { 0, 0, NULL, NULL, NULL },
        { 0, 0, NULL, NULL, NULL }, { 0, 0, NULL, NULL, NULL },
        { 0, 0, NULL, NULL, NULL }, { 0, 0, NULL, NULL, NULL },
        { 0, 0, NULL, NULL, NULL }, { 0, 0, NULL, NULL, NULL }
    }    
};


/*
 * $Log: mix_mfi.c,v $
 * Revision 1.8.2.1  1997/09/06 17:16:01  pekangas
 * Optimized the floating point mixing routines for Pentium Pro / Pentium II
 *
 * Revision 1.8  1997/07/31 10:56:51  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.7  1997/07/30 19:04:03  pekangas
 * Added optimized mono u-law mixing routines and enabled u-law autoconversion
 * by default
 *
 * Revision 1.6  1997/07/24 17:10:01  pekangas
 * Added surround support and support for separate surround and middle mixing
 * routines.
 *
 * Revision 1.5  1997/07/17 09:56:41  pekangas
 * dsmmix.c now takes care of halving volume for mixing stereo samples
 * to mono output
 *
 * Revision 1.4  1997/07/03 15:03:46  pekangas
 * Interpolating mixing no longer indexes past the real sample or loop end,
 * thus replicating data for it is not necessary
 *
 * Revision 1.3  1997/06/26 14:33:21  pekangas
 * Added proper interpolation for stereo samples
 * Added assembler versions of some of the routines
 *
 * Revision 1.2  1997/06/20 10:08:54  pekangas
 * Fixed to compile with Visual C
 *
 * Revision 1.1  1997/06/05 20:18:24  pekangas
 * Initial revision
 *
*/
