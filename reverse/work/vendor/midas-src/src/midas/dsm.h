/*      DSM.H
 *
 * Digital Sound Mixer
 *
 * $Id: dsm.h,v 1.26.2.1 1997/08/20 12:48:04 pekangas Exp $
 *
 * Copyright 1996,1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#ifndef __dsm_h
#define __dsm_h


#define VOLLEVELS 33
#define VOLSHIFT 1
#define VOLADD 1

/* Magic sample handle for streams: */
#define DSM_SMP_STREAM MAXSAMPLES


/****************************************************************************\
*       enum dsmMixMode
*       ---------------
* Description:  Available DSM mixing modes
\****************************************************************************/

enum dsmMixMode
{
    dsmMix8bit = 1,
    dsmMix16bit = 2,
    dsmMixMono = 4,
    dsmMixStereo = 16,
    dsmMix8bitMono = dsmMix8bit | dsmMixMono,
    dsmMix8bitStereo = dsmMix8bit | dsmMixStereo,
    dsmMix16bitMono = dsmMix16bit | dsmMixMono,
    dsmMix16bitStereo = dsmMix16bit | dsmMixStereo,
    dsmMixFloat = 32,
    dsmMixInteger = 64,
    dsmMixInterpolation = 128
};



/****************************************************************************\
*       enum dsmDataFormat
*       ------------------
* Description:  Possible DSM data formats, for post-processing functions
\****************************************************************************/

enum dsmDataFormat
{
    dsmDataFloat = 0,
    dsmDataInt
};



/****************************************************************************\
*       enum dsmPostProcPosition
*       ------------------------
* Description:  Post-processing position
\****************************************************************************/

enum dsmPostProcPosition
{
    dsmPostProcFirst = 0,
    dsmPostProcLast
};



/****************************************************************************\
*       enum dsmChanStatus
*       ------------------
* Description:  DSM channel sound playing status
\****************************************************************************/

enum dsmChanStatus
{
    dsmChanStopped = 0,                 /* playing is stopped */
    dsmChanEnd,                         /* playing has ended (not forced
                                           stopped */
    dsmChanPlaying,                     /* playing, not released */
    dsmChanReleased,                    /* playing, note has been released */
    dsmChanFadeOut                      /* fading out */
};




/****************************************************************************\
*       enum dsmPlayDir
*       ---------------
* Description:  Playing direction in bidirectional loops
\****************************************************************************/

enum dsmPlayDir
{
    dsmPlayBackwards = -1,              /* playing backwards */
    dsmPlayForward = 1                  /* playing forward */
};




/****************************************************************************\
*       struct dsmChannel
*       -----------------
* Description:  DSM channel data
\****************************************************************************/

typedef struct
{
    uchar       *sample;                /* sample data pointer */
    int         sampleType;             /* sample type, see enum
                                           sdSampleType */
    int         origSampleType;         /* original sample type, before
                                           possible autoconversion */
    int         samplePos;              /* sample position in memory */
    unsigned    sampleLength;           /* sample length */
    int         loopMode;               /* sample looping mode, see enum
                                           sdLoopMode */
    unsigned    loop1Start;             /* first loop start */
    unsigned    loop1End;               /* first loop end */
    int         loop1Type;              /* first loop type, see enum
                                           sdLoopType */
    unsigned    loop2Start;             /* second loop start */
    unsigned    loop2End;               /* second loop end */
    int         loop2Type;              /* second loop type, see enum
                                           sdLoopType */

    unsigned    playPos;                /* playing position whole part */
    unsigned    playPosLow;             /* playing position fractional part
                                           (only lower 16 bits used) */
    unsigned    streamWritePos;         /* stream write position */
    ulong       rate;                   /* playing rate in Hz */
    int         direction;              /* playing direction in bidirectional
                                           loops - 1 is forward, -1 back */
    unsigned    sampleHandle;           /* sample handle */
    unsigned    sampleChanged;          /* 1 if sample has been changed
                                           but values not yet set in
                                           channel struct */
    int         panning;                /* panning information */
    unsigned    volume;                 /* playing volume (0-64) */
    int         muted;                  /* 1 if channel muted, 0 if not */
    int         status;                 /* channel status, see enum dsm
                                           dsmChanStatus */
    int         loopNum;                /* currently played sample loop */
    void (CALLING *LoopCallback)(unsigned channel);     /* sample loop
                                                           callback */
    uchar       transBuffer[16];        /* translation buffer for
                                           interpolating mixing */
    int         inTranslation;          /* in translation mode */
    unsigned    numTransSamples;        /* number of translation samples */
    unsigned    transPos;               /* translation playing position,
                                           whole part */
    int         transSampleType;        /* translation buffer sample type */
    unsigned    leftVol;                /* actual left volume, 16.16 fixed
                                           point. Includes panning and master
                                           volume. */
    unsigned    rightVol;               /* actual right volume */
    unsigned    leftVolTarget, rightVolTarget;  /* volume ramp targets */
    unsigned    leftRampSpeed, rightRampSpeed;  /* volume ramp speeds */
    int         volumeRamping;          /* volume ramp in progress */
} dsmChannel;




/****************************************************************************\
*       struct dsmSample
*       ----------------
* Description:  DSM internal sample structure
\****************************************************************************/

typedef struct
{
    uchar       *sample;                /* sample data pointer */
    int         sampleType;             /* sample type, see enum
                                           sdSampleType */
    int         origSampleType;         /* original sample type, before
                                           possible autoconversion */
    int         samplePos;              /* sample position in memory */
    unsigned    sampleLength;           /* sample length */
    int         loopMode;               /* sample looping mode, see enum
                                           sdLoopMode */
    unsigned    loop1Start;             /* first loop start */
    unsigned    loop1End;               /* first loop end */
    int         loop1Type;              /* first loop type, see enum
                                           sdLoopType */
    unsigned    loop2Start;             /* second loop start */
    unsigned    loop2End;               /* second loop end */
    int         loop2Type;              /* second loop type, see enum
                                           sdLoopType */
    int         inUse;                  /* 1 if sample is in use, 0 if not
                                           (removed using dsmRemoveSample) */
    int         copied;                 /* 1 if a copied sample (should be
                                           deallocated), 0 if not */
} dsmSample;



/****************************************************************************\
*       struct dsmMixLoopRelocInfo
*       --------------------------
* Description:  Mixing loop relocation information structure
\****************************************************************************/

typedef struct
{
    unsigned numCopyBytes;              /* number of bytes to copy */
    U32 *relocTable;                    /* relocation table */
    unsigned numRelocEntries;           /* number of relocation table
                                           entries */
} dsmMixLoopRelocInfo;



/****************************************************************************\
*       struct dsmMixRoutine
*       --------------------
* Description:  A single mixing routine
\****************************************************************************/

typedef struct
{
    unsigned mainLoopAlign;
    unsigned mainLoopRepeat;
    void (CALLING *mixLoop)(unsigned numSamples, int nextSampleOffset);
    void (CALLING *mainMixLoop)(unsigned numSamples, int nextSampleOffset);
    dsmMixLoopRelocInfo *mainLoopReloc;
} dsmMixRoutine;



/****************************************************************************\
*       struct dsmMixRoutineSet
*       -----------------------
* Description:  A complete mixing routine set for a given mixing mode
\****************************************************************************/

typedef struct
{
    dsmMixRoutine routines[sdNumSampleTypes];         /* normal */
    dsmMixRoutine middleRoutines[sdNumSampleTypes];   /* middle panning */
    dsmMixRoutine surroundRoutines[sdNumSampleTypes]; /* surround panning */
} dsmMixRoutineSet;



/****************************************************************************\
*       typedef ... dsmPostProcFunction
*       -------------------------------
* Description:  A post processing function pointer.
\****************************************************************************/

typedef void (CALLING *dsmPostProcFunction)(void *data, unsigned numSamples,
                                            void *user);



/****************************************************************************\
*       typedef dsmPostProcessor
*       ------------------------
* Description:  A post processing routine structure
\****************************************************************************/

typedef struct _dsmPostProcessor
{
    struct _dsmPostProcessor *next, *prev;  /* list links, used by DSM */
    void *userData;                         /* user data arg, used by DSM */
    dsmPostProcFunction floatMono;
    dsmPostProcFunction floatStereo;
    dsmPostProcFunction intMono;
    dsmPostProcFunction intStereo;
} dsmPostProcessor;



#ifdef __cplusplus
extern "C" {
#endif




extern void * GLOBALVAR dsmMixBuffer;   /* DSM mixing buffer. dsmPlay()
                                           writes the mixed data here. Post-
                                           processing is usually necessary. */
extern unsigned GLOBALVAR dsmMixBufferSize;      /* DSM mixing buffer size */

/* The following global variables are used internally by different DSM
   functions and should not be accessed by other modules: */

extern unsigned GLOBALVAR dsmMixRate;      /* mixing rate in Hz */
extern unsigned GLOBALVAR dsmMode;         /* output mode (see enum
                                              dsmMixMode) */
extern int GLOBALVAR *dsmVolumeTable;      /* pointer to volume table */
extern int GLOBALVAR *dsmUlawVolumeTable;  /* u-law volume table */

extern float GLOBALVAR *dsmByteFloatTable; /* sample byte->float table */
extern float GLOBALVAR *dsmUlawFloatTable; /* sample ulaw->float table */


extern dsmChannel * GLOBALVAR dsmChannels;  /* pointer to channel datas */
extern dsmSample * GLOBALVAR dsmSamples;    /* sample structures */

extern unsigned GLOBALVAR dsmNumChannels;   /* number of open channels */
extern unsigned GLOBALVAR dsmAllocatedChannels; /* number of allocated
                                                   channels */
extern unsigned GLOBALVAR dsmAmplification; /* amplification level */
extern dsmMixRoutineSet* GLOBALVAR dsmActMixRoutines;
extern unsigned GLOBALVAR dsmMasterVolume;  /* master volume */
extern int GLOBALVAR dsmMuted;              /* 1 if muted, 0 if not */
extern int GLOBALVAR dsmPaused;             /* 1 if paused, 0 if not */
extern volatile unsigned GLOBALVAR dsmChPlay; /* 1 if data on channels may be
                                                 played */
extern dsmPostProcessor* GLOBALVAR dsmPostProcList; /* the post-processor
                                                       list */
extern int GLOBALVAR dsmAutoFilterType;     /* autodetected filter type */



/* Global variables for mixing routine state information: */
extern int GLOBALVAR dsmMixSrcPos;
extern uchar* GLOBALVAR dsmMixDest;
extern float GLOBALVAR dsmMixLeftVolFloat, dsmMixRightVolFloat;
extern int GLOBALVAR dsmMixLeftVolInt, dsmMixRightVolInt;
extern int* GLOBALVAR dsmMixLeftVolTable;
extern int* GLOBALVAR dsmMixRightVolTable;
extern int* GLOBALVAR dsmMixLeftUlawVolTable;
extern int* GLOBALVAR dsmMixRightUlawVolTable;
extern int GLOBALVAR dsmMixStep;
extern uchar* GLOBALVAR dsmMixSample;

/* Mixing routine sets: */
extern dsmMixRoutineSet GLOBALVAR dsmMixMonoFloat;
extern dsmMixRoutineSet GLOBALVAR dsmMixStereoFloat;
extern dsmMixRoutineSet GLOBALVAR dsmMixMonoFloatInterp;
extern dsmMixRoutineSet GLOBALVAR dsmMixStereoFloatInterp;
extern dsmMixRoutineSet GLOBALVAR dsmMixMonoInteger;
extern dsmMixRoutineSet GLOBALVAR dsmMixStereoInteger;



/****************************************************************************\
*
* Function:     int dsmInit(unsigned mixRate, unsigned mode);
*
* Description:  Initializes Digital Sound Mixer
*
* Input:        unsigned mixRate        mixing rate in Hz
*               unsigned mode           mixing mode (see enum dsmMixMode)
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmInit(unsigned mixRate, unsigned mode);




/****************************************************************************\
*
* Function:     int dsmClose(void)
*
* Description:  Uninitializes Digital Sound Mixer
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmClose(void);




/****************************************************************************\
*
* Function:     int dsmGetMixRate(unsigned *mixRate)
*
* Description:  Reads the actual mixing rate
*
* Input:        unsigned *mixRate       pointer to mixing rate variable
*
* Returns:      MIDAS error code.
*               Mixing rate, in Hz, is stored in *mixRate
*
\****************************************************************************/

int CALLING dsmGetMixRate(unsigned *mixRate);




/****************************************************************************\
*
* Function:     int dsmOpenChannels(unsigned channels);
*
* Description:  Opens channels for output
*
* Input:        unsigned channels       number of channels to open
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmOpenChannels(unsigned channels);




/****************************************************************************\
*
* Function:     int dsmCalcVolTable(unsigned amplification)
*
* Description:  Calculates a new volume table
*
* Input:        unsigned amplification  Amplification level. 64 - normal
*                                       (100%), 32 = 50%, 128 = 200% etc.
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmCalcVolTable(unsigned amplification);




/****************************************************************************\
*
* Function:     int dsmCloseChannels(void)
*
* Description:  Closes open output channels
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmCloseChannels(void);




/****************************************************************************\
*
* Function:     int dsmClearChannels(void)
*
* Description:  Clears open channels (removes all sounds)
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmClearChannels(void);




/****************************************************************************\
*
* Function:     int dsmMute(int mute)
*
* Description:  Mutes all channels
*
* Input:        int mute                1 = mute, 0 = un-mute
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmMute(int mute);




/****************************************************************************\
*
* Function:     int dsmPause(int pause)
*
* Description:  Pauses or resumes playing
*
* Input:        int pause               1 = pause, 0 = resume
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmPause(int pause);




/****************************************************************************\
*
* Function:     int dsmSetMasterVolume(unsigned masterVolume)
*
* Description:  Sets the master volume
*
* Input:        unsigned masterVolume   master volume (0 - 64)
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetMasterVolume(unsigned masterVolume);




/****************************************************************************\
*
* Function:     int dsmGetMasterVolume(unsigned *masterVolume)
*
* Description:  Reads the master volume
*
* Input:        unsigned *masterVolume  pointer to master volume
*
* Returns:      MIDAS error code. Master volume is written to *masterVolume.
*
\****************************************************************************/

int CALLING dsmGetMasterVolume(unsigned *masterVolume);




/****************************************************************************\
*
* Function:     int dsmSetAmplification(unsigned amplification)
*
* Description:  Sets amplification level and calculates new volume table.
*
* Input:        unsigned amplification  amplification level, 64 = normal
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetAmplification(unsigned amplification);




/****************************************************************************\
*
* Function:     int dsmGetAmplification(unsigned *amplification)
*
* Description:  Reads the amplification level
*
* Input:        unsigned *amplification   pointer to amplification level
*
* Returns:      MIDAS error code. Amplification level is written to
*               *amplification.
*
\****************************************************************************/

int CALLING dsmGetAmplification(unsigned *amplification);




/****************************************************************************\
*
* Function:     int dsmPlaySound(unsigned channel, ulong rate)
*
* Description:  Starts playing a sound
*
* Input:        unsigned channel        channel number
*               ulong rate              playing rate in Hz
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmPlaySound(unsigned channel, ulong rate);




/****************************************************************************\
*
* Function:     int dsmReleaseSound(unsigned channel)
*
* Description:  Releases the current sound from the channel. If sdLoop1Rel or
*               sdLoop2 looping modes are used, playing will be continued from
*               the release part of the current sample (data after the end
*               of the first loop) after the end of the first loop is reached
*               next time, otherwise the sound will be stopped.
*
* Input:        unsigned channel        channel number
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmReleaseSound(unsigned channel);




/****************************************************************************\
*
* Function:     int dsmStopSound(unsigned channel)
*
* Description:  Stops playing a sound
*
* Input:        unsigned channel        channel number
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmStopSound(unsigned channel);




/****************************************************************************\
*
* Function:     int dsmSetRate(unsigned channel, ulong rate)
*
* Description:  Sets the playing rate
*
* Input:        unsigned channel        channel number
*               ulong rate              playing rate in Hz
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetRate(unsigned channel, ulong rate);




/****************************************************************************\
*
* Function:     int dsmGetRate(unsigned channel, ulong *rate)
*
* Description:  Reads the playing rate on a channel
*
* Input:        unsigned channel        channel number
*               ulong *rate             pointer to playing rate
*
* Returns:      MIDAS error code. Playing rate is written to *rate, 0 if
*               no sound is being played.
*
\****************************************************************************/

int CALLING dsmGetRate(unsigned channel, ulong *rate);




/****************************************************************************\
*
* Function:     int dsmSetVolume(unsigned channel, unsigned volume)
*
* Description:  Sets the playing volume
*
* Input:        unsigned channel        channel number
*               unsigned volume         playing volume (0-64)
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetVolume(unsigned channel, unsigned volume);




/****************************************************************************\
*
* Function:     int dsmGetVolume(unsigned channel, unsigned *volume)
*
* Description:  Reads the playing volume
*
* Input:        unsigned channel        channel number
*               unsigned *volume        pointer to volume
*
* Returns:      MIDAS error code. Playing volume is written to *volume.
*
\****************************************************************************/

int CALLING dsmGetVolume(unsigned channe, unsigned *volume);




/****************************************************************************\
*
* Function:     int dsmSetSample(unsigned channel, unsigned smpHandle)
*
* Description:  Sets the sample number on a channel
*
* Input:        unsigned channel        channel number
*               unsigned smpHandle      sample handle returned by
*                                       dsmAddSample()
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetSample(unsigned channel, unsigned smpHandle);




/****************************************************************************\
*
* Function:     int dsmGetSample(unsigned channel, unsigned *smpHandle)
*
* Description:  Reads current sample handle
*
* Input:        unsigned channel        channel number
*               unsigned *smpHandle     pointer to sample handle
*
* Returns:      MIDAS error code. Sample handle is written to *smpHandle;
*
\****************************************************************************/

int CALLING dsmGetSample(unsigned channel, unsigned *smpHandle);




/****************************************************************************\
*
* Function:     int dsmChangeSample(unsigned channel)
*
* Description:  Changes the sample used in a channel to the one specified
*               by the channel's sample handle. Used only internally by
*               other DSM functions, does no error checking.
*
* Input:        unsigned channel        channel number
*
* Returns:      MIDAS error code (does not fail)
*
\****************************************************************************/

int CALLING dsmChangeSample(unsigned channel);




/****************************************************************************\
*
* Function:     int dsmSetPosition(unsigned channel, unsigned position)
*
* Description:  Sets the playing position from the beginning of the sample
*
* Input:        unsigned channel        channel number
*               unsigned position       new playing position
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetPosition(unsigned channel, unsigned position);




/****************************************************************************\
*
* Function:     int dsmGetPosition(unsigned channel, unsigned *position)
*
* Description:  Reads the current playing position
*
* Input:        unsigned channel        channel number
*               unsigned *position      pointer to playing position
*
* Returns:      MIDAS error code. Playing position is written to *position.
*
\****************************************************************************/

int CALLING dsmGetPosition(unsigned channel, unsigned *position);




/****************************************************************************\
*
* Function:     int dsmGetDirection(unsigned channel, int *direction)
*
* Description:  Reads current playing direction
*
* Input:        unsigned channel        channel number
*               int *direction          pointer to playing direction. 1 is
*                                       forward, -1 backwards
*
* Returns:      MIDAS error code. Playing direction is written to *direction.
*
\****************************************************************************/

int CALLING dsmGetDirection(unsigned channel, int *direction);




/****************************************************************************\
*
* Function:     int dsmSetPanning(unsigned channel, int panning)
*
* Description:  Sets the panning position of a channel
*
* Input:        unsigned channel        channel number
*               int panning             panning position (see enum sdPanning)
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetPanning(unsigned channel, int panning);




/****************************************************************************\
*
* Function:     int dsmGetPanning(unsigned channel, int *panning)
*
* Description:  Reads the panning position of a channel
*
* Input:        unsigned channel        channel number
*               int *panning            pointer to panning position
*
* Returns:      MIDAS error code. Panning position is written to *panning.
*
\****************************************************************************/

int CALLING dsmGetPanning(unsigned channel, int *panning);




/****************************************************************************\
*
* Function:     int dsmMuteChannel(unsigned channel, int mute)
*
* Description:  Mutes/un-mutes a channel
*
* Input:        unsigned channel        channel number
*               int mute                muting status - 1 = mute, 0 = un-mute
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmMuteChannel(unsigned channel, int mute);




/****************************************************************************\
*
* Function:     int dsmAddSample(sdSample *sample, int copySample,
*                   unsigned *smpHandle);
*
* Description:  Adds a new sample to the DSM sample list and prepares it for
*               DSM use
*
* Input:        sdSample *sample        pointer to sample information
*                                           structure
*               int copySample          copy sample data to a new place in
*                                       memory? 1 = yes, 0 = no
*               unsigned *smpHandle     pointer to sample handle
*
* Returns:      MIDAS error code. Sample handle for the new sample is written
*               to *smpHandle
*
* Notes:        If copySample = 1, sample data must not be in EMS memory
*
\****************************************************************************/

int CALLING dsmAddSample(sdSample *sample, int copySample, unsigned *smpHandle);




/****************************************************************************\
*
* Function:     int dsmRemoveSample(unsigned smpHandle)
*
* Description:  Removes a sample from the sample list and deallocates it if
*               necessary.
*
* Input:        unsigned smpHandle      sample handle returned by
*                                       dsmAddSample()
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmRemoveSample(unsigned smpHandle);




/****************************************************************************\
*
* Function:     int dsmMixData(unsigned numSamples, int mix8bit, void *dest)
*
* Description:  Mixes sound data to *dest
*
* Input:        unsigned numSamples     number of destination samples to mix.
*                                       In stereo mode, a "sample" consists
*                                       really of two samples
*               void *dest              destination buffer
*
* Returns:      MIDAS error code. Mixed data is written to *dest. The output
*               data is 8-bit unsigned bytes for 8-bit output, and 16-bit
*               signed words for 16-bit output.
*
\****************************************************************************/

int CALLING dsmMixData(unsigned numSamples, void *dest);




/****************************************************************************\
*
* Function:     int dsmMix(unsigned channel, void *mixRoutine,
*                   unsigned volume, unsigned numElems);
*
* Description:  Mixes data for one channel. Used internally by dsmMixData().
*
* Input:        unsigned channel        channel number
*               void *mixRoutine        pointer to low-level mixing routine
*               unsigned volume         actual playing volume (volume in
*                                       channel structure is ignored)
*               unsigned numElems       number of elements to mix (see
*                                       dsmMixData())
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmMix(unsigned channel, void *mixRoutine, unsigned volume,
    unsigned numElems);




/****************************************************************************\
*
* Function:     int dsmClearBuffer(unsigned numElems)
*
* Description:  Clears the mixing buffer. Used only by dsmMixData().
*
* Input:        unsigned numElems       number of elements to clear
*
* Returns:      MIDAS error code.
*
\****************************************************************************/

int CALLING dsmClearBuffer(unsigned numElems);




#ifdef SUPPORTSTREAMS


/****************************************************************************\
*
* Function:     int dsmStartStream(unsigned channel, uchar *buffer,
*                   unsigned bufferLength, int sampleType);
*
* Description:  Starts playing a digital audio stream on a channel
*
* Input:        unsigned channel        channel number
*               uchar *buffer           pointer to stream buffer
*               unsigned bufferLength   buffer length in bytes
*               int sampleType          stream sample type
*               ulong rate              stream playing rate (in Hz)
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmStartStream(unsigned channel, uchar *buffer, unsigned
    bufferLength, int sampleType, ulong rate);




/****************************************************************************\
*
* Function:     int dsmStopStream(unsigned channel);
*
* Description:  Stops playing digital audio stream on a channel
*
* Input:        unsigned channel        channel number
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmStopStream(unsigned channel);




/****************************************************************************\
*
* Function:     int dsmSetLoopCallback(unsigned channel,
*                   void (CALLING *callback)(unsigned channel));
*
* Description:  Sets sample looping callback to a channel
*
* Input:        unsigned channel        channel number
*               [..] *callback          pointer to callback function, NULL to
*                                       disable callback
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetLoopCallback(unsigned channel,
    void (CALLING *callback)(unsigned channel));




/****************************************************************************\
*
* Function:     int dsmSetStreamWritePosition(unsigned channel,
*                   unsigned position)
*
* Description:  Sets the stream write position on a channel
*
* Input:        unsigned channel        channel number
*               unsigned position       new stream write position
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmSetStreamWritePosition(unsigned channel, unsigned position);




/****************************************************************************\
*
* Function:     int dsmPauseStream(unsigned channel, int pause)
*
* Description:  Pauses/resumes playing a stream on a channel
*
* Input:        unsigned channel        the channel the stream is being played
*                                       on
*               int pause               1 to pause, 0 to resume
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmPauseStream(unsigned channel, int pause);




#endif /* #ifdef SUPPORTSTREAMS */


int CALLING dsmMixChannel(unsigned channel, dsmMixRoutineSet *routineSet,
                          unsigned numSamples, void *dest);


/****************************************************************************\
*
* Function:     int dsmAddPostProcessor(dsmPostProcessor *postProc,
*                   unsigned procPos);
*
* Description:  Adds a post processor to the sound output
*
* Input:        dsmPostProcessor *postProc  the post-processor structure
*               unsigned procPos            post-processing position, see
*                                           enum dsmPostProcPosition
*               void *userData              user data argument for the
*                                           post-processing functions
*
* Returns:      MIDAS error code.
*
* Notes:        If any of the functions pointer is NULL, that function is
*               ignored.
*
\****************************************************************************/

int CALLING dsmAddPostProcessor(dsmPostProcessor *postProc, unsigned procPos,
                                void *userData);




/****************************************************************************\
*
* Function:     int dsmRemovePostProcessor(dsmPostProcessor *postProc)
*
* Description:  Removes a post processor from sound output
*
* Input:        dsmPostProcessor *postProc  the post-processor structure
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dsmRemovePostProcessor(dsmPostProcessor *postProc);


int CALLING dsmRelocMixingRoutine(dsmMixRoutine *routine);



/* Prototypes for the actual mixing routines: */
void CALLING dsmMix8bitMonoMono(void);      /* 8-bit Mono => Mono */
void CALLING dsmMix8bitMonoStereo(void);    /* 8-bit Mono => Stereo */
void CALLING dsmMix8bitStereoMono(void);    /* 8-bit Stereo => Mono */
void CALLING dsmMix8bitStereoStereo(void);  /* 8-bit Stereo => Stereo */
void CALLING dsmMix16bitMonoMono(void);     /* 16-bit Mono => Mono */
void CALLING dsmMix16bitMonoStereo(void);   /* 16-bit Mono => Stereo */
void CALLING dsmMix16bitStereoMono(void);   /* 16-bit Stereo => Mono */
void CALLING dsmMix16bitStereoStereo(void); /* 16-bit Stereo => Stereo */


void CALLING dsmFilterFloatMono(void *data, unsigned numSamples, void *user);
void CALLING dsmFilterFloatStereo(void *data, unsigned numSamples, void *user);
void CALLING dsmFilterIntMono(void *data, unsigned numSamples, void *user);
void CALLING dsmFilterIntStereo(void *data, unsigned numSamples, void *user);





#ifdef __cplusplus
}
#endif



/****************************************************************************\
*       enum dsmFunctIDs
*       ----------------
* Description:  ID numbers for DSM functions
\****************************************************************************/

enum dsmFunctIDs
{
    ID_dsmInit = ID_dsm,
    ID_dsmClose,
    ID_dsmGetMixRate,
    ID_dsmOpenChannels,
    ID_dsmCalcVolTable,
    ID_dsmCloseChannels,
    ID_dsmClearChannels,
    ID_dsmMute,
    ID_dsmPause,
    ID_dsmSetMasterVolume,
    ID_dsmGetMasterVolume,
    ID_dsmSetAmplification,
    ID_dsmGetAmplification,
    ID_dsmPlaySound,
    ID_dsmReleaseSound,
    ID_dsmStopSound,
    ID_dsmSetRate,
    ID_dsmGetRate,
    ID_dsmSetVolume,
    ID_dsmGetVolume,
    ID_dsmSetSample,
    ID_dsmGetSample,
    ID_dsmChangeSample,
    ID_dsmSetPosition,
    ID_dsmGetPosition,
    ID_dsmGetDirection,
    ID_dsmSetPanning,
    ID_dsmGetPanning,
    ID_dsmMuteChannel,
    ID_dsmAddSample,
    ID_dsmRemoveSample,
    ID_dsmMixData,
    ID_dsmMix,
    ID_dsmMixMoNormal,
    ID_dsmMixStNormal,
    ID_dsmClearBuffer,
    ID_dsmStartStream,
    ID_dsmStopStream,
    ID_dsmSetLoopCallback,
    ID_dsmSetStreamWritePosition,
    ID_dsmPauseStream,
    ID_dsmAddPostProcessor,
    ID_dsmRemovePostProcessor,
    ID_dsmFadeOut,
    ID_dsmRelocMixingRoutine
};


/****************************************************************************\
*       enum dsmProfIDs
*       ---------------
* Description:  Profiling IDs for DSM functions
\****************************************************************************/

#ifdef __mprof_h

enum dsmProfIDs
{
    DSM_PROF_FILTER_FLOAT_MONO = M_PROF_DSM + 0x40,
    DSM_PROF_FILTER_FLOAT_STEREO,
    DSM_PROF_FILTER_INT_MONO,
    DSM_PROF_FILTER_INT_STEREO
};

#endif


#endif


/*
 * $Log: dsm.h,v $
 * Revision 1.26.2.1  1997/08/20 12:48:04  pekangas
 * Manually merged fixes from the main trunk (oops)
 *
 * Revision 1.27  1997/08/19 17:22:21  pekangas
 * Added profiling IDs for filtering functions
 *
 * Revision 1.26  1997/07/31 10:56:38  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.25  1997/07/25 13:47:55  pekangas
 * Added automatic filtering and made it the default
 *
 * Revision 1.24  1997/07/25 09:58:09  pekangas
 * Added integer filters
 *
 * Revision 1.23  1997/07/24 17:10:00  pekangas
 * Added surround support and support for separate surround and middle mixing
 * routines.
 *
 * Revision 1.22  1997/07/24 14:54:10  pekangas
 * Added support for integer stereo mixing
 *
 * Revision 1.21  1997/07/17 13:31:05  pekangas
 * Added support for copying and relocating mixing loops in data segment,
 * thus allowing self-modifying code.
 *
 * Revision 1.20  1997/07/17 09:55:46  pekangas
 * Added support for integer mono mixing
 *
 * Revision 1.19  1997/07/10 18:40:22  pekangas
 * Added echo effect support
 *
 * Revision 1.18  1997/07/05 14:21:09  pekangas
 * Fixed the clicks for good
 *
 * Revision 1.17  1997/07/03 15:03:45  pekangas
 * Interpolating mixing no longer indexes past the real sample or loop end,
 * thus replicating data for it is not necessary
 *
 * Revision 1.16  1997/06/20 10:09:46  pekangas
 * Added volume ramping
 *
 * Revision 1.15  1997/06/11 15:37:10  pekangas
 * Added interpolating stereo mixing routines
 *
 * Revision 1.14  1997/06/11 14:37:07  pekangas
 * Added support for post-processing routines.
 * Added automatic amplification for dsmOpenChannel().
 * Removed unused outputBits -argument from dsmInit().
 *
 * Revision 1.13  1997/06/10 23:03:35  jpaana
 * Unified sample type definitions
 *
 * Revision 1.12  1997/06/05 20:18:46  pekangas
 * Added preliminary support for interpolating mixing (mono only at the
 * moment)
 *
 * Revision 1.11  1997/05/30 18:25:27  pekangas
 * Completely new mixing routines
 *
 * Revision 1.10  1997/04/07 21:07:50  pekangas
 * Added the ability to pause/resume streams.
 * Added functions to query the number of stream bytes buffered
 *
 * Revision 1.9  1997/03/05 19:35:49  pekangas
 * Added origSampleType fields to dsmChannel and dsmSample. These are needed
 * by sample autoconversion
 *
 * Revision 1.8  1997/02/20 19:50:02  pekangas
 * Added u-law volume table
 *
 * Revision 1.7  1997/01/16 18:41:59  pekangas
 * Changed copyright messages to Housemarque
 *
 * Revision 1.6  1997/01/16 18:19:10  pekangas
 * Added support for setting the stream write position.
 * Stream data is no longer played past the write position
 *
 * Revision 1.5  1996/07/13 18:13:25  pekangas
 * Fixed to compile with Visual C
 *
 * Revision 1.4  1996/06/26 19:15:03  pekangas
 * Added sample loop callbacks
 *
 * Revision 1.3  1996/05/28 20:29:56  pekangas
 * Changed mixing routine prototypes and added new ones
 *
 * Revision 1.2  1996/05/26 20:55:20  pekangas
 * Added dsmStartStream and dsmStopStream
 *
 * Revision 1.1  1996/05/22 20:49:33  pekangas
 * Initial revision
 *
*/
