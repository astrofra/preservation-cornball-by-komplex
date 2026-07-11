/*      mix_mi.c
 *
 * Digital Sound Mixer, mono integer non-interpolating mixing routines
 *
 * $Id: mix_mi.c,v 1.5 1997/07/31 10:56:51 pekangas Exp $
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

RCSID(const char *mix_mi_rcsid = "$Id: mix_mi.c,v 1.5 1997/07/31 10:56:51 pekangas Exp $";)



/* Mix 8-bit mono samples: */
static void CALLING mix8Mono(unsigned numSamples, int nextSampleOffset)
{
    int *dest = (int*) dsmMixDest;
    uchar *sample = dsmMixSample;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        *(dest++) += dsmMixLeftVolTable[sample[mixPos >> 16]];
        mixPos += dsmMixStep;
        numSamples--;
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}


/* Mix 16-bit mono samples: */
static void CALLING mix16Mono(unsigned numSamples, int nextSampleOffset)
{
    int *dest = (int*) dsmMixDest;
    short *sample = (short*) dsmMixSample;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        *(dest++) += (dsmMixLeftVolInt * ((int) sample[mixPos >> 16])) >> 8;
        mixPos += dsmMixStep;
        numSamples--;
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}



/* Mix u-law mono samples: */
static void CALLING mixUlawMono(unsigned numSamples, int nextSampleOffset)
{
    int *dest = (int*) dsmMixDest;
    uchar *sample = dsmMixSample;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        *(dest++) += dsmMixLeftUlawVolTable[sample[mixPos >> 16]];
        mixPos += dsmMixStep;
        numSamples--;
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}




/* Mix 8-bit stereo samples: */
static void CALLING mix8Stereo(unsigned numSamples, int nextSampleOffset)
{
    int *dest = (int*) dsmMixDest;
    uchar *sample = dsmMixSample;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        *(dest++) += dsmMixLeftVolTable[sample[2 * (mixPos >> 16)]] +
            dsmMixLeftVolTable[sample[2 * (mixPos >> 16) + 1]];
        mixPos += dsmMixStep;
        numSamples--;
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}


/* Mix 16-bit stereo samples: */
static void CALLING mix16Stereo(unsigned numSamples, int nextSampleOffset)
{
    int *dest = (int*) dsmMixDest;
    short *sample = (short*) dsmMixSample;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        *(dest++) += (dsmMixLeftVolInt * (((int) sample[2 * (mixPos >> 16)]) +
                                         ((int) sample[2 * (mixPos >> 16)+1])))
                                                                    >> 8;
        mixPos += dsmMixStep;
        numSamples--;
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}


/* Mix u-law stereo samples: */
static void CALLING mixUlawStereo(unsigned numSamples, int nextSampleOffset)
{
    int *dest = (int*) dsmMixDest;
    uchar *sample = dsmMixSample;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        *(dest++) += dsmMixLeftUlawVolTable[sample[2 * (mixPos >> 16)]] +
            dsmMixLeftUlawVolTable[sample[2 * (mixPos >> 16) + 1]];
        mixPos += dsmMixStep;
        numSamples--;
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}


#ifdef M_X86_ASSEMBLER
extern void CALLING dsmMixMono8MonoAsmI(unsigned numSamples,
                                        int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixMono8MonoAsmIReloc;

extern void CALLING dsmMixMono16MonoAsmI(unsigned numSamples,
                                         int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixMono16MonoAsmIReloc;

extern void CALLING dsmMixMonoUlawMonoAsmI(unsigned numSamples,
                                           int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixMonoUlawMonoAsmIReloc;
 #endif




dsmMixRoutineSet dsmMixMonoInteger =
{
    {
        { 0, 0, NULL, NULL, NULL },                     /* no sample */
#ifdef M_X86_ASSEMBLER        
        { 1, 2, &mix8Mono, &dsmMixMono8MonoAsmI,
          &dsmMixMono8MonoAsmIReloc },                  /* 8-bit mono */
        { 1, 2, &mix16Mono, &dsmMixMono16MonoAsmI,
          &dsmMixMono16MonoAsmIReloc },                 /* 8-bit mono */
#else
        { 1, 1, &mix8Mono, &mix8Mono, NULL },           /* 8-bit mono */
        { 1, 1, &mix16Mono, &mix16Mono, NULL },         /* 16-bit mono */
#endif        
        { 1, 1, &mix8Stereo, &mix8Stereo, NULL },       /* 8-bit stereo */
        { 1, 1, &mix16Stereo, &mix16Stereo, NULL },     /* 16-bit stereo */
        { 0, 0, NULL, NULL, NULL },                     /* ADPCM mono */
        { 0, 0, NULL, NULL, NULL },                     /* ADPCM stereo */
#ifdef M_X86_ASSEMBLER        
        { 1, 2, &mixUlawMono, &dsmMixMonoUlawMonoAsmI,
          &dsmMixMonoUlawMonoAsmIReloc },               /* u-law mono */
#else
        { 1, 1, &mixUlawMono, &mixUlawMono, NULL },     /* u-law mono */
#endif        
        { 1, 1, &mixUlawStereo, &mixUlawStereo, NULL }  /* u-law stereo */
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
 * $Log: mix_mi.c,v $
 * Revision 1.5  1997/07/31 10:56:51  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.4  1997/07/30 19:04:03  pekangas
 * Added optimized mono u-law mixing routines and enabled u-law autoconversion
 * by default
 *
 * Revision 1.3  1997/07/24 17:10:01  pekangas
 * Added surround support and support for separate surround and middle mixing
 * routines.
 *
 * Revision 1.2  1997/07/17 13:31:27  pekangas
 * Added optimized 16-bit mono sample mixing
 * Optimized 8-bit mono sample mixing a bit more
 *
 * Revision 1.1  1997/07/17 09:55:08  pekangas
 * Initial revision
 *
*/
