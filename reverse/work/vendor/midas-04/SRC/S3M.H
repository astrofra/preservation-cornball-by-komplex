/*      S3M.H
 *
 * Scream Tracker 3 Module Player, v1.23
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/



/****************************************************************************\
*       struct s3mHeader
*       ----------------
* Description:  Scream Tracker 3 module file header
\****************************************************************************/

typedef struct
{
	char		name[28];				/* song name */
	uchar		num1A;					/* 0x1A */
	uchar		type;					/* file type */
	ushort		unused1;
	ushort		songLength; 			/* number of orders */
	ushort		numInsts;				/* number of instruments */
	ushort		numPatts;				/* number of patterns */
	struct								/* flags: */
	{
		int 	st2Vibrato : 1; 		/* Scream Tracker 2 vibrato */
		int 	st2Tempo : 1;			/* Scream Tracker 2 tempo */
		int 	ptSlides : 1;			/* ProTracker slides */
		int 	zeroVolOpt : 1; 		/* 0-volume optimizations */
		int 	ptLimits : 1;			/* ProTracker limits */
		int 	filter : 1; 			/* Enable filter / sfx */
        int     fastVolSlide : 1;       /* Fast volume slides ( < ST3.2) */
        int     unused : 9;
	} flags;
	ushort		trackerVer; 			/* tracker version */
	ushort		formatVer;				/* file format version */
	char		SCRM[4];				/* "SCRM" */
	uchar		masterVol;				/* master volume */
	uchar		speed;					/* initial speed */
	uchar		tempo;					/* initial tempo */
	uchar		masterMult; 			/* master multiplier (bits 0-3),
										   stereo (bit 4) */
    uchar       ultraClicks;            /* Ultraclick removal info */
    uchar       panningMagic;           /* 0FCh if panning infos exist */
    uchar       unused2[8];
    ushort      special;                /* pointer to special custom data */
	uchar		chanSettings[32];		/* channel settings */

} s3mHeader;



/****************************************************************************\
*       struct s3mInstHdr
*       -----------------
* Description:  Scream Tracker 3 module instrument file header
\****************************************************************************/

typedef struct
{
	uchar		type;					/* instrument type */
	char		dosName[12];			/* DOS filename (8+3) */
	char		zero;					/* 0 */
	ushort		samplePtr;				/* paragraph ptr to sample data */
	ulong		length; 				/* sample length */
	ulong		loopStart;				/* sample loop start */
	ulong		loopEnd;				/* sample loop end */
	uchar		volume; 				/* volume */
	uchar		disk;					/* instrument disk number */
	uchar		pack;					/* sample packing info (0 = raw,
										   1 = DP30ADPCM1) */
	uchar		flags;					/* bit0 = loop, bit1 = stereo,
										   bit2 = 16-bit */
	ulong		c2Rate; 				/* C2 sampling rate */
	ulong		unused;
	ushort		gusPos; 				/* position in GUS memory / 32 */
	ushort		int512;
	ulong		intLastUsed;
    char        iname[28];              /* instrument name */
    char        SCRS[4];                /* "SCRS" if sample */
} s3mInstHdr;



/****************************************************************************\
*       struct s3mChannel
*       -----------------
* Description:  Scream Tracker 3 module player internal channel structure
\****************************************************************************/

typedef struct
{
    uchar   note;
    uchar   inst;
    uchar   vol;
    uchar   cmd;
    uchar   info;
    uchar   flags;

    uchar   sample;
    uchar   volume;

    ushort  period;
    uchar   snote;
    uchar   preinfo;
    ushort  toperi;
    uchar   notepsp;
    uchar   retrigc;

    uchar   status;                    /* Includes bits like volume changed */

    uchar   vibcmd;
    uchar   vibpos;

    uchar   volbar;

    uchar   trefl;
    uchar   trecnt;
} s3mChannel;



extern ModulePlayer far GLOBALVAR mpS3M;  /* Scream Tracker 3 Module Player */


#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************\
*
* Function:     int s3mLoadModule(char *fileName, SoundDevice *SD,
*                   int (*SaveSampleInfo)(ushort sdInstHandle, uchar *sample,
*                   ushort slength, ushort loopStart, ushort loopEnd),
*                   mpModule **module);
*
* Description:  Loads a Scream Tracker 3 module into memory
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

int CALLING s3mLoadModule(char *fileName, SoundDevice *SD,
    int CALLING (*SaveSampleInfo)(ushort sdInstHandle, uchar *sample,
    ushort slength, ushort loopStart, ushort loopEnd), mpModule **module);




/****************************************************************************\
*
* Function:     int s3mFreeModule(mpModule *module, SoundDevice *SD);
*
* Description:  Deallocates a Scream Tracker 3 module
*
* Input:        mpModule *module        module to be deallocated
*               SoundDevice *SD         Sound Device that has stored the
*                                       samples
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING s3mFreeModule(mpModule *module, SoundDevice *SD);




/****************************************************************************\
*
* Function:     int s3mDetectChannels(mpModule *s3m, ushort *numChns);
*
* Description:  Detects the number of channels in a Scream Tracker 3 module
*
* Input:        mpModule *s3m           pointer to module structure
*               ushort *numChns         pointer to channel number variable
*
* Returns:      MIDAS error code.
*               Number of channels in module stored in *numChns.
*
\****************************************************************************/

int CALLING s3mDetectChannels(mpModule *s3m, ushort *numChns);



/***************************************************************************\
*
* Function:     int s3mFindUsedInsts(mpModule *s3m, ushort *used);
*
* Description:  Finds which instruments are used in a Scream Tracker 3 module.
*
* Input:        mpModule *s3m           Pointer to module structure. At least
*                                       the patterns must have been loaded
*               uchar    *used          Pointer to instruments used array -
*                                       one byte per instrument. Set to 1 if
*                                       instrument is used, 0 if not.
*
* Returns:      MIDAS error code.
*
\***************************************************************************/

int CALLING s3mFindUsedInsts(mpModule *s3m, uchar *used);



int CALLING s3mIdentify(uchar *header, int *recognized);
int CALLING s3mInit(SoundDevice *SD);
int CALLING s3mClose(void);
int CALLING s3mPlayModule(mpModule *module, ushort firstSDChannel,
    ushort numSDChannels, ushort loopStart, ushort loopEnd);
int CALLING s3mStopModule(void);
int CALLING s3mSetUpdRateFunct(void (*SetUpdRate)(ushort updRate));
int CALLING s3mPlay(void);
int CALLING s3mSetPosition(ushort pos);
int CALLING s3mGetInformation(mpInformation *info);


#ifdef __cplusplus
}
#endif


/****************************************************************************\
*       enum s3mFunctIDs
*       ----------------
* Description:  ID numbers for Scream Tracker 3 Module Player functions
\****************************************************************************/

enum s3mFunctIDs
{
    ID_s3mIdentify = ID_s3m,
    ID_s3mInit,
    ID_s3mClose,
    ID_s3mLoadModule,
    ID_s3mFreeModule,
    ID_s3mPlayModule,
    ID_s3mStopModule,
    ID_s3mSetUpdRateFunct,
    ID_s3mPlay,
    ID_s3mSetPosition,
    ID_s3mGetInformation,
    ID_s3mDetectChannels,
    ID_s3mFindUsedInsts
};

