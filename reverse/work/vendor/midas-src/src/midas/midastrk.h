/*      midastrk.h
 *
 * MIDAS tracker API
 *
 * $Id: midastrk.h,v 1.5.2.2 1997/11/20 20:43:03 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/


#ifndef __midastrk_h
#define __midastrk_h


/* This is a kluge, but necessary as Watcom C sucks: */
#ifdef EXPORT_IN_MIDASDLL_H

#ifdef __WC32__
    #define _FUNC(x) x __export __stdcall
    #define MIDAS_CALL __cdecl
#else
    #define _FUNC(x) __declspec(dllexport) x __stdcall
    #define MIDAS_CALL __cdecl
#endif

#else
    #if defined(__UNIX__) || defined(__DJGPP__)
        #define _FUNC(x) x
        #define MIDAS_CALL
        #define EMPTYARRAY 0
    #else
        #define EMPTYARRAY
        #ifdef __DOS__
            #define _FUNC(x) x cdecl
            #define MIDAS_CALL __cdecl
        #else
            #define _FUNC(x) x __stdcall
            #define MIDAS_CALL __cdecl
        #endif
    #endif
#endif


#if defined(__WATCOMC__) && defined(__cplusplus)
/* Disable to annoying Watcom C++ warnings - I have no idea how to get around
   these without breaking Visual C compatibility: */
#pragma warning 604 9
#pragma warning 594 9
#endif


/* We'll need to define DWORD, BOOL, TRUE and FALSE if someone hasn't
   done that before. For now, we'll just assume that if no-one has defined
   TRUE we need to define everything. There definitions are compatible with
   windows.h. If something else in your system defines these differently,
   things should still work OK as long as FALSE is 0, TRUE is nonzero and
   DWORD is 32-bit. Take care that you don't compare BOOLs like "bool == TRUE"
   in that case though, just use "bool".

   THIS IS UGLY AND MAY NEED FIXING!
   ---------------------------------
*/

#ifndef TRUE
#define TRUE 1
#define FALSE 0
typedef int BOOL;
typedef unsigned long DWORD;
#endif /* ifndef TRUE */

#ifndef __MTYPES_H
typedef unsigned char uchar;
#ifndef _LINUX_TYPES_H
typedef unsigned short ushort;
typedef unsigned long ulong;
#endif
#endif


/* The command numbers: */
enum MTRKcommands
{
    MTRKcNone = 0,                       /* no command */
    MTRKcArpeggio = 1,                   /* arpeggio */
    MTRKcSlideUp = 2,                    /* period slide up */
    MTRKcSlideDown = 3,                  /* period slide down */
    MTRKcTonePortamento = 4,             /* tone portamento */
    MTRKcVibrato = 5,                    /* vibrato */
    MTRKcTPortVSlide = 6,                /* tone portamento + volume slide */
    MTRKcVibVSlide = 7,                  /* vibrato + volume slide */
    MTRKcTremolo = 8,                    /* tremolo */
    MTRKcSetPanning = 9,                 /* set panning (PT cmd 8) */
    MTRKcSampleOffset = 10,              /* set sample offset */
    MTRKcVolumeSlide = 11,               /* volume slide */
    MTRKcPositionJump = 12,              /* position jump */
    MTRKcSetVolume = 13,                 /* set volume */
    MTRKcPatternBreak = 14,              /* pattern break (to a row) */
    MTRKcSetSpeed = 15,                  /* set speed */
    MTRKcSetTempo = 16,                  /* set tempo in BPM */
    MTRKcFineSlideUp = 17,               /* fine period slide up */
    MTRKcFineSlideDown = 18,             /* fine period slide down */
    MTRKcPatternLoop = 19,               /* pattern loop set/loop */
    MTRKcSetPanning16 = 20,              /* set 16-point panning value */
    MTRKcPTRetrig = 21,                  /* Protracker-style retrig note */
    MTRKcFineVolSlideUp = 22,            /* fine volume slide up */
    MTRKcFineVolSlideDown = 23,          /* fine volume slide down */
    MTRKcNoteCut = 24,                   /* note cut */
    MTRKcNoteDelay = 25,                 /* note delay */
    MTRKcPatternDelay = 26,              /* pattern delay */
    MTRKcSetMVolume = 27,                /* set master volume */
    MTRKcMVolSlide = 28,                 /* master volume slide */
    MTRKcS3MRetrig = 29,                 /* S3M retrig note */
    MTRKcMusicSync = 30,                 /* music synchronization command */
    MTRKcExtraFineSlideUp = 31,          /* extra fine period slide up */
    MTRKcExtraFineSlideDown = 32,        /* extra fine period slide down */
    MTRKcPanSlide = 33,                  /* panning slide */
    MTRKnumCommands                      /* total number of commands */
};



/* Sample looping modes: */
enum MTRKloopModes
{
    MTRKloopNone = 0,                   /* no looping */
    MTRKloop1 = 1,                      /* 1 loop, release ends note */
    MTRKloop1Rel = 2,                   /* 1 loop, sample data after loop is
                                           played when note is released */
    MTRKloop2 = 3,                      /* 2 loops, when note is released
                                           playing is continued looping the
                                           second loop */
    MTRKloopAmigaNone = 4,              /* Amiga compatible looping, no
                                           loop - ALE logic enabled anyway */
    MTRKloopAmiga = 5                   /* Amiga compatible looping - 1
                                           loop, ALE logic enabled */
};


/* Sample loop types: (for a single loop) */
enum MTRKloopTypes
{
/*    MTRKloopNone = 0,*/                   /* no looping */
    MTRKloopUnidir = 1,                 /* unidirectional loop */
    MTRKloopBidir = 2                   /* bidirectional loop */    
};



/* The sample structure: */
typedef struct
{
    char        name[32];               /* sample name */
    unsigned    sdHandle;               /* Sound Device sample handle */
    uchar       *sample;                /* sample data pointer, NULL if sample
                                           data only in SD memory */
    unsigned    sampleLength;           /* sample length */
    uchar       sampleType;             /* sample type, see enum
                                           sdSampleType */
    uchar       loopMode;               /* sample looping mode, see enum
                                           sdLoopMode */
    uchar       loop1Type;              /* first loop type, see enum
                                           sdLoopType */
    uchar       loop2Type;              /* second loop type, see enum
                                           sdLoopType */
    unsigned    loop1Start;             /* first loop start */
    unsigned    loop1End;               /* first loop end */
    unsigned    loop2Start;             /* second loop start */
    unsigned    loop2End;               /* second loop end */
    unsigned    volume;                 /* sample volume */
    unsigned    gvolume;                /* sample global volume */
    int         panning;                /* sample panning position */
    long        baseTune;               /* base tune */
    int         finetune;               /* fine tuning value */
    ulong       fileOffset;             /* sample start offset in module
                                           file */
    uchar       vibType;                /* IT auto-vibrato type */
    uchar       vibSweep;               /* IT auto-vibrato sweep value */
    uchar       vibDepth;               /* IT auto-vibrato depth */
    uchar       vibRate;                /* IT auto-vibrato rate */    
    /* In tracker mode, the sample pointer is valid. */
} MTRKsample;



/* Envelope point structure: */
typedef struct
{
    ushort       x;
    signed short y;
} MTRKenvelopePoint;



/* Instrument envelope structure. */

typedef struct
{
    uchar       numPoints;              /* number of points in envelope */
    
    signed char sustain;                /* sustain point number, -1 if no
                                           sustain is used */
    signed char loopStart;              /* envelope loop start point number,
                                           -1 if no loop */
    signed char loopEnd;                /* envelope loop end point number, -1
                                           if no loop */
    signed char susLoopStart;           /* sustain loop start point number,
                                           -1 if no loop */
    signed char susLoopEnd;             /* sustain loop end point number, -1
                                           if no loop */
    MTRKenvelopePoint points[25];       /* envelope points: a maximum of 25
                                           points in (X,Y) coordinate pairs */
} MTRKenvelope;



/* The instrument structure. In tracker mode, with XMs, the module loader
   always allocates space for 16 sample pointers for each instrument.*/
typedef struct
{
    char        name[32];               /* instrument name */
    unsigned    numSamples;             /* number of samples */
    uchar       *noteSamples;           /* sample numbers for all keyboard
                                           notes, NULL if sample 0 is used
                                           for all (always 96 bytes) */
    MTRKenvelope *volEnvelope;          /* pointer to volume envelope info or
                                           NULL if volume envelope is not
                                           used */
    MTRKenvelope *panEnvelope;          /* pointer to panning envelope info or
                                           NULL if panning envelope is not
                                           used */
    MTRKenvelope *pitchEnvelope;        /* (IT) pointer to pitch envelope info
                                           or NULL if pitch envelope is not
                                           used */    
    int         volFadeout;             /* FT2/IT volume fade-out speed */
    unsigned    volume;                 /* IT instrument volume */
    uchar       vibType;                /* FT2 auto-vibrato type */
    uchar       vibSweep;               /* FT2 auto-vibrato sweep value */
    uchar       vibDepth;               /* FT2 auto-vibrato depth */
    uchar       vibRate;                /* FT2 auto-vibrato rate */
    uchar       NNA;                    /* IT New Note Action */
    uchar       DCT;                    /* IT Duplicate note Check Type */
    uchar       DCA;                    /* IT Duplicate note Check Action */
    uchar       PPS;                    /* IT Pitch Pan Separation */
    uchar       PPC;                    /* IT Pitch Pan Center note */
    uchar       DfP;                    /* IT Default Panning */
    uchar       RV;                     /* IT Random Volume */
    uchar       RP;                     /* IT Random Panning */
    
    int         used;                   /* 1 if instrument is used, 0 if not.
                                           Unused instruments are not loaded
                                           or added to Sound Device */
    MTRKsample*  samples[EMPTYARRAY];   /* pointers to samples - the numbers
                                           in the noteSamples[] array refer
                                           to these sample numbers in FT2 mode,
                                           and to global sample numbers in IT
                                           mode. */
} MTRKinstrument;




/* The pattern structure: */
typedef struct
{
    unsigned    length;                 /* pattern size in bytes, INCLUDING
                                           this header */
    unsigned    rows;                   /* number of rows in pattern */
    uchar       data[EMPTYARRAY];       /* pattern data */

    /* Pattern data format:
       The first byte is compression information byte. Bits 0-4 form the
       channel number (0-31) and bits 5-7 compression info:
                bit 5   if 1, note and instrument number bytes follow
                bit 6   if 1, volume column byte follows
                bit 7   if 1, command and infobyte bytes follow
       If the compression infobyte is 0, it marks the end of the current
       row. Possible other data follows the compression information byte in
       the following order: note, instrument, volume, command, infobyte.

       In note numbers the upper nybble is the octave number, and lower
       nybble note number in octave. So, for example, D#2 becomes 0x23.
       0xFE as a note number represents a release note, and 0xFF no new
       note as does 0xFF as an instrument number. Valid instrument numbers
       range from 0 to 254 inclusive.

       Volume column number 0x50 is ignored, like command number 0.

       Pattern data decompressor marks that all data entries have all
       elements. It also allocates space for 256 rows of pattern data.
       "length" is ignored in decompressed patterns. Each pattern data
       row has module.numChannels MTRKpatternDataEntry structures for the
       actual data, plus a 0 byte for the end-of-row marker. Thus the
       length of each row is
       module.numChannels*sizeof(MTRKpatternDataEntry) + 1.
    */
} MTRKpattern;



/* Decompressed pattern data entry: */
typedef struct
{
    uchar compInfo;
    uchar note;
    uchar instrument;
    uchar volume;
    uchar command;
    uchar infobyte;
} MTRKpatternDataEntry;



/* The MIDAS module structure - MIDASmodule is really a pointer to this.
   In tracker mode, the loaders always allocate room for 256 instrument
   structure pointers, 16384 sample pointers, 256 pattern pointers,
   32 channels of panning positions and 256 song data entries. */
typedef struct
{
    uchar       name[32];               /* module name */
    int         playMode;               /* module playing mode */
    unsigned    songLength;             /* song length */
    unsigned    restart;                /* song restart position */
    unsigned    numInsts;               /* number of instruments */
    unsigned    numSamples;             /* number of samples */
    unsigned    numPatts;               /* number of patterns */
    uchar       numChannels;            /* number of channels in module */
    uchar       masterVolume;           /* initial master volume */
    uchar       speed;                  /* initial speed */
    uchar       tempo;                  /* initial BPM tempo */
    int         *panning;               /* initial channel panning
                                           positions */
    ushort      *songData;              /* song data - pattern playing
                                           order */
    MTRKinstrument **instruments;       /* instrument data */
    MTRKsample  **samples;              /* sample data */    
    MTRKpattern **patterns;             /* pattern data */
    struct  /* Format-specific playing information flags: */
    {
        int     ptLimits : 1;           /* ST3: Protracker limits */
        int     fastVolSlides : 1;      /* ST3: Fast volume slides */
        int     extOctaves : 1;         /* PT: Extended octaves needed */
        int     linFreqTable : 1;       /* FT2: Linear frequency table used */
	int     instruments : 1;        /* IT: Instrument mode used */        
    } playFlags;
} MTRKmodule;







#ifdef __cplusplus
extern "C" {
#endif


_FUNC(BOOL)     MTRKdecompressPattern(MTRKmodule *module, unsigned pattern);
_FUNC(BOOL)     MTRKcompressPattern(MTRKmodule *module, unsigned pattern);

_FUNC(MIDASmodulePlayHandle) MTRKplayModuleInstruments(MTRKmodule *module);
_FUNC(BOOL)     MTRKplayInstrumentNote(MIDASmodulePlayHandle playHandle,
                                       unsigned channel, unsigned instrument,
                                       unsigned note);
_FUNC(BOOL)     MTRKplaySampleNote(MIDASmodulePlayHandle playHandle,
                                   unsigned channel, unsigned instrument,
                                   unsigned sample, unsigned note);
_FUNC(BOOL)     MTRKreleaseNote(MIDASmodulePlayHandle playHandle,
                                unsigned channel);
_FUNC(BOOL)     MTRKstopNote(MIDASmodulePlayHandle playHandle,
                             unsigned channel);
_FUNC(void*)    MTRKmemAlloc(unsigned numBytes);
_FUNC(BOOL)     MTRKmemFree(void *block);

_FUNC(BOOL)     MTRKresetSample(MTRKmodule *module, unsigned instrument,
                                unsigned sample,
                                MIDASmodulePlayHandle playHandle);
_FUNC(BOOL)     MTRKremoveInstrument(MTRKmodule *module, unsigned instrument,
                                     MIDASmodulePlayHandle playHandle);
_FUNC(BOOL)     MTRKremoveSample(MTRKmodule *module, unsigned instrument,
                                 unsigned sample,
                                 MIDASmodulePlayHandle playHandle);
_FUNC(BOOL)     MTRKnewInstrument(MTRKmodule *module, unsigned instrument,
                                  MIDASmodulePlayHandle playHandle);
_FUNC(BOOL)     MTRKnewSample(MTRKmodule *module, unsigned instrument,
                              unsigned sample,
                              MIDASmodulePlayHandle playHandle);

_FUNC(BOOL)     MTRKsetPlayTempo(MIDASmodulePlayHandle playHandle,
                                 unsigned tempo);
_FUNC(BOOL)     MTRKsetPlaySpeed(MIDASmodulePlayHandle playHandle,
                                 unsigned speed);
_FUNC(unsigned) MTRKgetPlayTempo(MIDASmodulePlayHandle playHandle);
_FUNC(unsigned) MTRKgetPlaySpeed(MIDASmodulePlayHandle playHandle);

#ifdef __cplusplus
}
#endif




#endif


/*
 * $Log: midastrk.h,v $
 * Revision 1.5.2.2  1997/11/20 20:43:03  pekangas
 * Added speed and tempo functions
 *
 * Revision 1.5.2.1  1997/11/18 19:36:23  pekangas
 * Added functions for changing sample data, and adding and removing samples
 * and instruments
 *
 * Revision 1.5  1997/08/13 17:35:33  pekangas
 * Updated for 1.1, added instrument playing code and memAlloc/Free wrappers
 *
 * Revision 1.4  1997/07/31 10:56:50  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.3  1997/06/02 00:54:16  jpaana
 * Changed most __LINUX__ defines to __UNIX__ for generic *nix porting
 *
 * Revision 1.2  1997/05/26 16:40:38  pekangas
 * Some comment updates
 *
 * Revision 1.1  1997/05/20 09:39:12  pekangas
 * Initial revision
 *
*/
