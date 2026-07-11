/*      mix_si.c
 *
 * Digital Sound Mixer, stereo integer non-interpolating mixing routines
 *
 * $Id: mix_si.c,v 1.5.2.2 1997/08/20 17:49:26 pekangas Exp $
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

RCSID(const char *mix_si_rcsid = "$Id: mix_si.c,v 1.5.2.2 1997/08/20 17:49:26 pekangas Exp $";)



/* Mix 8-bit mono samples: */
static void CALLING mix8Mono(unsigned numSamples, int nextSampleOffset)
{
    int *dest = (int*) dsmMixDest;
    uchar *sample = dsmMixSample, s;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        s = sample[mixPos >> 16];
        *(dest++) += dsmMixLeftVolTable[s];
        *(dest++) += dsmMixRightVolTable[s];
        mixPos += dsmMixStep;
        numSamples--;
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}


/* Mix 8-bit mono samples, surround: */
static void CALLING mix8MonoS(unsigned numSamples, int nextSampleOffset)
{
    int *dest = (int*) dsmMixDest;
    uchar *sample = dsmMixSample, s;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        s = sample[mixPos >> 16];
        *(dest++) += dsmMixLeftVolTable[s];
        *(dest++) -= dsmMixRightVolTable[s];
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
    int s;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        s = (int) sample[mixPos >> 16];
        *(dest++) += (dsmMixLeftVolInt * s) >> 8;
        *(dest++) += (dsmMixRightVolInt * s) >> 8;
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
    uchar *sample = dsmMixSample, s;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        s = sample[mixPos >> 16];
        *(dest++) += dsmMixLeftUlawVolTable[s];
        *(dest++) += dsmMixRightUlawVolTable[s];
        mixPos += dsmMixStep;
        numSamples--;
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}




/* Mix u-law mono samples, surround: */
static void CALLING mixUlawMonoS(unsigned numSamples, int nextSampleOffset)
{
    int *dest = (int*) dsmMixDest;
    uchar *sample = dsmMixSample, s;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        s = sample[mixPos >> 16];
        *(dest++) += dsmMixLeftUlawVolTable[s];
        *(dest++) -= dsmMixRightUlawVolTable[s];
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
        *(dest++) += dsmMixLeftVolTable[sample[2 * (mixPos >> 16)]];
        *(dest++) += dsmMixRightVolTable[sample[2 * (mixPos >> 16) + 1]];
        mixPos += dsmMixStep;
        numSamples--;
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}


/* Mix 8-bit stereo samples, surround: */
static void CALLING mix8StereoS(unsigned numSamples, int nextSampleOffset)
{
    int *dest = (int*) dsmMixDest;
    uchar *sample = dsmMixSample;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        *(dest++) += dsmMixLeftVolTable[sample[2 * (mixPos >> 16)]];
        *(dest++) -= dsmMixRightVolTable[sample[2 * (mixPos >> 16) + 1]];
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
        *(dest++) += (dsmMixLeftVolInt * ((int) sample[2 * (mixPos >> 16)]))
                                                                   >> 8;
        *(dest++) += (dsmMixRightVolInt * ((int) sample[2 * (mixPos >> 16)
                                                       + 1])) >> 8;
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
        *(dest++) += dsmMixLeftUlawVolTable[sample[2 * (mixPos >> 16)]];
        *(dest++) += dsmMixRightUlawVolTable[sample[2 * (mixPos >> 16) + 1]];
        mixPos += dsmMixStep;
        numSamples--;
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}



/* Mix u-law stereo samples, surround: */
static void CALLING mixUlawStereoS(unsigned numSamples, int nextSampleOffset)
{
    int *dest = (int*) dsmMixDest;
    uchar *sample = dsmMixSample;
    int mixPos = dsmMixSrcPos;

    nextSampleOffset = nextSampleOffset;

    while ( numSamples )
    {
        *(dest++) += dsmMixLeftUlawVolTable[sample[2 * (mixPos >> 16)]];
        *(dest++) -= dsmMixRightUlawVolTable[sample[2 * (mixPos >> 16) + 1]];
        mixPos += dsmMixStep;
        numSamples--;
    }

    dsmMixSrcPos = mixPos;
    dsmMixDest = (uchar*) dest;
}



#ifdef M_X86_ASSEMBLER
extern void CALLING dsmMixStereo8MonoAsmI(unsigned numSamples,
                                          int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereo8MonoAsmIReloc;
extern void CALLING dsmMixStereo8MonoAsmSI(unsigned numSamples,
                                           int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereo8MonoAsmSIReloc;
extern void CALLING dsmMixStereo8MonoAsmMI(unsigned numSamples,
                                           int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereo8MonoAsmMIReloc;

extern void CALLING dsmMixStereo16MonoAsmI(unsigned numSamples,
                                           int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereo16MonoAsmIReloc;
extern void CALLING dsmMixStereo16MonoAsmSI(unsigned numSamples,
                                            int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereo16MonoAsmSIReloc;
extern void CALLING dsmMixStereo16MonoAsmMI(unsigned numSamples,
                                            int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereo16MonoAsmMIReloc;

extern void CALLING dsmMixStereoUlawMonoAsmI(unsigned numSamples,
                                             int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereoUlawMonoAsmIReloc;
extern void CALLING dsmMixStereoUlawMonoAsmSI(unsigned numSamples,
                                              int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereoUlawMonoAsmSIReloc;
extern void CALLING dsmMixStereoUlawMonoAsmMI(unsigned numSamples,
                                              int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereoUlawMonoAsmMIReloc;

extern void CALLING dsmMixStereo16StereoAsmI(unsigned numSamples,
                                             int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereo16StereoAsmIReloc;
extern void CALLING dsmMixStereo16StereoAsmMI(unsigned numSamples,
                                              int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereo16StereoAsmMIReloc;
extern void CALLING dsmMixStereo16StereoAsmSI(unsigned numSamples,
                                              int nextSampleOffset);
extern dsmMixLoopRelocInfo dsmMixStereo16StereoAsmSIReloc;
#endif


dsmMixRoutineSet dsmMixStereoInteger =
{
    {
        { 0, 0, NULL, NULL, NULL },                     /* no sample */
#ifdef M_X86_ASSEMBLER        
        { 1, 4, &mix8Mono, &dsmMixStereo8MonoAsmI,
          &dsmMixStereo8MonoAsmIReloc },                /* 8-bit mono */
        { 1, 1, &mix16Mono, &dsmMixStereo16MonoAsmI,
          &dsmMixStereo16MonoAsmIReloc },               /* 16-bit mono */
        { 1, 1, &mix8Stereo, &mix8Stereo, NULL },       /* 8-bit stereo */
        { 1, 1, &mix16Mono, &dsmMixStereo16StereoAsmI,
          &dsmMixStereo16StereoAsmIReloc },             /* 16-bit stereo */
#else
        { 1, 1, &mix8Mono, &mix8Mono, NULL },           /* 8-bit mono */
        { 1, 1, &mix16Mono, &mix16Mono, NULL },         /* 16-bit mono */
        { 1, 1, &mix8Stereo, &mix8Stereo, NULL },       /* 8-bit stereo */
        { 1, 1, &mix16Stereo, &mix16Stereo, NULL },     /* 16-bit stereo */
#endif
        { 0, 0, NULL, NULL, NULL },                     /* ADPCM mono */
        { 0, 0, NULL, NULL, NULL },                     /* ADPCM stereo */
#ifdef M_X86_ASSEMBLER
        { 1, 4, &mixUlawMono, &dsmMixStereoUlawMonoAsmI,
          &dsmMixStereoUlawMonoAsmIReloc },             /* u-bit mono */
#else        
        { 1, 1, &mixUlawMono, &mixUlawMono, NULL },     /* u-law mono */
#endif        
        { 1, 1, &mixUlawStereo, &mixUlawStereo, NULL }  /* u-law stereo */
    },
    
    /* Middle: */
    {
        { 0, 0, NULL, NULL, NULL },
#ifdef M_X86_ASSEMBLER        
        { 1, 4, &mix8Mono, &dsmMixStereo8MonoAsmMI,
          &dsmMixStereo8MonoAsmMIReloc },                   /* 8-bit mono */
        { 1, 1, &mix16Mono, &dsmMixStereo16MonoAsmMI,
          &dsmMixStereo16MonoAsmMIReloc },                  /* 16-bit mono */
        { 0, 0, NULL, NULL, NULL },
        { 1, 1, &mix16Stereo, &dsmMixStereo16StereoAsmMI,
          &dsmMixStereo16StereoAsmMIReloc },                /* 16-bit stereo */
        { 0, 0, NULL, NULL, NULL }, { 0, 0, NULL, NULL, NULL },
        { 1, 4, &mixUlawMono, &dsmMixStereoUlawMonoAsmMI,
          &dsmMixStereoUlawMonoAsmMIReloc },            /* u-law mono */
#else
        { 0, 0, NULL, NULL, NULL }, { 0, 0, NULL, NULL, NULL },
        { 0, 0, NULL, NULL, NULL }, { 0, 0, NULL, NULL, NULL },
        { 0, 0, NULL, NULL, NULL }, { 0, 0, NULL, NULL, NULL },
        { 0, 0, NULL, NULL, NULL },
#endif        
        { 0, 0, NULL, NULL, NULL }
    },

    /* Surround: */
    {
        { 0, 0, NULL, NULL, NULL },                         /* no sample */
#ifdef M_X86_ASSEMBLER        
        { 1, 4, &mix8MonoS, &dsmMixStereo8MonoAsmSI,
          &dsmMixStereo8MonoAsmSIReloc },                   /* 8-bit mono */
        { 1, 1, &mix16Mono, &dsmMixStereo16MonoAsmSI,
          &dsmMixStereo16MonoAsmSIReloc },                  /* 8-bit mono */
        { 1, 1, &mix8StereoS, &mix8StereoS, NULL },         /* 8-bit stereo */
        { 1, 1, &mix16Stereo, &dsmMixStereo16StereoAsmSI,
          &dsmMixStereo16StereoAsmSIReloc },                /* 16-bit stereo */
        { 0,0, NULL, NULL, NULL }, { 0, 0, NULL, NULL, NULL },
        { 1, 4, &mixUlawMonoS, &dsmMixStereoUlawMonoAsmSI,
          &dsmMixStereoUlawMonoAsmSIReloc },                /* u-law mono */
#else
        { 1, 1, &mix8MonoS, &mix8MonoS, NULL },             /* 8-bit mono */
        { 1, 1, &mix16Mono, &mix16Mono, NULL },             /* 16-bit mono */
        { 1, 1, &mix8StereoS, &mix8StereoS, NULL },         /* 8-bit stereo */
        { 1, 1, &mix16Stereo, &mix16Stereo, NULL },         /* 16-bit stereo */
        { 0,0, NULL, NULL, NULL }, { 0, 0, NULL, NULL, NULL },
        { 1, 1, &mixUlawMonoS, &mixUlawMonoS, NULL },       /* u-law mono */
#endif
        { 1, 1, &mixUlawStereoS, &mixUlawStereoS, NULL }    /* u-law stereo */
    }    
};


/*
 * $Log: mix_si.c,v $
 * Revision 1.5.2.2  1997/08/20 17:49:26  pekangas
 * Added assembler optimized versions of stereo integer mixing of
 * 16-bit stereo samples with middle and surround panning
 *
 * Revision 1.5.2.1  1997/08/20 14:07:49  pekangas
 * Added an assembler-optimized 16-bit stereo integer mixing routine
 *
 * Revision 1.5  1997/07/31 10:56:51  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.4  1997/07/30 19:04:03  pekangas
 * Added optimized mono u-law mixing routines and enabled u-law autoconversion
 * by default
 *
 * Revision 1.3  1997/07/30 13:33:44  pekangas
 * Added optimized integer middle mixing routines
 * Optimized integer surround routines
 *
 * Revision 1.2  1997/07/24 17:10:02  pekangas
 * Added surround support and support for separate surround and middle mixing
 * routines.
 *
 * Revision 1.1  1997/07/24 14:55:34  pekangas
 * Initial revision
*/
