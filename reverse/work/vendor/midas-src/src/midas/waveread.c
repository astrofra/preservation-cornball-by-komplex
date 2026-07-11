/*      waveread.c
 *
 * RIFF WAVE reader routines
 *
 * $Id: waveread.c,v 1.2 1997/07/31 10:56:57 pekangas Exp $
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
#include "mutils.h"
#include "mmem.h"
#include "rawfile.h"
#include "file.h"
#include "sdevice.h"
#include "waveread.h"


RCSID(const char *waveread_rcsid = "$Id: waveread.c,v 1.2 1997/07/31 10:56:57 pekangas Exp $";)


/****************************************************************************\
*
* Function:     int mWaveFindChunk(fileHandle f, const char *name,
*                   unsigned *length)
*
* Description:  Finds the next chunk with a given name from a RIFF WAVE file,
*               and reads its length.
*
* Input:        fileHandle f            file handle for the file   
*               const char *name        chunk name to look for
*               unsigned *length        pointer to chunk length destination
*
* Returns:      MIDAS error code. Chunk length in bytes (excluding the name
*               and length) is written to *length. 
*
\****************************************************************************/

int CALLING mWaveFindChunk(fileHandle f, const char *name, unsigned *length)
{
    int error;
    static mWaveChunkHeader chunk;

    do
    {
        if ( (error = fileRead(f, (void*) &chunk, sizeof(mWaveChunkHeader)))
             != OK )
            PASSERROR(ID_mWaveFindChunk);
        if ( !mMemEqual(chunk.name, name, 4) )
        {
            if ( (error = fileSeek(f, chunk.length, fileSeekRelative)) != OK )
                PASSERROR(ID_mWaveFindChunk);
        }
    } while ( !mMemEqual(chunk.name, name, 4) );

    *length = chunk.length;
    
    return OK;
}




/****************************************************************************\
*
* Function:     int mWaveOpen(char *fileName, mWaveState **state)
*
* Description:  Opens a RIFF WAVE file for reading, reads its format
*               information, finds the start and length of sample data,
*               and sets the file reading position to the start of the
*               sample data.
*
* Input:        char *fileName          name of file
*               mWaveState **state      pointer to destination state pointer
*
* Returns:      MIDAS error code. Pointer to a WAVE reader state structure
*               is written to *state.
*
\****************************************************************************/

int CALLING mWaveOpen(char *fileName, mWaveState **state)
{
    int error;
    static fileHandle f;
    static mWaveRiffChunk riff;
    static uchar wave[4];
    static mWaveFmtData fmt;
    static mWavePcmFmtData pcmfmt;
    static unsigned length;
    static mWaveState *s;    

    /* Open the file: */
    if ( (error = fileOpen(fileName, fileOpenRead, &f)) != OK )        
        PASSERROR(ID_mWaveOpen);

    /* Read and check initial "RIFF" chunk and WAVE id: */
    if ( (error = fileRead(f, (void*) &riff, sizeof(mWaveRiffChunk))) != OK )
    {
        fileClose(f);
        PASSERROR(ID_mWaveOpen);
    }
    if ( !mMemEqual(riff.riff, "RIFF", 4) )
    {
        fileClose(f);
        ERROR(errBadWaveFile, ID_mWaveOpen);
        return errBadWaveFile;
    }
    if ( (error = fileRead(f, (void*) wave, 4)) != OK )
    {
        fileClose(f);
        PASSERROR(ID_mWaveOpen);
    }
    if ( !mMemEqual(wave, "WAVE", 4) )
    {
        fileClose(f);
        ERROR(errBadWaveFile, ID_mWaveOpen);
        return errBadWaveFile;
    }

    /* Find the format chunk and read it: */
    if ( (error = mWaveFindChunk(f, "fmt ", &length)) != OK )
    {
        fileClose(f);
        PASSERROR(ID_mWaveOpen);
    }
    if ( length < sizeof(mWaveFmtData) )
    {
        fileClose(f);
        ERROR(errBadWaveFile, ID_mWaveOpen);
        return errBadWaveFile;
    }    
    if ( (error = fileRead(f, (void*) &fmt, sizeof(mWaveFmtData))) != OK )
    {
        fileClose(f);
        PASSERROR(ID_mWaveOpen);
    }
    length -= sizeof(mWaveFmtData);

    /* Check that the WAVE file is a valid PCM file, and, if so, read the
       PCM format data: */
    if ( fmt.formatTag != 1 )
    {
        fileClose(f);
        ERROR(errUnsuppSampleFormat, ID_mWaveOpen);
        return errUnsuppSampleFormat;
    }
    if ( length < sizeof(mWavePcmFmtData) )
    {
        fileClose(f);
        ERROR(errBadWaveFile, ID_mWaveOpen);
        return errBadWaveFile;
    }
    if ( (error = fileRead(f, (void*) &pcmfmt, sizeof(mWavePcmFmtData)))
         != OK )
    {
        fileClose(f);
        PASSERROR(ID_mWaveOpen);
    }
    length -= sizeof(mWavePcmFmtData);

    /* Skip the rest of the fmt chunk: */
    if ( length > 0 )
    {
        if ( (error = fileSeek(f, length, fileSeekRelative)) != OK )
        {
            fileClose(f);
            PASSERROR(ID_mWaveOpen);
        }
    }

    /* Finally, find the start of the data chunk: */
    if ( (error = mWaveFindChunk(f, "data", &length)) != OK )
    {
        fileClose(f);
        PASSERROR(ID_mWaveOpen);
    }

    /* Allocate and build the state structure: */
    if ( (error = memAlloc(sizeof(mWaveState), (void**) &s)) != OK )
    {
        fileClose(f);
        PASSERROR(ID_mWaveOpen);
    }

    s->file = f;
    s->dataLength = length;
    s->dataLeft = length;
    s->sampleRate = fmt.samplesPerSec;
    s->sampleType = smpNone;

    if ( (error = fileGetPosition(f, (long*) &s->dataStart)) != OK )
    {
        memFree(s);
        fileClose(f);
        PASSERROR(ID_mWaveOpen);
    }

    /* Determine the sample type: */
    if ( (fmt.numChannels == 1) && (pcmfmt.bitsPerSample == 8) )
        s->sampleType = smp8bitMono;
    if ( (fmt.numChannels == 2) && (pcmfmt.bitsPerSample == 8) )
        s->sampleType = smp8bitStereo;
    if ( (fmt.numChannels == 1) && (pcmfmt.bitsPerSample == 16) )
        s->sampleType = smp16bitMono;
    if ( (fmt.numChannels == 2) && (pcmfmt.bitsPerSample == 16) )
        s->sampleType = smp16bitStereo;

    /* Check that we got it: */
    if ( s->sampleType == smpNone )
    {
        memFree(s);
        fileClose(f);
        ERROR(errUnsuppSampleFormat, ID_mWaveOpen);
        return errUnsuppSampleFormat;
    }

    *state = s;
    return OK;
}




/****************************************************************************\
*
* Function:     int mWaveClose(mWaveState *state)
*
* Description:  Closes a RIFF WAVE file opened with mWaveOpen() and
*               deallocates its state structure.
*
* Input:        mWaveState *state       the wave file state structure
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mWaveClose(mWaveState *state)
{
    int error;

    if ( (error = fileClose(state->file)) != OK )
        PASSERROR(ID_mWaveClose);
    if ( (error = memFree(state)) != OK )
        PASSERROR(ID_mWaveClose);

    return OK;
}




/*
 * $Log: waveread.c,v $
 * Revision 1.2  1997/07/31 10:56:57  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.1  1997/05/20 20:35:29  pekangas
 * Initial revision
 *
*/







