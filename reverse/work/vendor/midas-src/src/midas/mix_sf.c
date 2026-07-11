/*      mix_sf.c
 *
 * Digital Sound Mixer, stereo floating point non-interpolating mixing routines
 *
 * $Id: mix_sf.c,v 1.7.2.1 1997/09/06 17:16:02 pekangas Exp $
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

RCSID(const char *mix_sf_rcsid = "$Id: mix_sf.c,v 1.7.2.1 1997/09/06 17:16:02 pekangas Exp $";)



/* Mix 8-bit mono samples: */
static void CALLING mix8Mono(unsigned numSamples, int nextSampleOffset)
{
    float *dest = (float*) dsmMixDest;
    float s;
    uchar *sample = dsmMixSample;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;
	
    while ( numSamples )
    {
        s = dsmByteFloatTable[sample[mixPos >> 16]];
        *dest += dsmMixLeftVolFloat * s;
        *(dest+1) += dsmMixRightVolFloat * s;
        dest += 2;
        mixPos += dsmMixStep;
        numSamples--;
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}


/* Mix 16-bit mono samples: */
static void CALLING mix16Mono(unsigned numSamples, int nextSampleOffset)
{
    float *dest = (float*) dsmMixDest;
    float s;
    S16 *sample = (S16*) dsmMixSample;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;
    
    while ( numSamples )
    {
        s = (float) sample[mixPos >> 16];
        *dest += dsmMixLeftVolFloat * s;
        *(dest+1) += dsmMixRightVolFloat * s;
        dest += 2;
        mixPos += dsmMixStep;
        numSamples--;
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}



/* Mix u-law mono samples: */
static void CALLING mixUlawMono(unsigned numSamples, int nextSampleOffset)
{
    float *dest = (float*) dsmMixDest;
    float s;
    uchar *sample = dsmMixSample;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;
    
    while ( numSamples )
    {
        s = dsmUlawFloatTable[sample[mixPos >> 16]];
        *dest += dsmMixLeftVolFloat * s;
        *(dest+1) += dsmMixRightVolFloat * s;
        dest += 2;
        mixPos += dsmMixStep;
        numSamples--;
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}




/* Mix 8-bit stereo samples: */
static void CALLING mix8Stereo(unsigned numSamples, int nextSampleOffset)
{
    float *dest = (float*) dsmMixDest;
    uchar *sample = dsmMixSample;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        *dest += dsmMixLeftVolFloat *
            dsmByteFloatTable[sample[2*(dsmMixSrcPos >> 16)]];
        *(dest+1) += dsmMixRightVolFloat *
            dsmByteFloatTable[sample[2*(dsmMixSrcPos >> 16) + 1]];
        dest += 2;
        dsmMixSrcPos += dsmMixStep;
        numSamples--;
    }

    dsmMixDest = (uchar*) dest;
}


/* Mix 16-bit stereo samples: */
static void CALLING mix16Stereo(unsigned numSamples, int nextSampleOffset)
{
    float *dest = (float*) dsmMixDest;
    S16 *sample = (S16*) dsmMixSample;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        *dest += dsmMixLeftVolFloat *
            ((float) sample[2*(dsmMixSrcPos >> 16)]);
        *(dest+1) += dsmMixRightVolFloat *
            ((float) sample[2*(dsmMixSrcPos >> 16) + 1]);
        dest += 2;
        dsmMixSrcPos += dsmMixStep;
        numSamples--;
    }

    dsmMixDest = (uchar*) dest;    
}


/* Mix u-law stereo samples: */
static void CALLING mixUlawStereo(unsigned numSamples, int nextSampleOffset)
{
    float *dest = (float*) dsmMixDest;
    uchar *sample = dsmMixSample;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        *dest += dsmMixLeftVolFloat *
            dsmUlawFloatTable[sample[2*(dsmMixSrcPos >> 16)]];
        *(dest+1) += dsmMixRightVolFloat *
            dsmUlawFloatTable[sample[2*(dsmMixSrcPos >> 16) + 1]];
        dest += 2;
        dsmMixSrcPos += dsmMixStep;
        numSamples--;
    }

    dsmMixDest = (uchar*) dest;
}



#ifdef M_X86_ASSEMBLER
extern void CALLING dsmMixStereo8MonoAsmF(unsigned numSamples,
                                          int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereo8MonoAsmFReloc;

extern void CALLING dsmMixStereo16MonoAsmF(unsigned numSamples,
                                           int nextSampleOffset);

extern void CALLING dsmMixStereoUlawMonoAsmF(unsigned numSamples,
                                             int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereoUlawMonoAsmFReloc;
#endif

dsmMixRoutineSet dsmMixStereoFloat =
{
    {
        { 0, 0, NULL, NULL, NULL },                         /* no sample */
#ifdef M_X86_ASSEMBLER        
        { 1, 2, &mix8Mono, &dsmMixStereo8MonoAsmF,
          &dsmMixStereo8MonoAsmFReloc },                    /* 8-bit mono */
        { 1, 2, &mix16Mono, &dsmMixStereo16MonoAsmF, NULL },/* 16-bit mono */
#else
        { 1, 1, &mix8Mono, &mix8Mono, NULL },               /* 8-bit mono */
        { 1, 1, &mix16Mono, &mix16Mono, NULL },             /* 16-bit mono */
#endif        
        { 1, 1, &mix8Stereo, &mix8Stereo, NULL },           /* 8-bit stereo */
        { 1, 1, &mix16Stereo, &mix16Stereo, NULL },         /* 16-bit stereo */
        { 0, 0, NULL, NULL, NULL },                         /* ADPCM mono */
        { 0, 0, NULL, NULL, NULL },                         /* ADPCM stereo */
#ifdef M_X86_ASSEMBLER
        { 1, 2, &mixUlawMono, &dsmMixStereoUlawMonoAsmF,
          &dsmMixStereoUlawMonoAsmFReloc },                 /* 8-bit mono */
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
 * $Log: mix_sf.c,v $
 * Revision 1.7.2.1  1997/09/06 17:16:02  pekangas
 * Optimized the floating point mixing routines for Pentium Pro / Pentium II
 *
 * Revision 1.7  1997/07/31 10:56:51  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.6  1997/07/30 19:04:03  pekangas
 * Added optimized mono u-law mixing routines and enabled u-law autoconversion
 * by default
 *
 * Revision 1.5  1997/07/24 17:10:01  pekangas
 * Added surround support and support for separate surround and middle mixing
 * routines.
 *
 * Revision 1.4  1997/07/03 15:03:46  pekangas
 * Interpolating mixing no longer indexes past the real sample or loop end,
 * thus replicating data for it is not necessary
 *
 * Revision 1.3  1997/06/04 15:25:42  pekangas
 * Added faster assembler versions of the main mixing loops
 *
 * Revision 1.2  1997/05/31 22:50:39  jpaana
 * Some slight performance tweaks
 *
 * Revision 1.1  1997/05/30 18:25:02  pekangas
 * Initial revision
 *
*/
