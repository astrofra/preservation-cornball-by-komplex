/*      MTM.H
 *
 * Multitracker Module Player, v1.04
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#ifndef __MTM_H
#define __MTM_H


/****************************************************************************\
*       struct mtmInstHdr
*       -----------------
* Description:  Multitracker module instrument header
\****************************************************************************/

typedef struct
{
    char        iName[22];
    ulong       sLength;
    ulong       loopStart;
    ulong       loopEnd;
    uchar       fineTune;
    uchar       volume;
    struct      attributes
    {
        int     bits : 1;
        int     insused : 7;
    } attr;
} mtmInstHdr;



/****************************************************************************\
*       struct mtmHdr
*       -------------
* Description:  Multitracker module file header
\****************************************************************************/

typedef struct
{
    char        sign[3];
    uchar       version;
    char        sName[20];
    ushort      numTracks;
    uchar       lastPattern;
    uchar       lastOrder;
    ushort      lenComment;
    uchar       numInsts;
    uchar       attribute;
    uchar       beatsPerTrack;
    uchar       amountTracks;
    uchar       panPositions[32];
    mtmInstHdr  instruments[];
} mtmHdr;



/****************************************************************************\
*       struct mtmChannel
*       -----------------
* Description:  Multitracker Module Player internal channel structure
\****************************************************************************/

typedef struct
{
    uchar       note;
    uchar       inst;
    uchar       cmd;
    uchar       info;
    uchar       comp;

    uchar       sample;
    uchar       volume;

    ushort      period;
    ushort      snote;
    uchar       loff;
    uchar       coff;
    ushort      toperi;
    uchar       notepsp;
    uchar       retrigc;

    uchar       status;

    uchar       vibcmd;
    uchar       vibpos;

    uchar       trecmd;
    uchar       trepos;

    uchar       volbar;
    ushort      playoff;
} mtmChannel;



extern ModulePlayer far GLOBALVAR   mpMTM;  /* Multitracker Module Player */



#ifdef __cplusplus
extern "C" {
#endif


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

int CALLING mtmLoadModule(char *fileName, SoundDevice *SD,
    int CALLING (*SaveSampleInfo)(ushort sdInstHandle, uchar *sample,
    ushort slength, ushort loopStart, ushort loopEnd), mpModule **module);



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

int CALLING mtmFreeModule(mpModule *module, SoundDevice *sd);


int CALLING mtmIdentify(uchar *header, int *recognized);
int CALLING mtmInit(SoundDevice *SD);
int CALLING mtmClose(void);
int CALLING mtmPlayModule(mpModule *module, ushort firstSDChannel,
    ushort numSDChannels, ushort loopStart, ushort loopEnd);
int CALLING mtmStopModule(void);
int CALLING mtmSetUpdRateFunct(void (*SetUpdRate)(ushort updRate));
int CALLING mtmPlay(void);
int CALLING mtmSetPosition(ushort pos);
int CALLING mtmGetInformation(mpInformation **info);



/****************************************************************************\
*       enum mtmFunctIDs
*       ----------------
* Description:  ID numbers for Multitracker Module Player functions
\****************************************************************************/

enum mtmFunctIDs
{
    ID_mtmIdentify = ID_mtm,
    ID_mtmInit,
    ID_mtmClose,
    ID_mtmLoadModule,
    ID_mtmFreeModule,
    ID_mtmPlayModule,
    ID_mtmStopModule,
    ID_mtmSetUpdRateFunct,
    ID_mtmPlay,
    ID_mtmSetPosition,
    ID_mtmGetInformation,
    ID_mtmConvertSample,
    ID_mtmConvertTrack
};





#ifdef __cplusplus
}
#endif


#endif
