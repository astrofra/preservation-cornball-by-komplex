/*      oss.c
 *
 * Open Sound System Sound Device
 *
 * $Id: oss.c,v 1.20.2.2 1998/02/26 20:52:56 pekangas Exp $
 *
 * Copyright 1996-1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include "lang.h"
#ifdef __LINUX__
#include <sys/soundcard.h>
#else
#include <sys/audio.h>
#endif

#include "mtypes.h"
#include "errors.h"
#include "sdevice.h"
#include "mmem.h"
#include "dsm.h"
#include "mglobals.h"


RCSID(char const *oss_rcsid = "$Id: oss.c,v 1.20.2.2 1998/02/26 20:52:56 pekangas Exp $";)

#ifdef __LINUX__
#define DEVICE_NAME "/dev/dsp"
#else
#define DEVICE_NAME "/dev/audio"
#endif
    
#define open_mode O_WRONLY

static    int audio_fd;
#ifdef __LINUX__
static    int format_16bits = AFMT_S16_LE;
static    int format_8bits = AFMT_U8;
static    audio_buf_info info;
#endif
static    int format_stereo;
static    int format_mixingrate;
static    uchar *audioBuffer;
static    int audioBufferSize;

/* Make that "e" smaller (to c or even a) if you have small DMA buffer */

//static    int numFragments = 0xffff000e;

//#define DUMPBUFFER

#define OSSVERSION 1.1
#define OSSVERSTR "1.1"

/* Number of bits of accuracy in mixing for 8-bit output: */
#define MIX8BITS 12


/* Sound Device information */

    /* Sound Card names: */
#ifdef __LINUX__
static char     *ossCardName = "Open Sound System output";
#else
static char     *ossCardName = "HP-UX Audio output";
#endif


/* Sound Device internal static variables */
static unsigned mixRate, outputMode;
static unsigned mixElemSize;

static unsigned updateMix;              /* number of elements to mix between
                                           two updates */
static unsigned mixLeft;                /* number of elements to mix before
                                           next update */
static unsigned spaceLeft;              /* number of bytes of space to write */

#ifdef DUMPBUFFER
static FILE     *buff;
#endif


/****************************************************************************\
*       enum ossFunctIDs
*       -----------------
* Description:  ID numbers for OSS Sound Device functions
\****************************************************************************/

enum ossFunctIDs
{
    ID_ossDetect = ID_oss,
    ID_ossInit,
    ID_ossClose,
    ID_ossGetMode,
    ID_ossSetUpdRate,
    ID_ossStartPlay,
    ID_ossPlay
};



/* Local prototypes: */
int CALLING ossSetUpdRate(unsigned updRate);


/****************************************************************************\
*
* Function:     int ossDetect(int *result)
*
* Description:  Detects a OSS Sound Device
*
* Input:        int *result             pointer to detection result
*
* Returns:      MIDAS error code. Detection result (1 if detected, 0 if not)
*               is written to *result.
*
\****************************************************************************/

int CALLING ossDetect(int *result)
{
    /* If the output device can be opened, we have sound: */
    if (( audio_fd = open(DEVICE_NAME, open_mode, 0 )) == -1 )
	*result = 0;
    else
    {
	close(audio_fd);
	*result = 1;
    }
    return OK;
}




/****************************************************************************\
*
* Function:     int ossInit(unsigned mixRate, unsigned mode)
*
* Description:  Initializes OSS Sound Device
*
* Input:        unsigned mixRate        mixing rate in Hz
*               unsigned mode           output mode
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING ossInit(unsigned _mixRate, unsigned mode)
{
    int         error;
    int         mixMode;
    int         *modetag;
#ifdef __HPUX__
    int         *mr;
    static int mixRates[] =
    { 5512, 6615, 8000, 9600, 11025, 16000,
      18900, 22050, 27428, 32000, 44100, 48000, 0 };
#endif

    mixRate = _mixRate;

    /* Determine the actual output mode: */
    if ( mode & sdMono )
	format_stereo = 1;
    else
	format_stereo = 2;

#ifdef __LINUX__    
    if ( mode & sd8bit )
	modetag = &format_8bits;
    else
	modetag = &format_16bits;
#endif
    
    /* Open output device using the format just set up: */
    if (( audio_fd = open(DEVICE_NAME, open_mode, 0 )) == -1 )
    {
	perror(DEVICE_NAME);
	return errDeviceNotAvailable;
    }

#ifdef __LINUX__
    if ( ((int)mode = ioctl(audio_fd, SNDCTL_DSP_SETFMT, modetag)) == -1)
    {
        perror("SNDCTL_DSP_SETFMT");
	return(errSDFailure);
    }
    if (ioctl(audio_fd, SNDCTL_DSP_CHANNELS, &format_stereo) == -1)
    {
        perror("SNDCTL_DSP_CHANNELS");
	return(errSDFailure);
    }

#endif

#ifdef __HPUX__
    if ( ( ioctl(audio_fd, AUDIO_SET_DATA_FORMAT, AUDIO_FORMAT_LINEAR16BIT ) ) < 0) 
    {
       perror("AUDIO_SET_DATA_FORMAT");
       return(errSDFailure);
    }
    if (ioctl(audio_fd, AUDIO_SET_CHANNELS, format_stereo) <0)
    {
       perror("AUDIO_SET_CHANNELS");
       return(errSDFailure);
    }
#endif

    /* Check what format we got and use it */   
    if (format_stereo == 2)
        outputMode = sdStereo;
    else
        outputMode = sdMono;

    
#ifdef __LINUX__
    if ( *modetag == AFMT_U8 )
        outputMode |= sd8bit;
    else
    {
        if ( *modetag == AFMT_S16_LE )
            outputMode |= sd16bit;
        else
	    return(errSDFailure);
    }
    
    format_mixingrate = mixRate;

    if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &format_mixingrate) == -1)
    {
        perror("SNDCTL_DSP_SPEED");
	return(errSDFailure);
    }

    mixRate = format_mixingrate;
#endif
    
#ifdef __HPUX__
    outputMode |= sd16bit;

    for ( mr = mixRates; *mr ; ++mr)
    {
      if (*mr >= mixRate) { format_mixingrate = *mr; break; }
      else format_mixingrate = *mr;
    }

    if (ioctl(audio_fd, AUDIO_SET_SAMPLE_RATE,format_mixingrate)< 0)
    {
        perror("SNDCTL_DSP_SPEED");
        return(errSDFailure);
    }
    mixRate = format_mixingrate;
#endif


    /* Calculate one mixing element size: */
    if ( outputMode & sd16bit )
        mixElemSize = 2;
    else
        mixElemSize = 1;
    if ( outputMode & sdStereo )
        mixElemSize <<= 1;

    /* Check correct mixing mode: */
    if ( outputMode & sdStereo )
        mixMode = dsmMixStereo;
    else
        mixMode = dsmMixMono;

    if ( outputMode & sd16bit )
        mixMode |= dsmMix16bit;
    else
        mixMode |= dsmMix8bit;

    if ( mOversampling )
	mixMode |= dsmMixInterpolation;
	
    /* Initialize Digital Sound Mixer: */
    if ( (error = dsmInit(mixRate, mixMode)) != OK )
        PASSERROR(ID_ossInit)

    /* Set update rate to 50Hz: */
    if ( (error = ossSetUpdRate(5000)) != OK )
        PASSERROR(ID_ossInit)

#ifdef __LINUX__
    /* Set audio fragment size (not necessary)*/

#if 0		
    if (ioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &numFragments) == -1)
    { /* Fatal error */
        perror("SNDCTL_DSP_SETFRAGMENT");
	return(errSDFailure);
    }
#endif
    
    if (ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &info) == -1)
    { /* Fatal error */
        perror("SNDCTL_DSP_GETOSPACE");
	return(errSDFailure);
    }

//    printf("Fragment size: %i, total fragments: %i\n", info.fragsize, info.fragstotal);
#endif

    /* Allocate audio buffer */
	
    audioBufferSize = mixRate * mixElemSize * mBufferLength / 1000;

    if ( (error = memAlloc(audioBufferSize, (void**)&audioBuffer)) != OK )
        PASSERROR(ID_ossInit);

#ifdef DUMPBUFFER
    buff = fopen("buffer.raw", "wb");
#endif
    return OK;
}




/****************************************************************************\
*
* Function:     ossClose(void)
*
* Description:  Uninitializes OSS Sound Device
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int ossClose(void)
{
    int         error;

#ifdef DUMPBUFFER
    fclose(buff);
#endif

    /* Uninitialize Digital Sound Mixer: */
    if ( (error = dsmClose()) != OK )
        PASSERROR(ID_ossClose)

    /* Deallocate audio buffer */
    if ( (error = memFree(audioBuffer)) != OK )
      PASSERROR(ID_ossClose);

    close(audio_fd);

    return OK;
}




/****************************************************************************\
*
* Function:     int ossGetMode(unsigned *mode)
*
* Description:  Reads the current output mode
*
* Input:        unsigned *mode          pointer to output mode
*
* Returns:      MIDAS error code. Output mode is written to *mode.
*
\****************************************************************************/

int CALLING ossGetMode(unsigned *mode)
{
    *mode = outputMode;

    return OK;
}


/****************************************************************************\
*
* Function:     int ossSetUpdRate(unsigned updRate);
*
* Description:  Sets the channel value update rate (depends on song tempo)
*
* Input:        unsigned updRate        update rate in 100*Hz (eg. 50Hz
*                                       becomes 5000).
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING ossSetUpdRate(unsigned updRate)
{
    /* Calculate number of elements to mix between two updates: (even) */
    mixLeft = updateMix = ((unsigned) ((100L * (ulong) mixRate) /
        ((ulong) updRate)) + 1) & 0xFFFFFFFE;

    return OK;
}




/****************************************************************************\
*
* Function:     int ossStartPlay(void)
*
* Description:  Prepares for playing - checks how many bytes we can mix
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING ossStartPlay(void)
{
#ifdef __LINUX__
    if (ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &info) == -1)
    {
        perror("SNDCTL_DSP_GETOSPACE");
	return(errSDFailure);
    }
    spaceLeft = info.bytes;
#endif
    return OK;
}




/****************************************************************************\
*
* Function:     int ossPlay(int *callMP);
*
* Description:  Plays the sound - mixes the correct amount of data with DSM
*               and copies it to output buffer with post-processing.
*               Also takes care of sending fully mixed buffer to the 
*               output device.
*
* Input:        int *callMP             pointer to music player calling flag
*
* Returns:      MIDAS error code. If enough data was mixed for one updating
*               round and music player should be called, 1 is written to
*               *callMP, otherwise 0 is written there. Note that if music
*               player can be called, ossPlay() should be called again
*               with a new check for music playing to ensure the mixing buffer
*               gets filled with new data.
*
\****************************************************************************/

int CALLING ossPlay(int *callMP)
{
    int         error;
    unsigned    numElems;

    do
    {
	
    /* Calculate number of mixing elements left: */
    numElems = audioBufferSize / mixElemSize;    
	
#ifdef __LINUX__
    if ( numElems > spaceLeft / mixElemSize )
        numElems = spaceLeft / mixElemSize;
#endif
    
    /* Check that we won't mix more data than there is to the next
       update: */
    if ( numElems > mixLeft )
        numElems = mixLeft;

    /* Decrease number of elements before next update: */
    mixLeft -= numElems;

    /* Mix the data: */
    if ( (error = dsmMixData(numElems, audioBuffer)) != OK )
        PASSERROR(ID_ossPlay)
    
#ifdef DUMPBUFFER
    fwrite(&audioBuffer[oldPos], numElems * mixElemSize, 1, buff);
#endif


    /* Write the mixed data to the output device: */

    write(audio_fd, audioBuffer, numElems * mixElemSize);

    spaceLeft -= numElems * mixElemSize;
    
    /* Check if the music player should be called: */
    if ( mixLeft == 0)
    {
        mixLeft = updateMix;
        *callMP = 1;
        return OK;
    }

    }while(spaceLeft);
	
	    
    /* No more data fits to the mixing buffers - just return without
       update: */
    *callMP = 0;

    return OK;
}




    /* OSS Sound Device structure: */

SoundDevice     OSS = {
    0,                                  /* tempoPoll = 0 */
    sdUseMixRate | sdUseOutputMode | sdUseDSM,  /* configBits */
    0,                                  /* port */
    0,                                  /* IRQ */
    0,                                  /* DMA */
    1,                                  /* cardType */
    1,                                  /* numCardTypes */
    sdMono | sdStereo | sd8bit | sd16bit,       /* modes */

    "Open Sound System Sound Device " OSSVERSTR,              /* name */
    &ossCardName,                               /* cardNames */
    0,                                          /* numPortAddresses */
    NULL,                                       /* portAddresses */

    &ossDetect,
    &ossInit,
    &ossClose,
    &dsmGetMixRate,
    &ossGetMode,
    &dsmOpenChannels,
    &dsmCloseChannels,
    &dsmClearChannels,
    &dsmMute,
    &dsmPause,
    &dsmSetMasterVolume,
    &dsmGetMasterVolume,
    &dsmSetAmplification,
    &dsmGetAmplification,
    &dsmPlaySound,
    &dsmReleaseSound,
    &dsmStopSound,
    &dsmSetRate,
    &dsmGetRate,
    &dsmSetVolume,
    &dsmGetVolume,
    &dsmSetSample,
    &dsmGetSample,
    &dsmSetPosition,
    &dsmGetPosition,
    &dsmGetDirection,
    &dsmSetPanning,
    &dsmGetPanning,
    &dsmMuteChannel,
    &dsmAddSample,
    &dsmRemoveSample,
    &ossSetUpdRate,
    &ossStartPlay,
    &ossPlay
#ifdef SUPPORTSTREAMS
    ,
    &dsmStartStream,
    &dsmStopStream,
    &dsmSetLoopCallback,
    &dsmSetStreamWritePosition,
    &dsmPauseStream
#endif    
};


/*
 * $Log: oss.c,v $
 * Revision 1.20.2.2  1998/02/26 20:52:56  pekangas
 * Added stream support
 *
 * Revision 1.20.2.1  1997/08/21 22:30:35  pekangas
 * Added proper detection(?)
 *
 * Revision 1.20  1997/08/01 18:44:05  jpaana
 * Fixed 8-bit output and cleaned up the code a bit
 *
 * Revision 1.19  1997/07/31 10:56:54  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.18  1997/06/22 21:10:12  jpaana
 * Removed unnecessary ossOpenChannels and fixed HP-UX support
 *
 * Revision 1.17  1997/06/05 22:42:11  jpaana
 * Cleaned up and added support for interpolation
 *
 * Revision 1.16  1997/06/02 11:20:36  jpaana
 * A pretty basic fix (now doesn't block under any conditions) and included
 * HP-UX support :)
 *
 * Revision 1.15  1997/05/22 19:44:54  jpaana
 * Made fragment size setting not to cause error anymore
 *
 * Revision 1.14  1997/05/03 22:18:14  jpaana
 * Fixed a warning.
 *
 * Revision 1.13  1997/04/08 15:48:08  pekangas
 * Fixed a signed/unsigned comparison warning
 *
 * Revision 1.12  1997/03/17 17:36:47  jpaana
 * Slight changes to get more non-blocking polling
 *
 * Revision 1.11  1996/11/09 21:08:04  jpaana
 * Fixed some "comparison between signed and unsigned" warnings
 *
 * Revision 1.10  1996/09/22 17:11:56  jpaana
 * Still tweaking...
 *
 * Revision 1.9  1996/09/21 17:18:01  jpaana
 * Misc Fixes
 *
 * Revision 1.8  1996/09/21 16:40:26  jpaana
 * Fixed some typos
 *
 * Revision 1.7  1996/09/21 16:38:00  jpaana
 * Renamed to Open Sound System Sound Device (blah)
 *
 * Revision 1.6  1996/09/15 09:18:28  jpaana
 * Removed some debug texts
 *
 * Revision 1.5  1996/09/09 09:52:12  jpaana
 * Added some more fragments
 *
 * Revision 1.4  1996/09/08 20:32:32  jpaana
 * Misc. tweaking (most commented out now)
 *
 * Revision 1.3  1996/08/03 13:16:42  jpaana
 * Fixed to work without Pthreads ;)
 *
 * Revision 1.1  1996/06/05 19:40:35  jpaana
 * Initial revision
 *
 * Revision 1.2  1996/05/25 15:49:57  jpaana
 * Cleaned up
 *
 * Revision 1.1  1996/05/24 20:40:12  jpaana
 * Initial revision
 *
 *
*/
