/*      MTMLOAD.C
 *
 * Multitracker Module loader
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#include "mglobals.h"
#include "mmem.h"
#include "file.h"
#include "sdevice.h"
#include "mplayer.h"
#include "mtm.h"
#ifndef NOEMS
#include "ems.h"
#endif
#include "vu.h"
#include "mutils.h"

#ifndef NULL
    #define NULL 0L
#endif




/* Size of temporary memory area used for avoiding memory fragmentation
   if EMS is used or in protected mode */
#define TEMPSIZE 8192

/* Pass error code in variable "error" on, used in mtmLoadModule(). */
#define MTMLOADPASSERROR { mtmLoadError(SD); PASSERROR(ID_mtmLoadModule) }


/****************************************************************************\
*       Module loader buffers and file pointer. These variables are static
*       instead of local so that a separate deallocation can be used which
*       will be called before exiting in error situations
\****************************************************************************/
static fileHandle f;
static int      fileOpened;
static mpModule *mmod;
static uchar    *smpBuf;
static mtmInstHdr *instHdrs;
static ushort   *seqData;
static void     *tempmem;



/****************************************************************************\
*
* Function:     int mtmFreeModule(mpModule *module, SoundDevice *SD);
*
* Description:  Deallocates a Multitracker module
*
* Input:        mpModule *module        module to be deallocated
*               SoundDevice *SD         Sound Device that has stored the
*                                       samples
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mtmFreeModule(mpModule *module, SoundDevice *SD)
{
    int         i, error;

    if ( module == NULL )               /* valid module? */
    {
        ERROR(errUndefined, ID_mtmFreeModule);
        return errUndefined;
    }


    /* deallocate pattern orders if allocated: */
    if ( module->orders != NULL )
        if ( (error = memFree(module->orders)) != OK )
            PASSERROR(ID_mtmFreeModule)

    /* deallocate instrument used flags is allocated: */
    if ( module->instsUsed != NULL )
        if ( (error = memFree(module->instsUsed)) != OK )
            PASSERROR(ID_mtmFreeModule)

    if ( module->insts != NULL )        /* instruments? */
    {
        for ( i = 0; i < module->numInsts; i++ )
        {
            /* If the instrument has been added to Sound Device, remove
               it, otherwise just deallocate the sample if allocated */

            if ( (module->insts[i].sdInstHandle != 0) && (SD != NULL) )
            {
                if ( (error = SD->RemInstrument(
                    module->insts[i].sdInstHandle)) != OK )
                    PASSERROR(ID_mtmFreeModule)
            }
            else
                if ( module->insts[i].sample != NULL )
                    if ( (error = memFree(module->insts[i].sample)) != OK )
                        PASSERROR(ID_mtmFreeModule)
        }
        /* deallocate instrument structures: */
        if ( (error = memFree(module->insts)) != OK )
            PASSERROR(ID_mtmFreeModule)
    }

    if ( module->patterns != NULL )
    {
        for ( i = 0; i < module->numPatts; i++ )
        {
            if ( module->patterns[i] != NULL )
            {
                /* if the pattern has been allocate, deallocate it - either
                   from conventional memory or from EMS */

#ifndef NOEMS
                if ( useEMS == 1 )
                {
                    if ( (error = emsFree((emsBlock*) module->patterns[i]))
                        != OK )
                        PASSERROR(ID_mtmFreeModule)
                }
                else
#endif
                    if ( (error = memFree(module->patterns[i])) != OK )
                        PASSERROR(ID_mtmFreeModule)
            }
        }
        /* deallocate pattern pointers: */
        if ( (error = memFree(module->patterns)) != OK )
            PASSERROR(ID_mtmFreeModule)
    }

    /* deallocate the module: */
    if ( (error = memFree(module)) != OK)
        PASSERROR(ID_mtmFreeModule)

    return OK;
}



/****************************************************************************\
*
* Function:     void mtmLoadError(SoundDevice *SD)
*
* Description:  Stops loading the module, deallocates all buffers and closes
*               the file.
*
* Input:        SoundDevice *SD         Sound Device that has been used for
*                                       loading.
*
\****************************************************************************/

static void mtmLoadError(SoundDevice *SD)
{
    /* Close file if opened. Do not process errors. */
    if ( fileOpened )
        if ( fileClose(f) != OK )
            return;

    /* Attempt to deallocate module if allocated. Do not process errors. */
    if ( mmod != NULL )
        if ( mtmFreeModule(mmod, SD) != OK )
            return;

    /* Deallocate buffers if allocated. Do not process errors. */
    if ( instHdrs != NULL )
        if ( memFree(instHdrs) != OK )
            return;
    if ( seqData != NULL )
        if ( memFree(seqData) != OK )
            return;
    if ( smpBuf != NULL )
        if ( memFree(smpBuf) != OK )
            return;
    if ( tempmem != NULL )
        if ( memFree(tempmem) != OK )
            return;
}



/****************************************************************************\
*
* Function:     int mtmLoadModule(char *fileName, SoundDevice *SD,
*                   int (*SaveSampleInfo)(ushort sdInstHandle, uchar *sample,
*                   ushort slength, ushort loopStart, ushort loopEnd),
*                   mpModule **module);
*
* Description:  Loads a Multitracker module into memory
*
* Input:        char *fileName          name of module file to be loaded
*               SoundDevice *SD         Sound Device which will store the
*                                       samples. NULL if the samples should
*                                       not be added to a Sound Device, but
*                                       should be left in conventional memory
*                                       instead.
*               int (*SaveSampleInfo)() Pointer to sample information saving
*                                       function. sdInstHandle = Sound Device
*                                       instrument handle, sample = pointer to
*                                       sample data, slength = sample length,
*                                       loopStart = sample loop start,
*                                       loopEnd = sample loop end. The
*                                       function must return a MIDAS error
*                                       code. NULL if no such function is
*                                       used.
*               mpModule **module       pointer to variable which will store
*                                       the module pointer.
*
* Returns:      MIDAS error code.
*               Pointer to module structure is stored in *module.
*
* Notes:        The only practical use at this point for SaveSampleInfo() are
*               the real VU-meters. To load a module and add the prepare the
*               VU meter information point SaveSampleInfo to vuPrepare().
*
\****************************************************************************/

int CALLING mtmLoadModule(char *fileName, SoundDevice *SD,
    int CALLING (*SaveSampleInfo)(ushort sdInstHandle, uchar *sample,
    ushort slength, ushort loopStart, ushort loopEnd), mpModule **module)
{
    int             error;              /* MIDAS error code */
    mtmHdr          mtmh;
    mtmInstHdr      *mtmi;
    mpInstrument    *inst;
    mpPattern       *pattData;

    ushort          trackLen;

    int             i, c, r;
    ushort          numPatts;
    ushort          chans;
    ulong           foffset;
    ulong           toffset;
    ulong           soffset;

    ushort          slength;            /* sample length */
    ushort          loopStart;          /* sample loop start */
    ushort          loopLength;         /* sample loop length */

    ulong           maxSmpLength;
    uchar           instx;

    void            *p;
    uchar           *temppi;

    /* point buffers to NULL and set fileOpened to 0 so that mtmLoadError()
       can be called at any point: */
    fileOpened = 0;
    mmod = NULL;
    instHdrs = NULL;
    seqData = NULL;
    smpBuf = NULL;
    tempmem = NULL;


    /* Open module file: */
    if ( (error = fileOpen(fileName, fileOpenRead, &f)) != OK )
        MTMLOADPASSERROR

    /* Allocate memory for the module structure: */
    if ( (error = memAlloc(sizeof(mpModule), (void**) &mmod)) != OK )
        MTMLOADPASSERROR

    mmod->orders = NULL;                 /* clear module structure so that */
    mmod->insts = NULL;                  /* it can be deallocated with */
    mmod->patterns = NULL;               /* modFree() at any point */
    mmod->instsUsed = NULL;

    mmod->MP = &mpMTM;                  /* point MP field to module player */

    /* read Multitracker module header: */
    if ( (error = fileRead(f, &mtmh, sizeof(mtmHdr))) != OK )
        MTMLOADPASSERROR

    /* Check MTM sign */
    if ( !mMemEqual(&mtmh.sign[0], "MTM", 3) )
    {
        ERROR(errInvalidModule, ID_mtmLoadModule);
        mtmLoadError(SD);
        return errInvalidModule;
    }

    mmod->numChans = mtmh.amountTracks;     /* store number of channels */
    chans = mmod->numChans;

    mMemCopy(&mmod->songName[0], &mtmh.sName[0], 20); /* copy song name */
    mmod->songName[20] = 0;                 /* force terminating '\0' */
    mmod->songLength = mtmh.lastOrder + 1;  /* copy song length */
    mmod->numInsts = mtmh.numInsts;         /* set number of instruments */

    mMemCopy(&mmod->ID, &mtmh.sign[0], 4);  /* copy module signature */
    mmod->IDnum = idMTM;                    /* Multitracker module */

    /* Set proper channel panning values to module structure: */
    for ( i = 0; i < chans; i++ )
    {
        r = mtmh.panPositions[i] - 8;
        if ( r >= 0 )
            r++;
        mmod->chanSettings[i] = r * 8;
    }

    numPatts = mtmh.lastPattern + 1;
    mmod->numPatts = numPatts * chans;      /* store number of tracks */

    /* allocate memory for instrument headers: */
    if ( (error = memAlloc(mmod->numInsts * sizeof(mtmInstHdr),
        (void**) &instHdrs)) != OK )
        MTMLOADPASSERROR

    /* seek to instrument headers */
    if ( (error = fileSeek(f, sizeof(mtmHdr), fileSeekAbsolute)) != OK )
        MTMLOADPASSERROR

    /* read instrument data: */
    if ( (error = fileRead(f, instHdrs, mmod->numInsts * sizeof(mtmInstHdr)))
        != OK )
        MTMLOADPASSERROR

    /* allocate memory for pattern orders: */
    if ( (error = memAlloc(mmod->songLength, (void**) &mmod->orders)) != OK )
        MTMLOADPASSERROR

    /* seek to pattern orders: */
    foffset = sizeof(mtmHdr) + mmod->numInsts * sizeof(mtmInstHdr);
    if ( (error = fileSeek(f, foffset, fileSeekAbsolute)) != OK )
        MTMLOADPASSERROR

    /* read pattern playing orders: */
    if ( (error = fileRead(f, mmod->orders, mmod->songLength)) != OK )
        MTMLOADPASSERROR

    /* allocate memory for pattern (actually track) pointers */
    if ( (error = memAlloc((mmod->numPatts) * sizeof(mpPattern*),
        (void**) &mmod->patterns)) != OK )
        MTMLOADPASSERROR

    for ( i = 0; i < mmod->numPatts; i++ ) /* point all unallocated patterns */
        mmod->patterns[i] = NULL;          /* to NULL for safety */


    /* allocate memory for instrument used flags: */
    if ( (error = memAlloc(mmod->numInsts, (void**) &mmod->instsUsed)) != OK )
        MTMLOADPASSERROR

    /* Mark all instruments unused: */
    for ( i = 0; i < mmod->numInsts; i++ )
        mmod->instsUsed[i] = 0;

    /* allocate memory for instrument structures: */
    if ( (error = memAlloc(mmod->numInsts * sizeof(mpInstrument),
        (void**) &mmod->insts)) != OK )
        MTMLOADPASSERROR

    /* clear all instruments and find maximum instrument length: */
    maxSmpLength = 0;
    for ( i = 0; i < mmod->numInsts; i++ )
    {
        mmod->insts[i].sample = NULL;
        mmod->insts[i].sdInstHandle = 0;
        if ( maxSmpLength < instHdrs[i].sLength )
            maxSmpLength = instHdrs[i].sLength;
    }

    /* check that none of the instruments is too long: */
    if ( maxSmpLength > SMPMAX )
    {
        ERROR(errInvalidInst, ID_mtmLoadModule);
        mtmLoadError(SD);
        return errInvalidInst;
    }

    trackLen = 3 * mtmh.beatsPerTrack;
    toffset = foffset + 128;                    /* Track offset */
    foffset = toffset + mtmh.numTracks * 192;   /* Sequencing data offset */
    soffset = foffset + 2 * 32 * numPatts + mtmh.lenComment;
                                                /* Sample offset */

    /* Allocate memory for sequencing data: */
    if ( (error = memAlloc(2 * 32 * mmod->songLength, (void**) &seqData))
        != OK )
        MTMLOADPASSERROR

    /* seek to sequencing data: */
    if ( (error = fileSeek(f, foffset, fileSeekAbsolute)) != OK )
        MTMLOADPASSERROR

    /* read sequencing data: */
    if ( (error = fileRead(f, seqData, 2 * 32 * mmod->songLength)) != OK )
        MTMLOADPASSERROR


    /* Load all patterns: */

    for ( i = 0; i < numPatts; i++ )
    {
        /* Load all tracks of the pattern */
        for ( c = 0; c < chans; c++ )
        {
            if ( (r = seqData[i * 32 + c]) != 0)
            {
#ifndef NOEMS
                if ( useEMS == 1 )          /* is EMS memory used? */
                {
                    /* Allocate EMS memory for track: */
                    if ( (error = emsAlloc(sizeof(mpModule) + trackLen,
                        (emsBlock**) &p)) != OK )
                        MTMLOADPASSERROR

                    /* map EMS block to conventional memory and point pattData
                        to the memory area: */
                    if ( (error = emsMap((emsBlock*) p, (void**) &pattData))
                        != OK )
                        MTMLOADPASSERROR
                }
                else
                {
#endif
                    /* EMS memory not in use - allocate conventional memory */
                    if ( (error = memAlloc(sizeof(mpModule) + trackLen, &p))
                        != OK )
                        MTMLOADPASSERROR

                    pattData = p;
#ifndef NOEMS
                }
#endif

                temppi = (uchar*) &pattData->data[0];
                pattData->length = trackLen;


                /* load track data: */

                foffset = toffset + trackLen * (r - 1);
                if ( (error = fileSeek(f, foffset, fileSeekAbsolute)) != OK )
                    MTMLOADPASSERROR
                if ( (error = fileRead(f, &pattData->data[0], trackLen))
                    != OK )
                    MTMLOADPASSERROR

                /* check used instruments: */
                for ( r = 0; r < trackLen; r += 3)
                {
                    instx = (((temppi[r] & 3) << 4) |
                    ((temppi[r + 1] >> 4 & 0xF)));
                    if ((instx > 0) && (instx <= mmod->numInsts))
                        mmod->instsUsed[instx - 1] = 1;
                }
                mmod->patterns[i * chans + c] = p;
            }
            else
            {
                mmod->patterns[i * chans + c] = NULL;
            }
        }

    }

    /* deallocate pattern loading buffers: */
    if ( (error = memFree(seqData)) != OK )
        MTMLOADPASSERROR
    seqData = NULL;


    /* If EMS is used, allocate TEMPSIZE bytes of memory before the sample
       buffer and deallocate it after allocating the sample buffer to
       minimize memory fragmentation. This is not necessary if the samples
       will not be added to a Sound Device. */
#ifndef NOEMS
    if ( useEMS )
    {
#endif
        if ( SD != NULL )
        {
            if ( (error = memAlloc(TEMPSIZE, &tempmem)) != OK )
            {
                MTMLOADPASSERROR
            }
        }
#ifndef NOEMS
    }
#endif

    /* allocate memory for sample loading buffer if needed: */
    if ( SD != NULL )
    {
        if ( (error = memAlloc(maxSmpLength, (void**) &smpBuf)) != OK )
        {
            MTMLOADPASSERROR
        }
    }

#ifndef NOEMS
    if ( useEMS )
    {
#endif
        if ( SD != NULL )
        {
            if ( (error = memFree(tempmem)) != OK )
            {
                MTMLOADPASSERROR
            }
        }
        tempmem = NULL;
#ifndef NOEMS
    }
#endif

    for ( i = 0; i < mmod->numInsts; i++ )
    {
        inst = &mmod->insts[i];          /* point inst to current instrument
                                            structure */

        mtmi = &instHdrs[i];             /* point mtmi to current Multitracker
                                            module instrument */

        /* Copy sample length, loop start and loop end. */

        slength = mtmi->sLength;
        loopStart = mtmi->loopStart;
        loopLength = mtmi->loopEnd;

        mMemCopy(&inst->iname[0], &mtmi->iName[0], 22);  /* copy inst name */
        inst->iname[22] = 0;            /* force terminating '\0' */
        inst->loopStart = loopStart;    /* copy sample loop start */
        inst->loopEnd = loopLength;     /* sample loop end */

        /* If sample loop end is past byte 2, the sample is looping */

        if (inst->loopEnd > 2)
        {
            inst->looping = 1;
            inst->length = inst->loopEnd;  /* use loop end as sample length */
        }                               /* if looping to avoid loading */
        else                            /* unnecessary sample data */
        {
            inst->looping = 0;
            inst->loopEnd = 0;          /* set loop end to 0 if no loop */
            inst->length = slength;     /* use sample length */
        }

        inst->volume = mtmi->volume;        /* copy default volume */
        inst->finetune = mtmi->fineTune;    /* copy finetune */

        if (mmod->instsUsed[i] == 1)        /* if not used, don't load */
        {
            if ( inst->length != 0 )        /* is there a sample for this inst? */
            {
                if ( SD == NULL )
                {
                    /* No Sound Device used - allocate memory for the sample
                       and point inst->sample to the memory area: */
                    if ( (error = memAlloc(inst->length, (void**) &smpBuf)) != OK )
                        MTMLOADPASSERROR
                    inst->sample = smpBuf;
                }
                else
                {
                    /* The instruments will be added to a Sound Device - load
                       the sample to a buffer (allocated before) and point
                       inst->sample to NULL: */
                    inst->sample = NULL;
                }

                /* seek to sample start position: */
                if ( (error = fileSeek(f, soffset, fileSeekAbsolute)) != OK )
                    MTMLOADPASSERROR

                /* read sample to buffer: */
                if ( (error = fileRead(f, smpBuf, inst->length)) != OK )
                    MTMLOADPASSERROR
            }
            else
                inst->sample = NULL;

            /* add the instrument to Sound Device: */
            if ( SD != NULL )
            {
                error = SD->AddInstrument(smpBuf, smp8bit, inst->length,
                    inst->loopStart, inst->loopEnd, inst->volume, inst->looping,
                    1, &inst->sdInstHandle);
                if ( error != OK )
                {
                    MTMLOADPASSERROR
                }
            }

            /* Call SaveSampleInfo() if not NULL: */
            if ( SaveSampleInfo != NULL )
            {
                if ( (error = (*SaveSampleInfo)(inst->sdInstHandle, smpBuf,
                    inst->length, inst->loopStart, inst->loopEnd)) != OK )
                    MTMLOADPASSERROR
            }
        }
        else
            inst->sample = NULL;

        soffset += slength;             /* point foffset to next sample */
    }

    /* deallocate sample loading buffer: */
    if ( SD != NULL )
    {
        if ( (error = memFree(smpBuf)) != OK )
        {
            MTMLOADPASSERROR
        }
    }
    smpBuf = NULL;

    /* deallocate sample headers: */
    if ( (error = memFree(instHdrs)) != OK )
        MTMLOADPASSERROR
    instHdrs = NULL;

    if ( (error = fileClose(f)) != OK )
        MTMLOADPASSERROR
    fileOpened = 0;

    *module = mmod;                     /* return module ptr in *module */
    return OK;
}

/*



The following is a summary of the MultiTracker Module (MTM) fromat.  It is
intended for programmers who wish to support the format in any manner.  Note
that all effects are defined as the current Protracker effects standard. A
short summary of this standard is provided in the documentation file for the
Multitracker Module Editor.

ÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
Position³Length³Description
ÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
0       ³3     ³"MTM" file marker
3       ³BYTE  ³version number - upper nybble is major version #, lower is
        ³      ³                 minor version number
4       ³20    ³ASCIIZ songname
24      ³WORD  ³number of tracks saved
26      ³BYTE  ³last pattern number saved
27      ³BYTE  ³last order number to play (songlength-1)
28      ³WORD  ³length of extra comment field
30      ³BYTE  ³number of samples saved (NOS)
31      ³BYTE  ³attribute byte (currently defined as 0)
32      ³BYTE  ³beats per track
33      ³BYTE  ³number of tracks to be played back
34      ³32    ³voice pan positions
ÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
66      ³NOS*37³Instrument data:
ÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
        ³22    ³sample name
        ³DWORD ³sample length in bytes
        ³DWORD ³offset of beginning of sample loop in bytes
        ³DWORD ³offset of end of sample loop in bytes
        ³BYTE  ³finetune value
        ³BYTE  ³standard volume of sample
        ³BYTE  ³attribute byte: bit meaning
        ³      ³                0   0=8 bit sample 1=16 bit sample
        ³      ³                1-7 undefined (set to zero)
ÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
66+     ³128   ³Pattern order data
(NOS*37)³      ³
ÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
194+    ³trks* ³Track data:
(NOS*37)³192   ³Each track is saved independently and takes exactly 192 bytes.
        ³      ³The tracks are arranged as 64 consecutive 3-byte notes.  These
        ³      ³notes have the following format:
        ³      ³
        ³      ³
        ³      ³  BYTE 0   BYTE 1   BYTE 2
        ³      ³ ppppppii iiiieeee aaaaaaaa
        ³      ³
        ³      ³ p = pitch value (0=no pitch stated)
        ³      ³ i = instrument number (0=no instrument number)
        ³      ³ e = effect number
        ³      ³ a = effect argument
        ³      ³
ÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
194+    ³      ³Track sequencing data
NOS*37+ ³      ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
trks*192³(last pattern saved + 1)*32 WORDS³
ÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
        ³      ³The track sequencing data is really just a listing of which
        ³      ³track is used as which voice in each saved pattern.  This is
        ³      ³necessary since one track may be a part of many different
        ³      ³patterns. (not orders)  Doing this saves much of the memory
        ³      ³wasted in a normal MOD by repetition of certain tracks over
        ³      ³and over again throughout the file.
        ³      ³
        ³      ³Note that track zero is never saved, but always considered as
        ³      ³an empty track.  Therefore, track numbering for the saved
        ³      ³tracks really starts at one.
        ³      ³
        ³      ³The data is organized in sets of 32 voices.  First comes a
        ³      ³WORD representing which track is in pattern 0, voice 0.  The
        ³      ³next WORD is pattern 0, voice 1, etc.  This is repeated for
        ³      ³each pattern saved, giving a total track sequencing size of
        ³      ³32 WORDS per pattern saved.
        ³      ³
        ³      ³If your code uses MOD-style memory organization, you can still
        ³      ³play MTM's.  You merely jump to the track sequencing data, and
        ³      ³then load each pattern separately by jumping back and forth
        ³      ³between the track sequences and the actual track data.
ÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
194+    ³Header³Extra comment field
NOS*37+ ³says. ³(Length specified in the header)
trks*192ÀÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
+(last pattern saved + 1)*32*2      ³
ÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
194+    ³sample³Raw sample data
NOS*37+ ³length³(unsigned)
trks*192ÀÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
+(last pattern saved + 1)*32*2+     ³
length of extra comment field       ³
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

*/
