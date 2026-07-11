/*      mix_sfi.c
 *
 * Digital Sound Mixer, stereo floating point interpolating mixing routines
 *
 * $Id: mix_sfi.c,v 1.9.2.1 1997/09/06 17:16:02 pekangas Exp $
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

RCSID(const char *mix_sfi_rcsid = "$Id: mix_sfi.c,v 1.9.2.1 1997/09/06 17:16:02 pekangas Exp $";)



/* Mix 8-bit mono samples: */
static void CALLING mix8Mono(unsigned numSamples, int nextSampleOffset)
{
    float *dest = (float*) dsmMixDest;
    uchar *sample = dsmMixSample;
    int  mixPos = dsmMixSrcPos;
    float s0, s1, s;
    float volMulL = dsmMixLeftVolFloat * (1.0f / 65536.0f);
    float volMulR = dsmMixRightVolFloat * (1.0f / 65536.0f);

    if ( nextSampleOffset != 1 )
    {
        while ( numSamples )
        {
            s0 = dsmByteFloatTable[sample[mixPos >> 16]];
            s1 = dsmByteFloatTable[sample[(mixPos >> 16) + nextSampleOffset]];
            s = (((float) (65536 - (mixPos & 0xFFFF))) * s0 +
                 ((float) (mixPos & 0xFFFF)) * s1);
            *dest += volMulL * s;
            *(dest+1) += volMulR * s;
            dest += 2;
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
            s = (((float) (65536 - (mixPos & 0xFFFF))) * s0 +
                 ((float) (mixPos & 0xFFFF)) * s1);
            *dest += volMulL * s;
            *(dest+1) += volMulR * s;
            dest += 2;
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
    float s0, s1, s;
    float volMulL = dsmMixLeftVolFloat * (1.0f / 65536.0f);
    float volMulR = dsmMixRightVolFloat * (1.0f / 65536.0f);

    if ( nextSampleOffset != 1 )
    {
        while ( numSamples )
        {
            s0 = (float) sample[mixPos >> 16];
            s1 = (float) sample[(mixPos >> 16) + nextSampleOffset];
            s = (((float) (65536 - (mixPos & 0xFFFF))) * s0 +
                 ((float) (mixPos & 0xFFFF)) * s1);
            *dest += volMulL * s;
            *(dest+1) += volMulR * s;
            dest += 2;
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
            s = (((float) (65536 - (mixPos & 0xFFFF))) * s0 +
                 ((float) (mixPos & 0xFFFF)) * s1);
            *dest += volMulL * s;
            *(dest+1) += volMulR * s;
            dest += 2;
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
    float s0, s1, s;
    float volMulL = dsmMixLeftVolFloat * (1.0f / 65536.0f);
    float volMulR = dsmMixRightVolFloat * (1.0f / 65536.0f);

    if ( nextSampleOffset != 1 )
    {
        while ( numSamples )
        {
            s0 = dsmUlawFloatTable[sample[mixPos >> 16]];
            s1 = dsmUlawFloatTable[sample[(mixPos >> 16) + nextSampleOffset]];
            s = (((float) (65536 - (mixPos & 0xFFFF))) * s0 +
                 ((float) (mixPos & 0xFFFF)) * s1);
            *dest += volMulL * s;
            *(dest+1) += volMulR * s;
            dest += 2;
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
            s = (((float) (65536 - (mixPos & 0xFFFF))) * s0 +
                 ((float) (mixPos & 0xFFFF)) * s1);
            *dest += volMulL * s;
            *(dest+1) += volMulR * s;
            dest += 2;
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
    float volMulL = dsmMixLeftVolFloat * (1.0f / 65536.0f);
    float volMulR = dsmMixRightVolFloat * (1.0f / 65536.0f);

    if ( nextSampleOffset != 1 )
    {
        while ( numSamples )
        {
            i0 = (float) (65536 - (mixPos & 0xFFFF));
            i1 = (float) (mixPos & 0xFFFF);
            *dest += volMulL *
                (i0 * dsmByteFloatTable[sample[2 * (mixPos >> 16)]] +
                 i1 * dsmByteFloatTable[sample[2 * (mixPos >> 16) +
                                              2 * nextSampleOffset]]);
            *(dest+1) += volMulR *
                (i0 * dsmByteFloatTable[sample[2 * (mixPos >> 16) + 1]] +
                 i1 * dsmByteFloatTable[sample[2 * (mixPos >> 16) +
                                              2 * nextSampleOffset + 1]]);
            dest += 2;
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
            *dest += volMulL *
                (i0 * dsmByteFloatTable[sample[2 * (mixPos >> 16)]] +
                 i1 * dsmByteFloatTable[sample[2 * (mixPos >> 16) + 2]]);
            *(dest+1) += volMulR *
                (i0 * dsmByteFloatTable[sample[2 * (mixPos >> 16) + 1]] +
                 i1 * dsmByteFloatTable[sample[2 * (mixPos >> 16) + 3]]);
            dest += 2;
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
    int mixPos = dsmMixSrcPos;
    float i0, i1;
    float volMulL = dsmMixLeftVolFloat * (1.0f / 65536.0f);
    float volMulR = dsmMixRightVolFloat * (1.0f / 65536.0f);

    if ( nextSampleOffset != 1 )
    {
        while ( numSamples )
        {
            i0 = (float) (65536 - (mixPos & 0xFFFF));
            i1 = (float) (mixPos & 0xFFFF);
            *dest += volMulL *
                (i0 * ((float) sample[2 * (mixPos >> 16)]) +
                 i1 * ((float) sample[2 * (mixPos >> 16) +
                                     2 * nextSampleOffset]));
            *(dest+1) += volMulR * 
                (i0 * ((float) sample[2 * (mixPos >> 16) + 1]) +
                 i1 * ((float) sample[2 * (mixPos >> 16) +
                                     2 * nextSampleOffset + 1]));
            dest += 2;
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
            *dest += volMulL *
                (i0 * ((float) sample[2 * (mixPos >> 16)]) +
                 i1 * ((float) sample[2 * (mixPos >> 16) + 2]));
            *(dest+1) += volMulR * 
                (i0 * ((float) sample[2 * (mixPos >> 16) + 1]) +
                 i1 * ((float) sample[2 * (mixPos >> 16) + 3]));
            dest += 2;
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
    float volMulL = dsmMixLeftVolFloat * (1.0f / 65536.0f);
    float volMulR = dsmMixRightVolFloat * (1.0f / 65536.0f);

    if ( nextSampleOffset != 1 )
    {
        while ( numSamples )
        {
            i0 = (float) (65536 - (mixPos & 0xFFFF));
            i1 = (float) (mixPos & 0xFFFF);
            *dest += volMulL *
                (i0 * dsmUlawFloatTable[sample[2 * (mixPos >> 16)]] +
                 i1 * dsmUlawFloatTable[sample[2 * (mixPos >> 16) +
                                              2 * nextSampleOffset]]);
            *(dest+1) += volMulR *
                (i0 * dsmUlawFloatTable[sample[2 * (mixPos >> 16) + 1]] +
                 i1 * dsmUlawFloatTable[sample[2 * (mixPos >> 16) +
                                              2 * nextSampleOffset + 1]]);
            dest += 2;
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
            *dest += volMulL *
                (i0 * dsmUlawFloatTable[sample[2 * (mixPos >> 16)]] +
                 i1 * dsmUlawFloatTable[sample[2 * (mixPos >> 16) + 2]]);
            *(dest+1) += volMulR *
                (i0 * dsmUlawFloatTable[sample[2 * (mixPos >> 16) + 1]] +
                 i1 * dsmUlawFloatTable[sample[2 * (mixPos >> 16) + 3]]);
            dest += 2;
            mixPos += dsmMixStep;
            numSamples--;
        }
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}



#ifdef M_X86_ASSEMBLER
extern void CALLING dsmMixStereo8MonoAsmFI(unsigned numSamples,
                                           int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereo8MonoAsmFIReloc;

extern void CALLING dsmMixStereo16MonoAsmFI(unsigned numSamples,
                                            int nextSampleOffset);

extern void CALLING dsmMixStereoUlawMonoAsmFI(unsigned numSamples,
                                              int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereoUlawMonoAsmFIReloc;
#endif


dsmMixRoutineSet dsmMixStereoFloatInterp =
{
    {
        { 0, 0, NULL, NULL, NULL },                         /* no sample */
#ifdef M_X86_ASSEMBLER        
        { 1, 2, &mix8Mono, &dsmMixStereo8MonoAsmFI,
          &dsmMixStereo8MonoAsmFIReloc },                   /* 8-bit mono */
        { 1, 2, &mix16Mono, &dsmMixStereo16MonoAsmFI, NULL},/* 16-bit mono */
#else
        { 1, 1, &mix8Mono, &mix8Mono, NULL },               /* 8-bit mono */
        { 1, 1, &mix16Mono, &mix16Mono, NULL },             /* 16-bit mono */
#endif        
        { 1, 1, &mix8Stereo, &mix8Stereo, NULL },           /* 8-bit stereo */
        { 1, 1, &mix16Stereo, &mix16Stereo, NULL },         /* 16-bit stereo */
        { 0, 0, NULL, NULL, NULL },                         /* ADPCM mono */
        { 0, 0, NULL, NULL, NULL },                         /* ADPCM stereo */
#ifdef M_X86_ASSEMBLER        
        { 1, 2, &mixUlawMono, &dsmMixStereoUlawMonoAsmFI,
          &dsmMixStereoUlawMonoAsmFIReloc },                /* 8-bit mono */
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
 * $Log: mix_sfi.c,v $
 * Revision 1.9.2.1  1997/09/06 17:16:02  pekangas
 * Optimized the floating point mixing routines for Pentium Pro / Pentium II
 *
 * Revision 1.9  1997/07/31 10:56:51  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.8  1997/07/30 19:04:03  pekangas
 * Added optimized mono u-law mixing routines and enabled u-law autoconversion
 * by default
 *
 * Revision 1.7  1997/07/24 17:10:02  pekangas
 * Added surround support and support for separate surround and middle mixing
 * routines.
 *
 * Revision 1.6  1997/07/03 15:03:46  pekangas
 * Interpolating mixing no longer indexes past the real sample or loop end,
 * thus replicating data for it is not necessary
 *
 * Revision 1.5  1997/06/26 19:13:51  pekangas
 * Added assembler versions of some of the routines
 *
 * Revision 1.4  1997/06/26 14:33:09  pekangas
 * Added proper interpolation for stereo samples
 *
 * Revision 1.3  1997/06/20 10:08:54  pekangas
 * Fixed to compile with Visual C
 *
 * Revision 1.2  1997/06/12 12:50:45  pekangas
 * Fixed 16-bit sample mixing
 *
 * Revision 1.1  1997/06/11 15:36:31  pekangas
 * Initial revision
 *
*/
