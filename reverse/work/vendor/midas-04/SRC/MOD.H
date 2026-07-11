/*      MOD.H
 *
 * ProTracker Module Player, v1.36
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#ifndef __MOD_H
#define __MOD_H


/****************************************************************************\
*       struct modInstHdr
*       -----------------
* Description:  Protracker module instrument header. Note that all 16-bit
*               fields are big-endian.
\****************************************************************************/

typedef struct
{
    char        iname[22];              /* instrument name */
    ushort      slength;                /* sample length */
    uchar       finetune;               /* sample finetune value */
    uchar       volume;                 /* sample default volume */
    ushort      loopStart;              /* sample loop start, in words */
    ushort      loopLength;             /* sample loop length, in words */
} modInstHdr;



/****************************************************************************\
*       struct modHeader
*       ----------------
* Description:  Protracker module file header
\****************************************************************************/

typedef struct
{
    char        songName[20];           /* song name */
    modInstHdr  instruments[31];        /* instrument headers */
    uchar       songLength;             /* song length */
    uchar       unused;                 /* unused by Protracker, used to be
                                           song restart position */
    uchar       orders[128];            /* pattern playing orders */
    char        sign[4];                /* module signature */
} modHeader;


/****************************************************************************\
*       struct modChannel
*       -----------------
* Description:  Protracker Module Player internal channel structure
\****************************************************************************/

typedef struct
{
    uchar   note;
    uchar   inst;
    uchar   cmd;
    uchar   info;
    uchar   comp;

    uchar   sample;
    uchar   volume;

    ushort  period;
    ushort  sNote;
    uchar   lOff;
    uchar   cOff;
    ushort  toPeri;
    uchar   notePSp;
    uchar   retrigC;

    uchar   status;

    uchar   vibCmd;
    uchar   vibPos;

    uchar   treCmd;
    uchar   trePos;

    uchar   volBar;

    uchar   loopPos;
    uchar   loopCnt;

    ushort  playOff;
} modChannel;



extern ModulePlayer far GLOBALVAR mpMOD;        /* Protracker Module Player */



#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************\
*
* Function:     int modLoadModule(char *fileName, SoundDevice *SD,
*                   int (*SaveSampleInfo)(ushort sdInstHandle, uchar *sample,
*                   ushort slength, ushort loopStart, ushort loopEnd),
*                   mpModule **module);
*
* Description:  Loads a Protracker module into memory
*
* Input:        char *fileName          name of module file to be loaded
*               SoundDevice *SD         Sound Device which will store the
*                                       samples
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

int CALLING modLoadModule(char *fileName, SoundDevice *SD,
    int CALLING (*SaveSampleInfo)(ushort sdInstHandle, uchar *sample,
    ushort slength, ushort loopStart, ushort loopEnd), mpModule **module);




/****************************************************************************\
*
* Function:     int modFreeModule(mpModule *module, SoundDevice *SD);
*
* Description:  Deallocates a Protracker module
*
* Input:        mpModule *module        module to be deallocated
*               SoundDevice *SD         Sound Device that has stored the
*                                       samples
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING modFreeModule(mpModule *module, SoundDevice *sd);


int CALLING modConvertSample(uchar *sample, ushort length);
int CALLING modConvertPattern(void *track, ushort numChans, ushort *trackLen,
            ushort *flags);

int CALLING modIdentify(uchar *header, int *recognized);
int CALLING modInit(SoundDevice *SD);
int CALLING modClose(void);
int CALLING modPlayModule(mpModule *module, ushort firstSDChannel,
    ushort numSDChannels, ushort loopStart, ushort loopEnd);
int CALLING modStopModule(void);
int CALLING modSetUpdRateFunct(void (*SetUpdRate)(ushort updRate));
int CALLING modPlay(void);
int CALLING modSetPosition(ushort pos);
int CALLING modGetInformation(mpInformation **info);



/****************************************************************************\
*       enum modFunctIDs
*       ----------------
* Description:  ID numbers for Protracker Module Player functions
\****************************************************************************/

enum modFunctIDs
{
    ID_modIdentify = ID_mod,
    ID_modInit,
    ID_modClose,
    ID_modLoadModule,
    ID_modFreeModule,
    ID_modPlayModule,
    ID_modStopModule,
    ID_modSetUpdRateFunct,
    ID_modPlay,
    ID_modSetPosition,
    ID_modGetInformation,
    ID_modConvertSample,
    ID_modConvertPattern
};





#ifdef __cplusplus
}
#endif


#endif
