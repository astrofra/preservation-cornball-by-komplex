/*      loadit.c
 *
 * Generic Module Player Impulse Tracker loader
 *
 * $Id: loadit.c,v 1.19.2.1 1997/09/07 21:14:31 jpaana Exp $
 *
 * Copyright 1996, 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include <stdio.h>
#include <string.h>

#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#include "mglobals.h"
#include "mmem.h"
#include "file.h"
#include "sdevice.h"
#include "gmplayer.h"
#include "it.h"
#include "mutils.h"

#ifdef __WC32__
#include <malloc.h>
#endif

RCSID(const char *loadit_rcsid = "$Id: loadit.c,v 1.19.2.1 1997/09/07 21:14:31 jpaana Exp $";)

/*#define CHECK_THE_HEAP*/

extern ulong *__nheapbeg;

//#define DEBUGMESSAGES


/****************************************************************************\
*
* Function:     TestHeap(void)
*
* Description:  Test if the heap is corrupted. Watcom C ONLY! Implemented here
*               because the Watcom _heapchk() often reports the heap is OK
*               even though it isn't.
*
* Returns:      MIDAS error code
*
\****************************************************************************/

#if defined(CHECK_THE_HEAP) && defined(__WC32__)
    /* Maximum valid block in heap: */
#define MAXBLK (2048*1024)

static int TestHeap(void)
{
    static struct _heapinfo hinfo;
    int         hstatus;
    unsigned    len;

    hinfo._pentry = NULL;

    while ( 1 )
    {
        hstatus = _heapwalk(&hinfo);
        if ( hstatus != _HEAPOK )
            break;
        len = hinfo._size;
        if ( (len == 0) || (len > MAXBLK) || (len & 3) )
            return errHeapCorrupted;
    }

    if ( (hstatus == _HEAPEND) || (hstatus == _HEAPEMPTY) )
        return OK;
    return errHeapCorrupted;
}
#define CHECKHEAP \
    { \
        if ( TestHeap() != OK ) \
        { \
            puts("HEAP CORRUPTED!"); \
            ERROR(errHeapCorrupted, ID_gmpLoadIT); \
            LoadError(); \
            return errHeapCorrupted; \
        } \
    }

#else
#define CHECKHEAP
#endif

/* Pass error code in variable "error" on, used in gmpLoadIT(). */
#define PASSERR() { LoadError(); PASSERROR(ID_gmpLoadIT) }


/* Conversion table from Impulse Tracker command number to GMP ones: */
static uchar itCommands[26] =
{
    gmpcSetSpeed,               /* A - Set speed */
    gmpcPositionJump,           /* B - Position jump */
    gmpcPatternBreak,           /* C - Pattern break */
    gmpcVolumeSlide,            /* D - Volume slide */
    gmpcSlideDown,              /* E - Slide down */
    gmpcSlideUp,                /* F - Slide up */
    gmpcTonePortamento,         /* G - Tone portamento */
    gmpcVibrato,                /* H - Vibrato */
    gmpcNone,                   /* I - Tremor */
    gmpcArpeggio,               /* J - Arpeggio */
    gmpcVibVSlide,              /* K - Vibrato + volume slide */
    gmpcTPortVSlide,            /* L - Tone portamento + volume slide */
    gmpcNone,                   /* M - Set channel volume */
    gmpcNone,                   /* N - Channel volume slide */
    gmpcSampleOffset,           /* O - Set sample offset */
    gmpcPanSlide,               /* P - Pan slide */
    gmpcS3MRetrig,              /* Q - ST3 extended retrig */
    gmpcTremolo,                /* R - Tremolo */
    gmpcNone,                   /* S - S-commands (table below) */
    gmpcSetTempo,               /* T - Set tempo */
    gmpcNone,                   /* U - Fine vibrato */
    gmpcSetMVolume,             /* V - Set master volume */
    gmpcMVolSlide,              /* W - Master volume slide */
    gmpcSetPanning,             /* X - Set panning */
    gmpcNone,                   /* Y - Panbrello */
    gmpcMusicSync               /* Z - Music synchronization (really midi
                                       macros) */
};

/* Conversion table from Impulse Tracker S-command numbers to GMP commands: */
static uchar itSCommands[16] =
{
    gmpcNone,                   /* S0 - Set filter */
    gmpcNone,                   /* S1 - Set glissando control */
    gmpcNone,                   /* S2 - Set finetune */
    gmpcNone,                   /* S3 - Set vibrato waveform */
    gmpcNone,                   /* S4 - Set tremolo waveform */
    gmpcNone,                   /* S5 - Set panbrello waveform */
    gmpcNone,                   /* S6 - Unused */
    gmpcNone,                   /* S7 - NNA's and stuff */
    gmpcSetPanning16,           /* S8 - Set panning */
    gmpcNone,                   /* S9 - Set surround sound (S91) */
    gmpcNone,                   /* SA - Stereo control (obsolete) */
    gmpcPatternLoop,            /* SB - Pattern loop */
    gmpcNoteCut,                /* SC - Note cut */
    gmpcNoteDelay,              /* SD - Note delay */
    gmpcPatternDelay,           /* SE - Pattern delay */
    gmpcNone                    /* SF - Invert loop (unused in ST3) */
};



/****************************************************************************\
*       Module loader buffers and file handle. These variables are static
*       instead of local so that a separate deallocation can be used which
*       will be called before exiting in error situations
\****************************************************************************/
static fileHandle f;                    /* file handle for module file */
static int      fileOpened;             /* 1 if module file has been opened */
static gmpModule *module;               /* pointer to GMP module structure */
static itHeader *header;                /* pointer to IT module header */
static uchar    *instUsed;              /* instrument used flags */
static itPattern *itPatt;               /* pointer to IT pattern data */
static gmpPattern *convPatt;            /* pointer to converted pattern data*/
static uchar    *smpBuf;                /* sample loading buffer */
static unsigned numChans;               /* number of channels in module */
static itInstHeader *itInst;            /* IT instrument header */
static itOldInstHeader *itOInst;        /* Old IT instrument header */
static itSampleHeader *itSample;        /* IT sample header */
static int      maxInstUsed;            /* maximum used instrument number */
static U32      *instOffsets;
static U32      *smpOffsets;
static U32      *pattOffsets;
static U8       *itOrders;
static unsigned maxChan;
static U8       maskVars[64];
static U8       lastNote[64];
static U8       lastInst[64];
static U8       lastVol[64];
static U8       lastCmd[64];
static U8       lastInfo[64];
static U8       convTab[120];

/****************************************************************************\
*
* Function:     int ConvertPattern(itPattern *srcPatt, gmpPattern *destPatt,
*                   unsigned *convLen);
*
* Description:  Converts a pattern from IT to GMP internal format.
*               Also updates instUsed flags.
*
* Input:        itPattern *srcPatt      pointer to pattern in IT format
*               gmpPattern *destPatt    pointer to destination GMP pattern
*               unsigned *convLen       pointer to converted pattern length
*
* Returns:      MIDAS error code. Converted pattern length (in bytes,
*               including header) is written to *convLen.
*
\****************************************************************************/

static int ConvertPattern(itPattern *srcPatt, gmpPattern *destPatt,
    unsigned *convLen)
{
    uchar       *src = &srcPatt->data[0];
    uchar       *dest = &destPatt->data[0];
    int         row, chan, t;
    unsigned    len;
    U8          note, inst, command, infobyte, vol, compInfo, mask;
    U8          itComp;

    /* Check if the pattern is empty: */
    if ( (srcPatt == NULL) || (srcPatt->pattDataSize == 0) )
    {
        /* Write row end marker for each row: */
        for ( row = 0; row < 64; row++ )
            *(dest++) = 0;

        /* Write number of rows to pattern header: */
        destPatt->rows = 64;

        /* Write converted pattern length to header and return it in
           *convLen: */
        *convLen = destPatt->length = 64 + sizeof(gmpPattern);
        return OK;
    }

    for ( row = 0; row < LE16(srcPatt->numRows); row++ )
    {
        while ( *src != 0 )
        {
            /* No data for this channel yet: */
            compInfo = 0;
            note = 0xFF;
            inst = 0xFF;
            command = 0;
            infobyte = 0;
            vol = 0;

            /* Get note number/compression info from pattern data: */
            itComp = *src++;

            chan = (itComp - 1) & 63;

            if ( itComp & 128 )
            {
                mask = *src++;
                maskVars[chan] = mask;
            }
            else
                mask = maskVars[chan];

            /* Check if there is a new note and instrument: */
            if ( mask & 1 )
            {
                /* Check that the note is not key off and convert it to GMP
                    internal format: */
                note = *src;
                if ( note < 120 )
                    note = ((note / 12 ) << 4) | (note % 12);

                /* In IT-format 0xFF is note off (similar to FT2 key off)
                   and 0xFE is note cut. As we already use 0xFF to no note,
                   this makes those compatible with our system. Phew. */

                if ( note > 0xF0 )
                    note--;

                src++;
                compInfo |= 32;
                lastNote[chan] = note;
            }

            if ( mask & 2)
            {
                /* Get the instrument number: */
                if ( *src <= module->numInsts )
                    inst = (*src) - 1;

                src++;

                /* Check if the instrument number if above maximum instrument
                    number used: */
                if ( inst < 100 )
                {
                    if ( inst < module->numInsts )
                    {
                        if ( inst > maxInstUsed )
                            maxInstUsed = inst;

                        /* Mark instrument used: */
                        instUsed[inst] = 1;
                    }
                }
                /* Mark that there is a new note and/or instrument: */
                compInfo |= 32;
                lastInst[chan] = inst;
            }

	    /* Check if there is a volume column byte: */
	    if ( mask & 4 )
	    {
		/* Get the volume column byte: */
		vol = *(src++);
		
		/* Mark that there is a volume column byte: */
		compInfo |= 64;
		lastVol[chan] = vol;
	    }

	    /* Check if there is a command: */
	    if ( mask & 8 )
	    {
		/* Get the command number: */
		command = *(src++);

		/* Get the infobyte: */
		infobyte = *(src++);
	    }

	    /* If there is a command or an infobyte we need to write the
	       command to the destination pattern: */
	    if ( (command > 0) && (command < 26 ) )
	    {
		switch ( command )
		{
		    case 19:
			/* Command S - check actual command number: */
			command = itSCommands[infobyte >> 4];
			infobyte &= 0x0F;
			break;
			
		    case 24:
			/* Command X - Set panning 0-255, convert to
			   0-63 (this is what IT does itself) */
			command = gmpcSetPanning;
			t = infobyte;
			infobyte = (t + 1) >> 2;
			break;

		    default:
			/* Convert command to GMP command number: */
			command = itCommands[command - 1];
			break;
		}

		/* Mark that there is a command: */
		if ( command != gmpcNone )
		    compInfo |= 128;
		lastCmd[chan] = command;
		lastInfo[chan] = infobyte;
	    }

	    if ( mask & 16 )
	    {
		note = lastNote[chan];
		compInfo |= 32;
	    }
	    if ( mask & 32 )
	    {
		inst = lastInst[chan];
		compInfo |= 32;
	    }
	    if ( mask & 64 )
	    {
		vol = lastVol[chan];
		compInfo |= 64;
	    }

	    if ( mask & 128 )
	    {
		command = lastCmd[chan];
		infobyte = lastInfo[chan];
		compInfo |= 128;
	    }

	    if ( chan < 32 )
	    {
                /* If the compression information is nonzero, there is some
                   data for this channel: */
                if ( ( compInfo & 0xE0 ) != 0 )
                {
		    if ( numChans <= (unsigned)(chan + 1) )
			numChans = chan + 1;

                    /* Set channel number to lower 5 bits of the compression info
                       and write it to destination: */
                    compInfo |= chan & 31;
                    if ( (unsigned) (chan & 31) <= maxChan )
                    {
                        *(dest++) = compInfo;

                        /* Check if there a note+instrument pair: */
                        if ( compInfo & 32 )
                        {
                            /* Write note and instrument numbers: */
                            *(dest++) = note;
                            *(dest++) = inst;
                        }

                        /* Check if there is a volume column byte: */
                        if ( compInfo & 64 )
                        {
                            /* Write the volume column byte: */
                            *(dest++) = vol;
                        }

                        /* Check if there is a command: */
                        if ( compInfo & 128 )
                        {
                            /* Write command and command infobyte: */
                            *(dest++) = command;
                            *(dest++) = infobyte;
                        }
                    }
                }
            }
        }

        /* Write row end marker: */
        *(dest++) = 0;
        src++;
    }

    /* Write number of rows to pattern header: */
    destPatt->rows = LE16(srcPatt->numRows);

    /* Calculate converted pattern length: */
    len = (unsigned) (dest  - ((uchar*) destPatt));

    /* Write converted pattern length to header and return it in *convLen: */
    destPatt->length = len;
    *convLen = len;

    return OK;
}



/****************************************************************************\
*
* Function:     void LoadError(void)
*
* Description:  Stops loading the module, deallocates all buffers and closes
*               the file.
*
\***************************************************************************/

#define condFree(x) { if ( x != NULL ) if ( memFree(x) != OK) return; }

static void LoadError(void)
{
    /* Close file if opened. Do not process errors. */
    if ( fileOpened )
        if ( fileClose(f) != OK )
            return;

    /* Attempt to deallocate module if allocated. Do not process errors. */
    if ( module != NULL )
        if ( gmpFreeModule(module) != OK )
            return;

    /* Deallocate structures if allocated: */
    condFree(header)
    condFree(instUsed)
    condFree(smpBuf)
    condFree(itInst)
    condFree(itOInst)
    condFree(itSample)
    condFree(itOrders)
    condFree(instOffsets)
    condFree(smpOffsets)
    condFree(pattOffsets)
}


/****************************************************************************\
*
* Function:     int ConvertEnvelope(itEnvelope *itEnv, gmpEnvelope *env);
*
* Description:  Converts an envelope from IT to GMP internal format.
*
* Input:        itEnvelope  *itEnv       pointer to envelope in IT format
*               gmpEnvelope **gmpEnv     pointer to destination GMP envelope
*
* Returns:      MIDAS error code.
*
\****************************************************************************/

int ConvertEnvelope(itEnvelope *itEnv, gmpEnvelope **gmpEnv)
{
    int error, i;
    gmpEnvelope *env;

    if ( itEnv->flags & 1)
    {
        if ( (error = memAlloc(sizeof(gmpEnvelope),
            (void**) gmpEnv)) != OK )
            PASSERR()

        env = *gmpEnv;

        env->numPoints = itEnv->num;

        if ( itEnv->flags & 2 )
        {
            env->loopStart = itEnv->loopStart;
            env->loopEnd = itEnv->loopEnd;
        }
        else
            env->loopStart = env->loopEnd = -1;

        if ( itEnv->flags & 4 )
        {
            env->susLoopStart = itEnv->susLoopStart;
            env->susLoopEnd = itEnv->susLoopEnd;
        }
        else
            env->susLoopStart = env->susLoopEnd = -1;

        for ( i = 0; i < env->numPoints; i++ )
        {
            env->points[i].x = LE16(itEnv->nodes[i].tick);
            env->points[i].y = itEnv->nodes[i].vol;
        }
    }
    else
        *gmpEnv = NULL;

    return OK;
}



int MakeNoteSamples(gmpInstrument *inst, U8 *noteSamples)
{
    unsigned i, j, k;

    memset(convTab, 0, 120);
    k = 0;

    for ( i = 0; i < 120; i++ )
    {
        inst->noteSamples[i*2] = noteSamples[i*2];
        if ( noteSamples[i*2+1] == 0 )
        {
            inst->noteSamples[i*2+1] = (uchar) -1;
        }
        else
        {
            for ( j = 0 ; j < k; j++ )
            {
                if ( convTab[j] == noteSamples[i*2+1] - 1)
                {
                    inst->noteSamples[i*2+1] = j;
                    break;
                }
            }
            if ( j == k )
            {
                inst->noteSamples[i*2+1] = k;
                convTab[k] = noteSamples[i*2+1] - 1;
                inst->samples[k] = module->samples[noteSamples[i*2+1]-1];
                k++;
            }
        }
    }
    inst->numSamples = k;
    return OK;
}


int NumSamples(U8 *noteSamples)
{
    unsigned i, j, k;

    memset(convTab, 0, 120);
    k = 0;

    for ( i = 0; i < 120; i++ )
    {
        if ( noteSamples[i*2+1] != 0 )
        {
            for ( j = 0 ; j < k; j++ )
            {
                if ( convTab[j] == noteSamples[i*2+1] - 1)
                    break;
            }
            if ( j == k )
            {
                convTab[k] = noteSamples[i*2+1] - 1;
                k++;
            }
        }
    }
    return k;
}



/****************************************************************************\
*
* Function:     int gmpLoadIT(char *fileName, int addSD, mpModule **module)
*
* Description:  Loads an Impulse Tracker module to memory in Generic Module
*               Player module format
*
* Input:        char *fileName          module file name
*               int addSD               1 if module samples should be added to
*                                       the current Sound Device, 0 if not
*               int (*SampleCallback)(...)  pointer to callback function that
*                                       will be called after sample has been
*                                       added to Sound Device, but before
*                                       sample data is deallocated
*               mpModule **module       pointer to GMP module pointer
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING gmpLoadIT(char *fileName, int addSD, int
    (CALLING *SampleCallback)(sdSample *sdsmp, gmpSample *gmpsmp),
    gmpModule **_module)
{

    int             error;              /* MIDAS error code */
    static gmpInstrument *inst;
    unsigned        i;
    int             j;
    ulong           maxSample;
    static unsigned convPattLen;
    static gmpSample *sample;
    static sdSample sdSmp;
    int             slength;
    unsigned        numInsts;
    unsigned        numSamples;
    unsigned        maxPatt;
    U8              *s;
    S16             *s2;
    int             mode, old;
    gmpEnvelope     *env;

    /* point buffers to NULL and set fileOpened to 0 so that LoadError()
       can be called at any point: */
    fileOpened = 0;
    module = NULL;
    header = NULL;
    instUsed = NULL;
    itPatt = NULL;
    convPatt = NULL;
    smpBuf = NULL;
    itInst = NULL;
    itOInst = NULL;
    itSample = NULL;
    itOrders = NULL;
    instOffsets = NULL;
    smpOffsets = NULL;
    pattOffsets = NULL;


#ifdef DEBUGMESSAGES
    puts("Loading header");
#endif

    /* Allocate memory for IT module header: */
    if ( (error = memAlloc(sizeof(itHeader), (void**) &header)) != OK )
        PASSERR()

    /* Open module file: */
    if ( (error = fileOpen(fileName, fileOpenRead, &f)) != OK )
        PASSERR()

    /* Allocate memory for the module structure: */
    if ( (error = memAlloc(sizeof(gmpModule), (void**) &module)) != OK )
        PASSERR()

    /* Clear module structure so that it can safely be deallocated with
       gmpFreeModule() at any point: */
    module->panning = NULL;
    module->songData = NULL;
    module->instruments = NULL;
    module->samples = NULL;
    module->patterns = NULL;

    /* Read Impulse Tracker module header: */
    if ( (error = fileRead(f, header, sizeof(itHeader))) != OK )
        PASSERR()

    /* Check that the header contains a valid signature and the file format
       version number is correct: */
    if ( (!mMemEqual(header->signature, "IMPM", 4)) )
    {
        ERROR(errInvalidModule, ID_gmpLoadIT);
        LoadError();
        return errInvalidModule;
    }

    /* Get number of channels from the header: */
    for ( i = 0; i < 32; i++ )
    {
	if ( header->chanPan[i] < 127 )
	    numChans = i;
    }

    maxChan = numChans + 1;

    /* Copy song name: */
    mMemCopy(module->name, header->name, 26);
    module->name[26] = 0;                   /* force terminating '\0' */

    module->songLength = LE16(header->songLength); /* copy song length */
    module->playMode = gmpIT;               /* set IT playing mode */
    module->masterVolume = 64;              /* maximum master volume */
    module->speed = (uchar) header->speed;  /* copy initial speed */
    module->tempo = (uchar) header->tempo;  /* copy initial tempo */
    module->restart = 0;                    /* restart position is always 0 */
    module->numPatts = LE16(header->numPatts); /* copy number of patterns */
    module->numInsts = LE16(header->numInsts); /* copy number of instruments */
    module->numSamples = LE16(header->numSamples); /* copy number of samples */

    numSamples = module->numSamples;

    mode = 0;   /* Sample mode */
    module->playFlags.instruments = 0;

    old = (LE16(header->compatibleVersion) < 0x200 );

    /* Check if instrument mode is used: */
    if ( LE16(header->flags) & itInstruments )
    {
        mode = 1;
        module->playFlags.instruments = 1;
    }

    /* Check if linear frequencies are used: */
    if ( LE16(header->flags) & itLinearSlides )
        module->playFlags.linFreqTable = 1;
    else
        module->playFlags.linFreqTable = 0;

    /* Allocate memory for channel initial panning positions: */
    if ( (error = memAlloc(64 * sizeof(int), (void**) &module->panning))
        != OK )
        PASSERR()

    /* Set initial panning position to all middle: */
    for ( i = 0; i < 64; i++ )
        module->panning[i] = header->chanPan[i];

    /* Allocate memory for song data: */
    if ( (error = memAlloc(sizeof(ushort) * module->songLength,
        (void**) &module->songData)) != OK )
        PASSERR()

    /* Allocate memory for song data: */
    if ( (error = memAlloc( module->songLength,
        (void**) &itOrders)) != OK )
        PASSERR()

    /* Read orders: */
    if ( (error = fileRead(f, itOrders, module->songLength)) != OK )
        PASSERR()

    /* Copy song data and find the maximum pattern used: */
    maxPatt = 0;
    for ( i = 0; i < module->songLength; i++ )
    {
        module->songData[i] = (signed char)itOrders[i];
        if ( module->songData[i] > maxPatt && (itOrders[i] < 250))
            maxPatt = module->songData[i];
    }

    /* Deallocate orders: */
    if ( (error = memFree(itOrders)) != OK )
        PASSERR()
    itOrders = NULL;

    /* If more patterns are used than there are in the file, allocate memory
       for all of them: */
    if ( maxPatt >= module->numPatts )
        module->numPatts = maxPatt + 1;

    /* Allocate memory for pattern pointers: */
    if ( (error = memAlloc(sizeof(gmpPattern*) * module->numPatts,
        (void**) &module->patterns)) != OK )
        PASSERR()

    /* Set all pattern pointers to NULL to mark them unallocated: */
    for ( i = 0; i < module->numPatts; i++ )
        module->patterns[i] = NULL;

    /* Allocate memory for instrument used flags: */
    if ( (error = memAlloc(256, (void**)&instUsed)) != OK )
        PASSERR()

    /* Mark all instruments unused: */
    for ( i = 0; i < 256; i++ )
        instUsed[i] = 0;

    /* Clear maximum instrument number used: */
    maxInstUsed = 0;

    /* Allocate memory for instrument pointers: */
    if ( mode )
    {
        if ( (error = memAlloc(4 * module->numInsts,
                               (void**) &instOffsets)) != OK )
            PASSERR()
    }

    /* Allocate memory for sample pointers: */
    if ( (error = memAlloc( 4 * numSamples,
        (void**) &smpOffsets)) != OK )
        PASSERR()

    /* Allocate memory for pattern pointers: */
    if ( (error = memAlloc(4 * module->numPatts,
        (void**) &pattOffsets)) != OK )
        PASSERR()

    /* Read instrument offsets: */
    if ( mode )
    {
        if ( (error = fileRead(f, instOffsets, 4 * module->numInsts)) != OK )
            PASSERR()
    }
    else
    {
        if (module->numInsts)
            if ( (error = fileSeek(f, 4 * module->numInsts, fileSeekRelative)) != OK )
                PASSERR()
    }
    /* Read sample offsets: */
    if ( (error = fileRead(f, smpOffsets, 4 * numSamples)) != OK )
        PASSERR()

    /* Read pattern offsets: */
    if ( (error = fileRead(f, pattOffsets, 4 * module->numPatts)) != OK )
        PASSERR()

    /* Allocate memory for pattern loading buffer: */
    if ( (error = memAlloc(sizeof(itPattern) + numChans * 5 * 256,
        (void**) &itPatt)) != OK )
        PASSERR()

    /* Allocate memory for pattern conversion buffer: (maximum GMP pattern
       data size is 6 bytes per row per channel plus header) */
    if ( (error = memAlloc(numChans * 256 * 6 + sizeof(gmpPattern),
        (void**) &convPatt)) != OK )
        PASSERR()

#ifdef DEBUGMESSAGES
    puts("Starting to load patterns");
#endif

    if ( !mode )
        module->numInsts = numSamples; /* !!!HACK!!! */

    /* Load and convert all patterns: */
    for ( i = 0; i < module->numPatts; i++ )
    {
#ifdef DEBUGMESSAGES
        printf("Pattern %i - ", i);
#endif

        CHECKHEAP

        /* Check if this pattern is in file: */
        if ( i < LE16(header->numPatts)  && pattOffsets[i] )
        {
            /* Seek to pattern start: */
            if ((error = fileSeek(f, LE32(pattOffsets[i]), fileSeekAbsolute)) != OK )
                PASSERR()

            /* Read pattern header: */
            if ( (error = fileRead(f, itPatt, sizeof(itPattern))) != OK )
                PASSERR()

            /* Read pattern data: */
            if ( itPatt->pattDataSize != 0 )
            {
                if ( (error = fileRead(f, itPatt->data, LE16(itPatt->pattDataSize)))
                    != OK )
                    PASSERR()
            }
        }
        else
        {
            /* Pattern used but not in file - mark empty: */
            itPatt->pattDataSize = 0;
            itPatt->numRows = LE16(64);

#ifdef DEBUGMESSAGES
            printf("Empty -");
#endif
        }

        CHECKHEAP

#ifdef DEBUGMESSAGES
        printf("Loaded, about to convert - ");
#endif

        /* Convert the pattern data, checking the instruments used: */
        if ( (error = ConvertPattern(itPatt, convPatt, &convPattLen))
            != OK )
            PASSERR()

#ifdef DEBUGMESSAGES
        printf("Conv. size %u - ", convPattLen);
#endif

        CHECKHEAP

        /* Allocate memory for converted pattern data for current pattern
           in module: */
        if ( (error = memAlloc(convPattLen, (void**)&module->patterns[i]))
             != OK )
             PASSERR()

        /* Copy the converted pattern data: */
        mMemCopy(module->patterns[i], convPatt, convPattLen);

#ifdef DEBUGMESSAGES
        puts("OK");
#endif
        CHECKHEAP
    }

#ifdef DEBUGMESSAGES
    puts("Patterns loaded");
#endif

    CHECKHEAP

    /* Deallocate pattern conversion buffer: */
    if ( (error = memFree(convPatt)) != OK )
        PASSERR()
    convPatt = NULL;

    /* Deallocate pattern loading buffer: */
    if ( (error = memFree(itPatt)) != OK )
        PASSERR()
    itPatt = NULL;

    /* Deallocate pattern offsets: */
    if ( (error = memFree(pattOffsets)) != OK )
        PASSERR()
    pattOffsets = NULL;

    maxChan++;
    /* store number of channels */
    module->numChannels = ( maxChan < numChans ) ? maxChan : numChans;
#ifdef DEBUGMESSAGES
    printf("Num.channels: %i\n", module->numChannels);
#endif
    CHECKHEAP


    /* Check actual number of instruments needed: (ensure that all used
       instruments are added to Sound Device) */
    if ( maxInstUsed >= LE16(header->numInsts) )
        numInsts = maxInstUsed + 1;
    else
        numInsts = LE16(header->numInsts);
    module->numInsts = numInsts;

#ifdef DEBUGMESSAGES
    printf("maxInstUsed = %i\n", maxInstUsed);
#endif


    /* Now we read all instrument and sample headers from the files,
       converting them to GMPlayer structures. Offsets to sample data in the
       file are saved in GMPlayer sample structure sample pointers (always
       at least 32-bit) so that they can be read faster. This is necessary
       as we need to be able to find out the maximum sample length to be able
       to allocate a loading buffer for the samples. */


    /* Allocate memory for instrument pointers: */
    if ( (error = memAlloc(sizeof(gmpInstrument*) * numInsts,
        (void**) &module->instruments)) != OK )
        PASSERR()

    /* Set all instrument pointers to NULL to mark them unallocated: */
    for ( i = 0; i < numInsts; i++ )
        module->instruments[i] = NULL;

    /* Allocate memory for sample pointers: */
    if ( (error = memAlloc(sizeof(gmpSample*) * numSamples,
        (void**) &module->samples)) != OK )
        PASSERR()

    /* Set all sample pointers to NULL to mark them unallocated: */
    for ( i = 0; i < numSamples; i++ )
        module->samples[i] = NULL;


    /* Allocate memory for instrument and sample headers: */
    if ( old )
    {
        if ( (error = memAlloc(sizeof(itOldInstHeader), (void**) &itOInst)) != OK )
            PASSERR()
    }
    else
    {
        if ( (error = memAlloc(sizeof(itInstHeader), (void**) &itInst)) != OK )
            PASSERR()
    }

    if ( (error = memAlloc(sizeof(itSampleHeader), (void**) &itSample))
        != OK )
        PASSERR()

    maxSample = 0;


    for ( i = 0; i < module->numSamples; i++ )
    {
        /* Seek to sample header in file: */
        if ( (error = fileSeek(f, LE32(smpOffsets[i]), fileSeekAbsolute))
            != OK )
            PASSERR()

        /* Read sample header from file: */
        if ( (error = fileRead(f, itSample, sizeof(itSampleHeader))) != OK )
            PASSERR()

        if ( LE32(itSample->length) > maxSample )
            maxSample = LE32(itSample->length);

	if ( itSample->flags & 2 )
            if ( LE32(itSample->length)*2 > maxSample )
                maxSample = LE32(itSample->length)*2;

    }

    /* Check that the maximum sample length is below the Sound Device limit:*/
    if ( maxSample > SMPMAX )
    {
        ERROR(errInvalidInst, ID_gmpLoadIT);
        LoadError();
        return errInvalidInst;
    }

    /* If samples should be added to Sound Device, allocate memory for sample
       loading buffer: */
    if ( addSD )
    {
        if ( (error = memAlloc(maxSample, (void**) &smpBuf)) != OK )
            PASSERR()
    }


    if ( maxInstUsed > (int)module->numInsts )
        module->numInsts = maxInstUsed;

#ifdef DEBUGMESSAGES
    printf("Max.inst. used: %i\n", maxInstUsed);
#endif
    CHECKHEAP

    /* Load all samples and convert sample and instrument data: */
    for ( i = 0; i < module->numSamples; i++ )
    {
        /* Seek to sample header in file: */
        if ( (error = fileSeek(f, LE32(smpOffsets[i]), fileSeekAbsolute))
            != OK )
            PASSERR()

        /* Read sample header from file: */
        if ( (error = fileRead(f, itSample, sizeof(itSampleHeader))) != OK )
            PASSERR()

        /* Allocate memory for this sample structure: */
        if ( (error = memAlloc(sizeof(gmpSample),
             (void**) &module->samples[i])) != OK )
            PASSERR()

        /* Point sample to current sample structure: */
        sample = module->samples[i];

        /* Copy sample name: */
        mMemCopy(&sample->name[0], &itSample->sampleName[0], 26);
        sample->name[26] = 0;             /* force terminating '\0' */

        /* Auto-vibrato: */
        sample->vibType = itSample->vibType;
        sample->vibSweep = itSample->vibRate;
        sample->vibDepth = itSample->vibDepth;
        sample->vibRate = itSample->vibSpeed;

        /* Mark there is no sample data and the sample has not been added
           to Sound Device: */
        sample->sample = NULL;
        sample->sdHandle = 0;

        /* Copy sample length, loop starts and ends: */
        slength = LE32(itSample->length);
        sample->sampleLength = slength;
        sample->loop1Start = LE32(itSample->loopStart);
        sample->loop1End = LE32(itSample->loopEnd);
        sample->loop2Start = LE32(itSample->susLoopStart);
        sample->loop2End = LE32(itSample->susLoopEnd);

        /* Check sample type */
        sample->sampleType = (itSample->flags & 2)?smp16bit:smp8bit;


        /* Set sample loop flags */
        if (!(itSample->flags & 48) )
        {
            /* Sample not looping: */
            sample->loopMode = sdLoopNone;
            sample->loop1Type = loopNone;
            sample->loop2Type = loopNone;
        }
        else
        {
            if ( itSample->flags & 32 )
            {
                /* Sample has two loops */
                sample->loopMode = sdLoop2;

                /* Is the second loop bi- or unidir */
                if ( itSample->flags & 128 )
                    sample->loop2Type = loopBidir;
                else
                    sample->loop2Type = loopUnidir;
            }
            else
            {
                /* Sample has one loop */
                sample->loopMode = sdLoop1Rel;
                sample->loop2Type = loopNone;
            }
            /* Is the first loop bi- or unidir */
            if ( itSample->flags & 64 )
                sample->loop1Type = loopBidir;
            else
                sample->loop1Type = loopUnidir;
        }

        /* Double lengths for 16-bit samples */
        if ( sample->sampleType == smp16bit )
        {
            sample->sampleLength <<= 1;
            sample->loop1Start <<= 1;
            sample->loop1End <<= 1;
            sample->loop2Start <<= 1;
            sample->loop2End <<= 1;
            slength <<= 1;
        }
        /* Copy sample default volume: */
        sample->volume = itSample->volume;
        sample->gvolume = itSample->GvL;

        /* No finetune used: */
        sample->finetune = 0;

        /* Copy sample base tune: */
        sample->baseTune = LE32(itSample->C5speed);

        /* Set sample default panning position to not used: */
        sample->panning = -1;

        /* Check if there is sample data for this sample: */
        if ( slength != 0 )
        {
            /* If sample data should not be added to Sound Device, allocate
               memory for the sample data and point smpBuf there: */
            if ( !addSD )
            {
                smpBuf = NULL;
                if ( (error = memAlloc(slength, (void**)&smpBuf)) != OK )
                    PASSERR()
                sample->sample = smpBuf;
            }
            else
            {
                /* Sample is added to the Sound Device - sample data is not
                   available: */
                sample->sample = NULL;
            }

            /* Seek to sample in file: */
            if ( (error = fileSeek(f, LE32(itSample->samplePointer), fileSeekAbsolute))
                 != OK )
                PASSERR()

            /* There is sample data - load sample: */
            if ( (error = fileRead(f, smpBuf, slength)) != OK )
                PASSERR()

            /* Set Sound Device sample type: */
            if ( itSample->flags & 2 )
            {
                sdSmp.sampleType = smp16bit;
#ifdef __BIGENDIAN__
                s2 = (S16*)smpBuf;
                for ( j = 0; j < slength; j += 2 )
                {
                    *s2 = LE16(*s2);
                    s2++;
                }
#endif
                if ( !(LE16(itSample->convert) & 1) )
                {
                    s2 = (S16*)smpBuf;
                    for ( j = 0; j < slength; j += 2 )
                    {
                        *s2 ^= 0x8000;
                        s2++;
                    }
                }
            }
            else
            {
                sdSmp.sampleType = smp8bit;

                if ( itSample->convert & 1 )
                {
                    s = smpBuf;
                    for (  j = 0; j < slength; j++ )
                    {
                        *s ^= 0x80;
                        s++;
                    }
                }
            }

            /* Set Sound Device sample position in memory: */
            sdSmp.samplePos = sdSmpConv;

            /* Point Sound Device sample data to sample loading buffer: */
            sdSmp.sample = smpBuf;

            /* Point smpBuf to NULL if the sample is not added to Sound
               Device to mark it should not be deallocated: */
            if ( !addSD )
                smpBuf = NULL;
        }
        else
        {
            /* Mark there is no sample data: */
            sdSmp.sampleType = smpNone;
            sdSmp.samplePos = sdSmpNone;
            sdSmp.sample = NULL;
        }

        sdSmp.sampleLength = slength;
        sdSmp.loopMode = sample->loopMode;
        sdSmp.loop1Start = sample->loop1Start;
        sdSmp.loop1End = sample->loop1End;
        sdSmp.loop1Type = sample->loop1Type;
        sdSmp.loop2Start = sample->loop2Start;
        sdSmp.loop2End = sample->loop2End;
        sdSmp.loop2Type = sample->loop2Type;

        /* Set up the rest of Sound Device sample structure so that the sample
           can be added to the Sound Device: */
        if ( addSD )
        {
            /* Add the sample to Sound Device and store the Sound Device
               sample handle in sample->sdHandle: */
            if ( (error = gmpSD->AddSample(&sdSmp, 1, &sample->sdHandle))
                 != OK)
                PASSERR()

            /* Call sample callback if used: */
            if ( SampleCallback != NULL )
            {
                if ( (error = SampleCallback(&sdSmp, sample)) != OK )
                    PASSERR()
            }
        }
        else
        {
            /* Sample data has not been added to Sound Device: */
            sample->sdHandle = 0;
        }

#ifdef DEBUGMESSAGES
        printf("Inst: %i, Len: %i\n", i, slength);
#endif
//        CHECKHEAP
    }


    /* Deallocate sample header */
    if ( (error = memFree(itSample)) != OK )
        PASSERR()
    itSample = NULL;

    /* Deallocate sample loading buffer if allocated: */
    if ( addSD )
    {
        if ( (error = memFree(smpBuf)) != OK )
            PASSERR()
    }
    smpBuf = NULL;


/* And so we get to the best part, the instruments... */

    for ( i = 0; i < numInsts; i++ )
    {
        if (mode)
        {
            /* Instrument mode! */

            /* Seek to instrument header in file: */
            if ( (error = fileSeek(f, LE32(instOffsets[i]), fileSeekAbsolute))
                 != OK )
                PASSERR()

            /* Are the headers of old format? */
            if ( old )
            {
                /* Read instrument header from file: */
                if ( (error = fileRead(f, itOInst, sizeof(itOldInstHeader))) != OK )
                    PASSERR()

                j = NumSamples(&itOInst->noteSampleTable[0]);

                if ( (error = memAlloc(sizeof(gmpInstrument) +
                    j * sizeof(gmpSample*),
                    (void**) &module->instruments[i])) != OK )
                    PASSERR()

                inst = module->instruments[i];

                mMemCopy(&inst->name[0], &itOInst->instName[0], 26);
                inst->name[26] = 0;

                inst->NNA = itOInst->NNA;
                inst->DCT = itOInst->DNC;
                inst->DCA = 0;
                inst->PPS = 0;
                inst->PPC = 0;
                inst->RV = 0;
                inst->RP = 0;
                inst->DfP = (uchar) -1;
                inst->volFadeout = LE16(itOInst->fadeOut);
                inst->volume = 64;
                inst->numSamples = j;
                inst->used = instUsed[i];

                if ( itOInst->flags & 1 )
                {
                    if ( (error = memAlloc(sizeof(gmpEnvelope),
                       (void**) &inst->volEnvelope)) != OK )
                        PASSERR()

                    env = inst->volEnvelope;

                    env->numPoints = 25;

                    if ( itOInst->flags & 2 )
                    {
                        env->loopStart = itOInst->volLoopStart;
                        env->loopEnd = itOInst->volLoopEnd;
                    }
                    else
                        env->loopStart = env->loopEnd = -1;

                    if ( itOInst->flags & 4 )
                    {
                        env->susLoopStart = itOInst->susLoopStart;
                        env->susLoopEnd = itOInst->susLoopEnd;
                    }
                    else
                        env->susLoopStart = env->susLoopEnd = -1;

                    for ( j = 0; j < env->numPoints; j++ )
                    {
                        env->points[j].x = itOInst->nodePoints[j*2];
                        env->points[j].y = itOInst->nodePoints[j*2+1];
                    }
                }
                else
                    inst->volEnvelope = NULL;

                inst->panEnvelope = NULL;
                inst->pitchEnvelope = NULL;

                if ( (error = memAlloc( 240,
                   (void**) &inst->noteSamples)) != OK )
                    PASSERR()

                if ( ( error = MakeNoteSamples(inst,
                   &itOInst->noteSampleTable[0])) != OK )
                    PASSERR()

            }   /* Old IT-instruments */

            else
            {
                /* Read instrument header from file: */
                if ( (error = fileRead(f, itInst, sizeof(itInstHeader))) != OK )
                    PASSERR()

                j = NumSamples(&itInst->noteSampleTable[0]);

                if ( (error = memAlloc(sizeof(gmpInstrument) +
                    j * sizeof(gmpSample*),
                    (void**) &module->instruments[i])) != OK )
                    PASSERR()

                inst = module->instruments[i];

                mMemCopy(&inst->name[0], &itInst->instName[0], 26);
                inst->name[26] = 0;

                inst->NNA = itInst->NNA;
                inst->DCT = itInst->DCT;
                inst->DCA = itInst->DCA;
                inst->PPS = itInst->PPS;
                inst->PPC = itInst->PPC;
                inst->RV = itInst->RV;
                inst->RP = itInst->RP;
                inst->DfP = itInst->DfP;
                inst->volFadeout = LE16(itInst->fadeOut);
                inst->volume = itInst->GbV;
                inst->numSamples = j;
                inst->used = instUsed[i];

                if ( ( error = ConvertEnvelope(&itInst->volEnvelope,
                       &inst->volEnvelope)) != OK )
                    PASSERR()

                if ( ( error = ConvertEnvelope(&itInst->panEnvelope,
                       &inst->panEnvelope)) != OK )
                    PASSERR()

                if ( ( error = ConvertEnvelope(&itInst->pitchEnvelope,
                       &inst->pitchEnvelope)) != OK )
                    PASSERR()

                if ( (error = memAlloc( 240,
                    (void**) &inst->noteSamples)) != OK )
                    PASSERR()

                if ( ( error = MakeNoteSamples(inst,
                    &itInst->noteSampleTable[0])) != OK )
                    PASSERR()

            } /* New IT-instruments */
        }
        else
        {
            /* Sample mode */

            /* Allocate memory for this instrument structure: */
            if ( (error = memAlloc(sizeof(gmpInstrument) + sizeof(gmpSample*),
                (void**) &module->instruments[i])) != OK )
                PASSERR()
            inst = module->instruments[i];

            sample = module->samples[i];

            mMemCopy(&inst->name[0], &sample->name[0], 26);
            inst->name[26] = 0;

            inst->noteSamples = NULL;
            inst->samples[0] = sample;
            inst->volEnvelope = NULL;
            inst->panEnvelope = NULL;
            inst->pitchEnvelope = NULL;
            inst->DfP = (uchar) -1;
            inst->vibType = inst->vibSweep = inst->vibDepth = inst->vibRate = 0;
            inst->volFadeout = 0;
            inst->volume = 64;

            if ( instUsed[i] )
            {
                inst->used = 1;
                inst->numSamples = 1;
            }
            else
            {
                inst->used = 0;
                inst->numSamples = 0;
            }
        }
    }

    /* Deallocate instrument headers: */
    if ( mode )
    {
        if ( old )
        {
            if ( (error = memFree(itOInst)) != OK )
                PASSERR()
            itOInst = NULL;
        }
        else
        {
            if ( (error = memFree(itInst)) != OK )
                PASSERR()
            itInst = NULL;
        }
    }
    CHECKHEAP

    /* Deallocate instrument use flags: */
    if ( (error = memFree(instUsed)) != OK )
        PASSERR()
    instUsed = 0;

    /* Now we are finished loading. Close module file: */
    if ( (error = fileClose(f)) != OK)
        PASSERR()
    fileOpened = 0;

    /* Deallocate Impulse Tracker module header: */
    if ( (error = memFree(header)) != OK )
        PASSERR();

    /* Return module pointer in *module: */
    *_module = module;

    return OK;
}


/*
 * $Log: loadit.c,v $
 * Revision 1.19.2.1  1997/09/07 21:14:31  jpaana
 * Fixed to take all data into account when calculating number of channels
 *
 * Revision 1.19  1997/08/17 12:35:36  pekangas
 * Changed command Z to be the music synchronization command
 *
 * Revision 1.18  1997/07/31 10:56:43  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.17  1997/07/15 12:18:54  pekangas
 * Fixed to compile and link with Visual C.
 * All API functions are again exported properly
 *
 * Revision 1.16  1997/07/14 08:04:28  jpaana
 * Misc. changes for better IT-compatibility
 *
 * Revision 1.15  1997/07/05 14:18:53  pekangas
 * Fixed some signed/unsigned mismatches
 *
 * Revision 1.14  1997/06/30 10:59:51  jpaana
 * Major rewrite on some parts for better IT-support
 *
 * Revision 1.13  1997/06/22 18:27:12  jpaana
 * Might work on big-endian machines, have to try it sometime...
 *
 * Revision 1.12  1997/06/22 18:09:25  jpaana
 * Made samples independent from instruments. A rather fundamental change, but
 * necessary for .it-support in instrument mode.
 *
 * Revision 1.11  1997/06/18 08:09:29  jpaana
 * Another quick fix
 *
 * Revision 1.10  1997/06/17 19:19:19  jpaana
 * A bit better fix
 *
 * Revision 1.9  1997/06/17 19:12:50  jpaana
 * Temporary fix
 *
 * Revision 1.8  1997/06/11 14:47:11  jpaana
 * Some cosmetic fixes to keep Prolint happy
 *
 * Revision 1.7  1997/06/11 04:21:22  jpaana
 * Fixed bidir-loops
 *
 * Revision 1.6  1997/06/11 03:48:07  jpaana
 * Fixed to handle unsigned/signed samples gracefully
 *
 * Revision 1.5  1997/06/11 03:29:23  jpaana
 * Fixed crashes by the end of the song
 *
 * Revision 1.4  1997/06/11 03:12:15  jpaana
 * IT-support made somewhat listenable...
 *
 * Revision 1.3  1997/06/11 00:03:00  jpaana
 * CVS fixes
 *
 *
*/

