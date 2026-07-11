/*      midasstr.c
 *
 * MIDAS stream library
 *
 * $Id: midasstr.c,v 1.21 1997/07/31 10:56:49 pekangas Exp $
 *
 * Copyright 1996,1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include "lang.h"

#if defined(__UNIX__)
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#else
#if defined(__NT__) || defined(__WINDOWS__) || defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#endif
#include <string.h>                     /* FIXME - don't use C RTL */
#endif

#include "mtypes.h"
#include "errors.h"
#include "mmem.h"
#include "sdevice.h"
#include "file.h"
#include "madpcm.h"
#include "waveread.h"
#include "midasstr.h"



/* Maximum number of streams: (just for keeping track of all streams in
   playback) */
#define MAXSTREAMS 32
/* 32 streams will probably bring any system to its knees, but we'll just
   be safe */

static strStream *streams[MAXSTREAMS];  /* all streams currently playing */
static SoundDevice *SD;                 /* Sound Device for streams */



/****************************************************************************\
*       enum strFunctIDs
*       ----------------
* Description:  Function IDs for stream library functions
\****************************************************************************/

enum strFunctIDs
{
    ID_strInit = ID_str,
    ID_strClose,
    ID_strPlayStreamFile,
    ID_strPlayStreamPolling,
    ID_strPlayStreamCallback,
    ID_strStopStream,
    ID_strFeedStreamData,
    ID_strSetStreamVolume,
    ID_strSetStreamPanning,
    ID_strIsStreamFinished,
    ID_strSetStreamRate,
    ID_strPauseStream,
    ID_strPlayStreamFileSection,
    ID_strPlayStreamWaveFile
};




/****************************************************************************\
*
* Function:     int strInit(SoundDevice *SD)
*
* Description:  Initializes the stream library
*
* Input:        SoundDevice *SD         Pointer to the Sound Device that will
*                                       be used for playing the streams
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING strInit(SoundDevice *_SD)
{
    int         i;

    /* Remember the Sound Device: */
    SD = _SD;

    /* No streams are being played: */
    for ( i = 0; i < MAXSTREAMS; i++ )
        streams[i] = NULL;

    return OK;
}




/****************************************************************************\
*
* Function:     int strClose(void)
*
* Description:  Uninitializes the stream library
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING strClose(void)
{
    int         i, error;

    /* Check if there are any streams being played - if yes, stop them: */
    for ( i = 0; i < MAXSTREAMS; i++ )
    {
        if ( streams[i] != NULL )
        {
            if ( (error = strStopStream(streams[i])) != OK )
                PASSERROR(ID_strClose)
        }
    }

    return OK;
}



/****************************************************************************\
*
* Function:     static int StartStream(strStream **stream)
*
* Description:  Starts a new stream, and allocates the info structure.
*
* Input:        strStream **stream      pointer to steam state pointer
*
* Returns:      MIDAS error code
*
\****************************************************************************/

static int StartStream(strStream **stream)
{
    int         error;
    int         num;

    /* Find a free spot from the stream table: */
    num = 0;
    while ( streams[num] != NULL )
    {
        num++;
        if ( num >= MAXSTREAMS )
            return errOutOfResources;
    }

    /* Allocate memory for the stream state structure: */
    if ( (error = memAlloc(sizeof(strStream), (void**) stream)) != OK )
        return error;

    streams[num] = *stream;

    return OK;
}




/****************************************************************************\
*
* Function:     static int FreeStream(strStream *stream)
*
* Description:  Deallocates a steram and removes it from the stream list
*
* Input:        strStream *stream       stream to be deallocated
*
* Returns:      MIDAS error code
*
\****************************************************************************/

static int FreeStream(strStream *stream)
{
    int         error;
    int         i;

    /* Remove the stream from the stream list: */
    for ( i = 0; i < MAXSTREAMS; i++ )
    {
        if ( streams[i] == stream )
            streams[i] = NULL;
    }

    /* Deallocate the stream structure: */
    if ( (error = memFree(stream)) != OK )
        return error;

    return OK;
}



/****************************************************************************\
*
* Function:     unsigned StreamBufferLeft(strStream *stream)
*
* Description:  Calculates the number of bytes of free space in a stream
*               buffer
*
* Input:        strStream *stream       stream to check
*
* Returns:      Number of bytes of free space left
*
\****************************************************************************/

static unsigned StreamBufferLeft(strStream *stream)
{
    static unsigned readPos;
    unsigned    spaceLeft;

    /* FIXME - error callback? */

    /* Get reading position: */
    SD->GetPosition(stream->sdChannel, &readPos);

    /* If read and write positions are equal, the whole buffer is empty: */
    if ( readPos == stream->bufferWritePos )
        return stream->bufferBytes - stream->sampleSize;

    /* Calculate the amount of free space: */
    if ( readPos >= stream->bufferWritePos )
        spaceLeft = readPos - stream->bufferWritePos;
    else
        spaceLeft = stream->bufferBytes - stream->bufferWritePos + readPos;

    /* Make sure that we won't wrap around: */
    if ( spaceLeft >= stream->sampleSize )
        spaceLeft -= stream->sampleSize;
    else
        spaceLeft = 0;

    return spaceLeft;
}




/****************************************************************************\
*
* Function:     void WriteStreamData(strStream *stream,
*                   unsigned spaceLeft, uchar *data, unsigned dataBytes,
*                   unsigned *dataUsed, unsigned *outputWritten)
*
* Description:  Writes stream data to the stream buffer, updating buffer write
*               position and taking care of wraparound
*
* Input:        strStream *stream       stream for the data
*               unsigned spaceLeft      number of bytes of space left in the
*                                       stream buffer
*               uchar *data             pointer to data
*               unsigned dataBytes      number of bytes of data available
*               unsigned *dataUsed      pointer to number of data bytes used
*               unsigned *outputWritten pointer to number of output bytes
*                                       written
*
* Returns:      The number of data bytes used is written to *dataUsed, and the
*               number of output bytes written is written to *outputWritten.
*
\****************************************************************************/

static void WriteStreamData(strStream *stream, unsigned spaceLeft,
    uchar *data, unsigned dataBytes, unsigned *dataUsed,
    unsigned *outputWritten)
{
    unsigned    len = stream->bufferBytes;
    unsigned    pos = stream->bufferWritePos;
    unsigned    left, now, startPos;
    static unsigned inputUsed, outputWr;

    *dataUsed = 0;
    *outputWritten = 0;

    /* Loop until we get everything copied: */
    while ( (spaceLeft > 0) && (dataBytes > 0) )
    {
        /* Get number of bytes of space before buffer end: */
        left = len - pos;

        /* Don't copy past buffer end: */
        if ( spaceLeft > left )
            now = left;
        else
            now = spaceLeft;

        startPos = pos;

        /* Copy/decode the data: */
        if ( stream->adpcm )
        {
            mAdpcmDecode(data, dataBytes, (short*) &stream->buffer[pos],
                now, &inputUsed, &outputWr, stream->adpcmState);

            data += inputUsed;
            *dataUsed += inputUsed;
            dataBytes -= inputUsed;

            pos += outputWr;
            spaceLeft -= outputWr;
            *outputWritten += outputWr;
        }
        else
        {
            if ( dataBytes < now )
                now = dataBytes;

            memcpy(&stream->buffer[pos], data, now);
            data += now;
            *dataUsed += now;
            dataBytes -= now;
            pos += now;
            spaceLeft -= now;
            *outputWritten += now;
        }

        /* Wrap to buffer beginning if necessary: */
        if ( pos >= len )
            pos = 0;
    }

    /* Remember the new buffer position: */
    stream->bufferWritePos = pos;
    /* FIXME: error check: */
    SD->SetStreamWritePosition(stream->sdChannel, pos);
}



#ifdef M_HAVE_THREADS



/****************************************************************************\
*
* Function:     void FillStreamBuffer(strStream *stream, uchar value,
*                   unsigned numBytes)
*
* Description:  Fills a stream buffer with a byte, updating buffer write
*               position and taking care of wraparound
*
* Input:        strStream *stream       stream for the data
*               uchar value             byte to fill the buffer with
*               unsigned numBytes       number of bytes to fill
*
\****************************************************************************/

static void FillStreamBuffer(strStream *stream, uchar value,
    unsigned numBytes)
{
    unsigned    len = stream->bufferBytes;
    unsigned    pos = stream->bufferWritePos;
    unsigned    left, now;

    /* Loop until we get everything filled */
    while ( numBytes )
    {
        /* Get number of bytes of space before buffer end: */
        left = len - pos;

        /* Don't fill past buffer end: */
        if ( numBytes > left )
            now = left;
        else
            now = numBytes;

        /* Fill it: */
        memset(&stream->buffer[pos], value, now);

        /* Advance buffer position: */
        pos += now;
        numBytes -= now;

        /* Wrap to buffer beginning if necessary: */
        if ( pos >= len )
            pos = 0;
    }

    /* Remember the new buffer position: */
    stream->bufferWritePos = pos;
    /* FIXME: error check: */
    SD->SetStreamWritePosition(stream->sdChannel, pos);
}




/****************************************************************************\
*
* Function:     void StreamPlayerThread(void *stream)
*
* Description:  The stream player thread
*
* Input:        void *stream            stream state information for this
*                                       stream
*
\****************************************************************************/

#ifdef __UNIX__
static void* StreamPlayerThread(void *stream)
#else
#ifdef __WC32__
static void StreamPlayerThread(void *stream)
#else
static void __cdecl StreamPlayerThread(void *stream)
#endif
#endif
{
    /* FIXME - we should check for errors, but for that we need some way to
       report the errors to the user - a callback maybe? */

    unsigned    bufferLeft;
    unsigned    doNow;
    unsigned    dataLeft = 0;
    unsigned    dataUsed, outputWritten;
    uchar       *data = NULL;
    strStream   *s = (strStream*) stream;

    /* Round and round we go, until it's exit time */

    while ( !s->threadExitFlag )
    {
        /* Get amount of free space in buffer: */
        bufferLeft = StreamBufferLeft(s);

        /* Make sure it fits and integral number of ADPCM sample pairs: */
        bufferLeft &= ~3;
        /* (plus alignment never hurts anyway) */

        /* Fill the free space - either read data from the file or just
           clear it: */
        while ( bufferLeft )
        {
            if ( !dataLeft && (s->fileLeft) )
            {
                /* No data left in file buffer, and there is still some
                   left in the file to read - get it */

                /* Don't read past End Of File: */
                if ( s->fileBufferBytes > s->fileLeft )
                    doNow = s->fileLeft;
                else
                    doNow = s->fileBufferBytes;

                /* Fill file buffer with data: */
                fileRead(s->f, s->fileBuffer, doNow);

                data = s->fileBuffer;
                dataLeft = doNow;
                s->fileLeft -= doNow;

                /* Check if we reached the file end and should loop: */
                if ( (s->fileLeft <= 0) && (s->loop) )
                {
                    /* Restart the file: */
                    s->fileLeft = s->fileLength;
                    fileSeek(s->f, s->fileOffset, fileSeekAbsolute);
                }
            }

            if ( dataLeft )
            {
                /* Write the data to the stream buffer: */
                WriteStreamData(s, bufferLeft, data, dataLeft, &dataUsed,
                    &outputWritten);

                data += dataUsed;
                dataLeft -= dataUsed;
                bufferLeft -= outputWritten;
            }
            else
            {
                /* No data left in file, just clear the buffer: */
                FillStreamBuffer(s, s->bufferClearVal, bufferLeft);
                bufferLeft = 0;
            }
        }

        /* Sleep for a while: */
#ifdef __UNIX__
        usleep(s->threadDelay * 1000);
#else
        Sleep(s->threadDelay);
#endif
    }

    /* Exit flag detected: */
    s->threadExitFlag = 0;

    /* Get lost: */
#ifdef __UNIX__
    pthread_exit(0);
    return NULL;
#else
    _endthread();
#endif
}




/****************************************************************************\
*
* Function:     int strPlayStreamFileSection(strStream *stream,
*                   unsigned channel, fileHandle f,
*                   unsigned startOffset, unsigned numBytes,
*                   unsigned sampleType, ulong sampleRate,
*                   unsigned bufferLength, int loop)
*
* Description:  Starts playing a digital sound stream from a file. Creates a
*               new thread that will take care of reading the file and feeding
*               it to the stream buffer.
*
* Input:        strStream *stream       stream state structure
*               fileHandle f            file handle for the stream file
*               unsigned startOffset    stream data start offset in file
*               unsigned numBytes       number of bytes of data in file
*               unsigned sampleType     stream sample type
*               ulong sampleRate        sampling rate
*               unsigned bufferLength   stream buffer length in milliseconds
*               int loop                1 if the stream should be looped,
*                                       0 if not
*
* Returns:      MIDAS error code.
*
\****************************************************************************/

int CALLING strPlayStreamFileSection(strStream *stream, unsigned channel,
                                     fileHandle f, unsigned startOffset,
                                     unsigned numBytes,  unsigned sampleType,
                                     ulong sampleRate, unsigned bufferLength,
                                     int loop)
{
    int         error;
    strStream   *s;
#ifdef __WIN32__
#ifdef __WC32__
    int         streamThread;
#else
    ulong       streamThread;
#endif
#endif
#ifdef __UNIX__
    int         code;
#endif

    /* Stop any sound on the channel: */
    if ( (error = SD->StopSound(channel)) != OK )
        PASSERROR(ID_strPlayStreamFileSection);

    s = stream;

    /* Initialize the stream structure: */
    s->sdChannel = channel;
    s->streamMode = strStreamPlayFile;
    s->loop = loop;
    s->threadExitFlag = 0;
    s->threadDelay = bufferLength / 8;
    s->callback = NULL;
    s->adpcmState = NULL;
    s->adpcm = 0;
    s->buffer = NULL;
    s->fileBuffer = NULL;
    s->f = NULL;

    /* Check the sample type and initialize accordingly: */
    switch ( sampleType )
    {
        case smp8bitMono:
        case smpUlawMono:
            s->sampleSize = 1;
            s->bufferClearVal = 128;
            break;

        case smp16bitMono:
            s->sampleSize = 2;
            s->bufferClearVal = 0;
            break;

        case smp8bitStereo:
        case smpUlawStereo:
            s->sampleSize = 2;
            s->bufferClearVal = 128;
            break;

        case smp16bitStereo:
            s->sampleSize = 4;
            s->bufferClearVal = 0;
            break;

        case smpAdpcmMono:
            s->sampleSize = 2;          /* buffer sample size */
            sampleType = smp16bitMono;  /* buffer sample type */
            s->bufferClearVal = 0;
            if ( (error = mAdpcmInitState(1, 0, &s->adpcmState)) != OK )
                PASSERROR(ID_strPlayStreamFileSection);
            s->adpcm = 1;
            break;

        case smpAdpcmStereo:
            s->sampleSize = 4;          /* buffer sample size */
            sampleType = smp16bitStereo; /* buffer sample type */
            s->bufferClearVal = 0;
            if ( (error = mAdpcmInitState(2, 0, &s->adpcmState)) != OK )
                PASSERROR(ID_strPlayStreamFileSection);
            s->adpcm = 1;
            break;

        default:
            ERROR(errInvalidSampleType, ID_strPlayStreamFileSection);
            return errInvalidSampleType;
    }

    /* Calculate the buffer size in samples: (make it a multiple of 4) */
    s->bufferSamples = ((bufferLength * sampleRate / 1000) + 3) & (~3);

    /* And in bytes: */
    s->bufferBytes = s->sampleSize * s->bufferSamples;

    /* Allocate the buffer (with space for one extra sample - needed as
       interpolation food): */
    if ( (error = memAlloc(s->bufferBytes + s->sampleSize,
                           (void**) &s->buffer)) != OK )
        PASSERROR(ID_strPlayStreamFileSection);

    /* Clear it: */
    s->bufferWritePos = 0;
    memset(s->buffer, s->bufferClearVal, s->bufferBytes);

    /* Allocate a file reading buffer: */
    if ( s->adpcm )
        s->fileBufferBytes = s->bufferBytes / 4;
    else
        s->fileBufferBytes = s->bufferBytes;
    if ( (error = memAlloc(s->fileBufferBytes, (void**) &s->fileBuffer))
        != OK )
    {
        memFree(s->buffer);
        PASSERROR(ID_strPlayStreamFileSection);
    }

    s->f = f;
    s->fileLength = s->fileLeft = numBytes;
    s->fileOffset = startOffset;

    /* Start playing the stream: */
    if ( ((error = SD->StartStream(s->sdChannel, s->buffer, s->bufferBytes,
                                   sampleType, sampleRate)) != OK) ||
         ((error = SD->SetVolume(s->sdChannel, 64)) != OK ) )
    {
        memFree(s->fileBuffer);
        memFree(s->buffer);
        PASSERROR(ID_strPlayStreamFileSection);
    }

    /* Start the stream player thread: */

#ifdef __WIN32__
#ifdef __WC32__
    streamThread = _beginthread(StreamPlayerThread, NULL, 8192, (void*) s);

    if ( streamThread == -1 )
    {
        /* Couldn't create thread */
        SD->StopStream(s->sdChannel);
        memFree(s->fileBuffer);
        memFree(s->buffer);
        ERROR(errOutOfResources, ID_strPlayStreamFileSection);
        return errOutOfResources;
    }
#else
    streamThread = _beginthread(StreamPlayerThread, 4096, (void*) s);

    if ( streamThread == -1 )
    {
        /* Couldn't create thread */
        SD->StopStream(s->sdChannel);
        memFree(s->fileBuffer);
        memFree(s->buffer);
        ERROR(errOutOfResources, ID_strPlayStreamFileSection);
        return errOutOfResources;
    }
#endif
#else
    code = pthread_create(&s->playerThread, NULL, StreamPlayerThread,
        (void*) s);
    if ( code )
    {
        /* Couldn't create thread */
        SD->StopStream(s->sdChannel);
        memFree(s->fileBuffer);
        memFree(s->buffer);
        ERROR(errOutOfResources, ID_strPlayStreamFileSection);
        return errOutOfResources;
    }
#endif
    /* The thread is playing OK */

    return OK;
}



/****************************************************************************\
*
* Function:     int strPlayStreamFile(unsigned channel, char *fileName,
*                   unsigned sampleType, ulong sampleRate,
*                   unsigned bufferLength, int loop, strStream **stream)
*
* Description:  Starts playing a digital sound stream from a file. Creates a
*               new thread that will take care of reading the file and feeding
*               it to the stream buffer
*
* Input:        unsigned channel        channel number for the stream
*               char *fileName          stream file name
*               unsigned sampleType     stream sample type
*               ulong sampleRate        sampling rate
*               unsigned bufferLength   stream buffer length in milliseconds
*               int loop                1 if the stream should be looped,
*                                       0 if not
*               strStream **stream      pointer to stream state pointer
*
* Returns:      MIDAS error code. Pointer to the stream state structure will
*               be written to *stream
*
\****************************************************************************/

int CALLING strPlayStreamFile(unsigned channel, char *fileName,
    unsigned sampleType, ulong sampleRate, unsigned bufferLength, int loop,
    strStream **stream)
{
    int error;
    strStream *s;
    fileHandle f;
    long fileLen;
    
    /* Allocate a stream state structure for the new stream: */
    if ( (error = StartStream(stream)) != OK )
        PASSERROR(ID_strPlayStreamFile);
    s = *stream;
    s->waveState = NULL;

    /* Open the stream file and get its length: */
    if ( (error = fileOpen(fileName, fileOpenRead, &f)) != OK )
    {
        FreeStream(s);
        PASSERROR(ID_strPlayStreamFile);
    }

    if ( (error = fileGetSize(f, &fileLen)) != OK )
    {
        fileClose(f);
        FreeStream(s);
        PASSERROR(ID_strPlayStreamFile);
    }

    if ( (error = strPlayStreamFileSection(s, channel, f, 0, fileLen,
                                           sampleType, sampleRate,
                                           bufferLength, loop)) != OK )
    {
        fileClose(f);
        FreeStream(s);
        PASSERROR(ID_strPlayStreamFile);
    }

    return OK;
}



/****************************************************************************\
*
* Function:     int strPlayStreamFile(unsigned channel, char *fileName,
*                   unsigned sampleType, ulong sampleRate,
*                   unsigned bufferLength, int loop, strStream **stream)
*
* Description:  Starts playing a digital sound stream from a RIFF WAVE file.
*               Creates a new thread that will take care of reading the file
*               and feeding it to the stream buffer
*
* Input:        unsigned channel        channel number for the stream
*               char *fileName          stream file name
*               unsigned bufferLength   stream buffer length in milliseconds
*               int loop                1 if the stream should be looped,
*                                       0 if not
*               strStream **stream      pointer to stream state pointer
*
* Returns:      MIDAS error code. Pointer to the stream state structure will
*               be written to *stream
*
\****************************************************************************/

int CALLING strPlayStreamWaveFile(unsigned channel, char *fileName,
                                  unsigned bufferLength, int loop,
                                  strStream **stream)
{
    int error;
    strStream *s;
    mWaveState *waveState;
    
    /* Allocate a stream state structure for the new stream: */
    if ( (error = StartStream(stream)) != OK )
        PASSERROR(ID_strPlayStreamFile);
    s = *stream;
    s->waveState = NULL;

    /* Open the wave file: */
    if ( (error = mWaveOpen(fileName, &waveState)) != OK )
    {
        FreeStream(s);
        PASSERROR(ID_strPlayStreamFile);
    }

    s->waveState = waveState;

    if ( (error = strPlayStreamFileSection(s, channel, waveState->file,
                                           waveState->dataStart,
                                           waveState->dataLength,
                                           waveState->sampleType,
                                           waveState->sampleRate,
                                           bufferLength, loop)) != OK )
    {
        mWaveClose(waveState);
        FreeStream(s);
        PASSERROR(ID_strPlayStreamWaveFile);
    }

    return OK;
}



#endif /* #ifdef M_HAVE_THREADS */




/****************************************************************************\
*
* Function:     int strStopStream(strStream *stream)
*
* Description:  Stops playing a stream. This function will also destroy the
*               playback thread for stream file playback.
*
* Input:        strStream *stream       stream state pointer for the stream
*
* Returns:      MIDAS error code.
*
\****************************************************************************/

int CALLING strStopStream(strStream *stream)
{
#ifdef __UNIX__
    void        *retval;
#endif
    int         error;

#ifdef M_HAVE_THREADS    
    /* Stop the player thread if we are playing from a file: */
    if ( stream->streamMode == strStreamPlayFile )
    {
        /* Signal the stream player thread that it should actually stop: */
        stream->threadExitFlag = 1;

        /* Wait until it stops: */
#ifdef __UNIX__
        pthread_join(stream->playerThread, &retval);
#else
        while ( stream->threadExitFlag )
            Sleep(stream->threadDelay);
#endif
        /* [Now is that ugly or what? But it works and it's portable] */
    }
#endif    

    /* Stop the Sound Device playing the stream: */
    if ( (error = SD->StopStream(stream->sdChannel)) != OK )
    {
        /* Failed - let's try to do at least some cleanup anyway: */
        if ( stream->streamMode == strStreamPlayFile )
        {
            fileClose(stream->f);
            memFree(stream->fileBuffer);
        }
        memFree(stream->buffer);
        FreeStream(stream);
        PASSERROR(ID_strStopStream);
    }

    /* Close the file and deallocate file buffer only if we were playing a
       stream file: */
    if ( stream->streamMode == strStreamPlayFile )
    {
        if ( stream->waveState != NULL )
        {
            /* Close the wave file: */
            if ( (error = mWaveClose(stream->waveState)) != OK )
            {
                memFree(stream->fileBuffer);
                memFree(stream->buffer);
                FreeStream(stream);
                PASSERROR(ID_strStopStream);
            }
        }
        else
        {
            /* Close the stream file: */
            if ( stream->f != NULL )
            {
                if ( (error = fileClose(stream->f)) != OK )
                {
                    memFree(stream->fileBuffer);
                    memFree(stream->buffer);
                    FreeStream(stream);
                    PASSERROR(ID_strStopStream);
                }
            }
        }

        /* Deallocate stream file buffer: */
        if ( stream->fileBuffer != NULL )
        {
            if ( (error = memFree(stream->fileBuffer)) != OK )
            {
                memFree(stream->buffer);
                FreeStream(stream);
                PASSERROR(ID_strStopStream);
            }
        }
    }

    /* Deallocate stream buffer: */
    if ( stream->buffer != NULL )
    {
        if ( (error = memFree(stream->buffer)) != OK )
        {
            FreeStream(stream);
            PASSERROR(ID_strStopStream);
        }
    }

    /* Deallocate ADPCM state structure if we have one: */
    if ( (stream->adpcm) && (stream->adpcmState != NULL) )
    {
        if ( (error = mAdpcmFreeState(stream->adpcmState)) != OK )
        {
            FreeStream(stream);
            PASSERROR(ID_strStopStream);
        }
    }

    /* Finally, free the stream: */
    if ( (error = FreeStream(stream)) != OK )
        PASSERROR(ID_strStopStream);

    /* Phew, done: */
    return OK;
}




/****************************************************************************\
*
* Function:     int strPlayStreamPolling(unsigned channel,
*                   unsigned sampleType, ulong sampleRate,
*                   unsigned bufferLength, strStream **stream)
*
* Description:  Starts playing a stream in polling mode. Use
*               strFeedStreamData() to feed the stream data to the player
*
* Input:        unsigned channel        channel number for the stream
*               unsigned sampleType     stream sample type
*               ulong sampleRate        stream sampling rate
*               unsigned bufferLength   stream buffer length in milliseconds
*               strStream **stream      pointer to stream state pointer
*
* Returns:      MIDAS error code. Pointer to the stream state structure will
*               be written to *stream
*
\****************************************************************************/

int CALLING strPlayStreamPolling(unsigned channel, unsigned sampleType,
    ulong sampleRate, unsigned bufferLength, strStream **stream)
{
    int         error;
    strStream   *s;

    /* Stop any sound on the channel: */
    if ( (error = SD->StopSound(channel)) != OK )
        PASSERROR(ID_strPlayStreamPolling);

    /* Allocate a stream state structure for the new stream: */
    if ( (error = StartStream(stream)) != OK )
        PASSERROR(ID_strPlayStreamPolling);
    s = *stream;

    /* Initialize the stream structure: */
    s->sdChannel = channel;
    s->streamMode = strStreamPoll;
    s->loop = 1;
    s->threadExitFlag = 0;
    s->threadDelay = bufferLength / 8;
    s->callback = NULL;
    s->adpcmState = NULL;
    s->adpcm = 0;
    s->buffer = NULL;
    s->fileBuffer = NULL;

    /* Check the sample type and initialize accordingly: */
    switch ( sampleType )
    {
        case smp8bitMono:
        case smpUlawMono:
            s->sampleSize = 1;
            s->bufferClearVal = 128;
            break;

        case smp16bitMono:
            s->sampleSize = 2;
            s->bufferClearVal = 0;
            break;

        case smp8bitStereo:
        case smpUlawStereo:
            s->sampleSize = 2;
            s->bufferClearVal = 128;
            break;

        case smp16bitStereo:
            s->sampleSize = 4;
            s->bufferClearVal = 0;
            break;

        case smpAdpcmMono:
            s->sampleSize = 2;          /* buffer sample size */
            sampleType = smp16bitMono;  /* buffer sample type */
            s->bufferClearVal = 0;
            if ( (error = mAdpcmInitState(1, 0, &s->adpcmState)) != OK )
                PASSERROR(ID_strPlayStreamPolling);
            s->adpcm = 1;
            break;

        case smpAdpcmStereo:
            s->sampleSize = 4;          /* buffer sample size */
            sampleType = smp16bitStereo; /* buffer sample type */
            s->bufferClearVal = 0;
            if ( (error = mAdpcmInitState(2, 0, &s->adpcmState)) != OK )
                PASSERROR(ID_strPlayStreamPolling);
            s->adpcm = 1;
            break;

        default:
            ERROR(errInvalidSampleType, ID_strPlayStreamPolling);
            return errInvalidSampleType;
    }

    /* Calculate the buffer size in samples: (make it a multiple of 4) */
    s->bufferSamples = ((bufferLength * sampleRate / 1000) + 3) & (~3);

    /* And in bytes: */
    s->bufferBytes = s->sampleSize * s->bufferSamples;

    /* Allocate the buffer: */
    if ( (error = memAlloc(s->bufferBytes, (void**) &s->buffer)) != OK )
    {
        FreeStream(s);
        PASSERROR(ID_strPlayStreamPolling);
    }

    /* Clear it: */
    s->bufferWritePos = 0;
    memset(s->buffer, s->bufferClearVal, s->bufferBytes);

    /* Start playing the stream: */
    if ( ((error = SD->StartStream(s->sdChannel, s->buffer, s->bufferBytes,
                                   sampleType, sampleRate)) != OK) ||
         ((error = SD->SetVolume(s->sdChannel, 64)) != OK ) )
    {
        memFree(s->buffer);
        FreeStream(s);
        PASSERROR(ID_strPlayStreamPolling);
    }

    return OK;
}



/****************************************************************************\
*
* Function:     int strPlayStreamCallback(unsigned sampleType,
*                  ulong sampleRate, unsigned bufferBytes,
*                  void (CALLING *callback)(uchar *buffer, strStream *stream))
*
* Description:  Starts playing a stream with a callback.
*
* Input:        unsigned sampleType     stream sample type
*               ulong sampleRate        stream sampling rate
*               unsigned bufferBytes    stream buffer size _IN BYTES_
*               ... *callback           stream player callback
*               strStream **stream      pointer to stream state pointer
*
* Returns:      MIDAS error code. Pointer to the stream state structure will
*               be written to *stream
*
* Notes:        The callback function will be called each time the whole
*               stream buffer needs to be filled. It receives as an argument
*               a pointer to the buffer, and the stream state pointer.
*
*               The function will be called from inside the mixing routine,
*               so it should return relatively rapidly - do not use this
*               function for, for example, loading data from disc.
*
\****************************************************************************/

int CALLING strPlayStreamCallback(unsigned sampleType, ulong sampleRate,
    unsigned bufferBytes,
    void (CALLING *callback)(uchar *buffer, strStream *stream));




/****************************************************************************\
*
* Function:     int strFeedStreamData(strStream *stream, uchar *data,
*                   unsigned numBytes, int feedAll, unsigned *numFed)
*
* Description:  Feeds sample data to a stream that is being played in polling
*               mode.
*
* Input:        strStream *stream       stream state pointer from
*                                       strPlayStreamPolling()
*               uchar *data             pointer to stream data
*               unsigned numBytes       number of bytes of data to feed. Note!
*                                       This must be a multiple of the stream
*                                       sample size
*               int feedAll             1 if all data should be fed in all
*                                       circumstances. The function will block
*                                       the current thread if this flag is 1
*                                       until all data is fed.
*               unsigned *numFed        pointer to a variable that will
*                                       contain the number of bytes actually
*                                       fed
*
* Returns:      MIDAS error code. The number of bytes of data actually fed is
*               written to *numFed.
*
\****************************************************************************/

int CALLING strFeedStreamData(strStream *stream, uchar *data,
    unsigned numBytes, int feedAll, unsigned *numFed)
{
    unsigned    bufferLeft;
    static unsigned outputWritten, dataUsed;

    *numFed = 0;

    /* Loop until all data has been fed if feedAll is 1: */
    do
    {
        /* Get amount of free space in buffer: */
        bufferLeft = StreamBufferLeft(stream);

        /* Make sure it fits and integral number of ADPCM sample pairs: */
        bufferLeft &= ~3;
        /* (plus alignment never hurts anyway) */

        /* Write the data there: */
        WriteStreamData(stream, bufferLeft, data, numBytes, &dataUsed,
            &outputWritten);

        numBytes -= dataUsed;
        data += dataUsed;
        *numFed += dataUsed;

        /* If we should feed all data, and there is data to go, sleep for
           a while: */
        if ( feedAll && numBytes )
        {
#ifdef M_HAVE_THREADS            
#ifdef __UNIX__
            usleep(stream->threadDelay * 1000);
#else
            Sleep(stream->threadDelay);
#endif
#endif            
        }
    } while ( feedAll && numBytes );

    return OK;
}



/****************************************************************************\
*
* Function:     int strSetStreamRate(strStream *stream, ulong sampleRate)
*
* Description:  Changes the sampling rate for a stream
*
* Input:        strStream *stream       stream state pointer
*               ulong sampleRate        new sampling rate in Hz
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING strSetStreamRate(strStream *stream, ulong sampleRate)
{
    int         error;

    /* Set the sample rate: */
    if ( (error = SD->SetRate(stream->sdChannel, sampleRate)) != OK )
        PASSERROR(ID_strSetStreamRate);

    return OK;
}




/****************************************************************************\
*
* Function:     int strSetStreamVolume(strStream *stream, unsigned volume)
*
* Description:  Changes the playback volume for a stream (the default is 64).
*
* Input:        strStream *stream       stream state pointer
*               unsigned volume         new volume (0-64)
*
* Returns:      MIDAS error code.
*
\****************************************************************************/

int CALLING strSetStreamVolume(strStream *stream, unsigned volume)
{
    int         error;

    /* Set the volume: */
    if ( (error = SD->SetVolume(stream->sdChannel, volume)) != OK )
        PASSERROR(ID_strSetStreamVolume);

    return OK;
}




/****************************************************************************\
*
* Function:     int strSetStreamPanning(strStream *stream, int panning)
*
* Description:  Changes the panning for a stream (the default is middle).
*
* Input:        strStream *stream       stream state pointer
*               int panning             new panning position
*
* Returns:      MIDAS error code.
*
\****************************************************************************/

int CALLING strSetStreamPanning(strStream *stream, int panning)
{
    int         error;

    /* Set the panning position: */
    if ( (error = SD->SetPanning(stream->sdChannel, panning)) != OK )
        PASSERROR(ID_strSetStreamPanning);

    return OK;
}



/****************************************************************************\
*
* Function:     int strGetStreamBytesBuffered(strStream *stream,
*                   unsigned *buffered)
*
* Description:  Get the amount of data currently stored in the stream buffer
*
* Input:        strStream *stream       stream
*               unsigned *buffered      pointer to destination variable
*
* Returns:      MIDAS error code. The amount of data (in bytes)
*               currently buffered is written to *buffered.
*
\****************************************************************************/

int CALLING strGetStreamBytesBuffered(strStream *stream, unsigned *buffered)
{
    *buffered = stream->bufferBytes - StreamBufferLeft(stream);

    return OK;
}




/****************************************************************************\
*
* Function:     int strIsStreamFinished(strStream *stream, int *finished)
*
* Description:  Checks whether a given stream has reached the end of the
*               stream file or not. Only applies to streams played from a
*               file.
*
* Input:        strStream *stream       stream state pointer
*               int *finished           pointer to result variable
*
* Returns:      MIDAS error code. If the stream is finished, 1 will be written
*               to *finished, otherwise *finished will contain 0.
*
\****************************************************************************/

int CALLING strIsStreamFinished(strStream *stream, int *finished);




/****************************************************************************\
*
* Function:     int strPauseStream(strStream *stream, int pause)
*
* Description:  Pauses/resumes a stream
*
* Input:        strStream *stream       stream to pause/resume  
*               int pause               1 to pause, 0 to resume
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING strPauseStream(strStream *stream, int pause)
{
    int error;

    if ( (error = SD->PauseStream(stream->sdChannel, pause)) != OK )
        PASSERROR(ID_strPauseStream);

    return OK;
}


/*
 * $Log: midasstr.c,v $
 * Revision 1.21  1997/07/31 10:56:49  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.20  1997/07/03 15:03:46  pekangas
 * Interpolating mixing no longer indexes past the real sample or loop end,
 * thus replicating data for it is not necessary
 *
 * Revision 1.19  1997/06/27 13:27:40  pekangas
 * Fixed to copy data to the loop end for interpolating mixing
 * Stream play functions now set the volume to 64 properly
 *
 * Revision 1.18  1997/06/02 00:54:15  jpaana
 * Changed most __LINUX__ defines to __UNIX__ for generic *nix porting
 *
 * Revision 1.17  1997/05/20 20:37:59  pekangas
 * Added WAVE support to both streams and samples
 *
 * Revision 1.16  1997/05/03 22:16:12  jpaana
 * Fixed a warning.
 *
 * Revision 1.15  1997/05/03 15:10:50  pekangas
 * Added stream support for DOS, removed GUS Sound Device from non-Lite
 * build. M_HAVE_THREADS now defined in threaded environment.
 *
 * Revision 1.14  1997/04/07 21:07:51  pekangas
 * Added the ability to pause/resume streams.
 * Added functions to query the number of stream bytes buffered
 *
 * Revision 1.13  1997/03/18 16:56:32  pekangas
 * Fixed strSetStreamVolume() and strSetStreamPanning()
 *
 * Revision 1.12  1997/02/20 19:49:05  pekangas
 * Added support for u-law streams
 *
 * Revision 1.11  1997/02/18 20:21:36  pekangas
 * New ADPCM encoding/decoding code: ADPCM streams now consist of frames,
 * and playback can be started at the beginning of any frame.
 *
 * Revision 1.10  1997/02/12 17:18:55  pekangas
 * Fixed manual feeding non-ADPCM streams (oops)
 *
 * Revision 1.9  1997/02/12 16:28:35  pekangas
 * Added support for ADPCM streams
 *
 * Revision 1.8  1997/02/07 15:15:33  pekangas
 * Fixed to compile for Linux with the new build system
 *
 * Revision 1.7  1997/01/26 23:31:44  jpaana
 * Small fixes for pthreads
 *
 * Revision 1.6  1997/01/16 19:31:53  pekangas
 * Fixed to compile with Linux GCC (but do they work?)
 *
 * Revision 1.5  1997/01/16 18:41:59  pekangas
 * Changed copyright messages to Housemarque
 *
 * Revision 1.4  1997/01/16 18:25:08  pekangas
 * Implemented strSetStreamRate, strSetStreamVolume and strSetStreamPanning
 *
*/
