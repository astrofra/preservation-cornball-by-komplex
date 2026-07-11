/*      MIDAS.C
 *
 * Simplified MIDAS Sound System API
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "midas.h"



/****************************************************************************\
*      Global variables:
\****************************************************************************/

SoundDevice     *midasSD;               /* current Sound Device */
ModulePlayer    *midasMP;               /* current Module Player */

SoundDevice     *midasSoundDevices[NUMSDEVICES] =
    { &GUS,                             /* array of pointers to all Sound */
      &PAS,                             /* Devices, in numbering and */
      &WSS,                             /* detection order - GUS is SD #1 */
      &SB,                              /* and will be detected first */
      &NSND };

int             midasDisableEMS;        /* 1 if EMS usage is disabled
                                           (default 0) */
int             midasSDNumber;          /* Sound Device number (-1 for
                                           autodetect, default -1) */
int             midasSDPort;            /* Sound Device I/O port number
                                           (-1 for autodetect or SD default,
                                           default -1) */
int             midasSDIRQ;             /* Sound Device IRQ number (-1 for
                                           autodetect or SD default,
                                           default -1) */
int             midasSDDMA;             /* Sound Device DMA channel number
                                           (-1 for autodetect or SD default,
                                           default -1) */
int             midasSDCard;            /* Sound Device sound card type
                                           (-1 for autodetect or SD default,
                                           default -1) */
unsigned        midasMixRate;           /* Sound Device mixing rate */
unsigned        midasOutputMode;        /* Sound Device output mode force
                                           bits, default 0 (SD default) */
int             midasAmplification;     /* Forced amplification level or -1
                                           for SD default (default -1) */
int             midasChannels;          /* number of channels open or 0 if no
                                           channels have been opened using
                                           midasOpenChannels() */
int             midasPlayerNum;         /* timer music player number */

int             midasEMSInit;           /* is EMS heap manager initialized? */
int             midasTMRInit;           /* is TempoTimer initialized? */
int             midasTMRPlay;           /* is sound being played with timer?*/
int             midasSDInit;            /* is Sound Device initialized? */
int             midasSDChans;           /* are Sound Device channels open? */
int             midasMPInit;            /* is Module Player initialized? */
int             midasMPPlay;            /* is Module Player playing? */
int             midasTMRMusic;          /* is music being player with timer?*/





/****************************************************************************\
*
* Function:     void midasError(int errNum)
*
* Description:  Prints a MIDAS error message to stderr and exits to DOS
*
* Input:        int errNum              MIDAS error code
*
\****************************************************************************/

void CALLING midasError(int errNum)
{
    char        errmsg[60];

    midasClose();                       /* uninitialize MIDAS Sound System */
    mStrCopy(&errmsg[0], "MIDAS Error: ");
    mStrAppend(&errmsg[0], errorMsg[errNum]);
    errErrorExit(&errmsg[0]);           /* print error message */
}




/****************************************************************************\
*
* Function:     void midasUninitError(int errNum)
*
* Description:  Prints an error message to stderr and exits to DOS without
*               uninitializing MIDAS. This function should only be used
*               from midasClose();
*
* Input:        int errNum              MIDAS error code
*
\****************************************************************************/

void CALLING midasUninitError(int errNum)
{
    char        errmsg[85];

    mStrCopy(&errmsg[0], "FATAL MIDAS uninitialization failure: ");
    mStrAppend(&errmsg[0], errorMsg[errNum]);
    errErrorExit(&errmsg[0]);           /* print error message */
}




/****************************************************************************\
*
* Function:     void midasDetectSD(void)
*
* Description:  Attempts to detect a Sound Device. Sets the global variable
*               midasSD to point to the detected Sound Device or NULL if no
*               Sound Device was detected
*
\****************************************************************************/

void CALLING midasDetectSD(void)
{
    int         dsd;
    int         dResult;
    int         error;

    midasSD = NULL;                     /* no Sound Device detected yet */
    midasSDNumber = -1;
    dsd = 0;                            /* start from first Sound Device */

    /* search through Sound Devices until a Sound Device is detected: */
    while ( (midasSD == NULL) && (dsd < NUMSDEVICES) )
    {
        /* attempt to detect current SD: */
        if ( (error = (*midasSoundDevices[dsd]->Detect)(&dResult)) != OK )
            midasError(error);
        if ( dResult == 1 )
        {
            midasSDNumber = dsd;        /* Sound Device detected */
            /* point midasSD to this Sound Device: */
            midasSD = midasSoundDevices[dsd];
        }
        dsd++;                          /* try next Sound Device */
    }
}




/****************************************************************************\
*
* Function:     void midasInit(void);
*
* Description:  Initializes MIDAS Sound System
*
\****************************************************************************/

void CALLING midasInit(void)
{
    int         error, result;

#ifndef NOEMS
    if ( !midasDisableEMS )             /* is EMS usage disabled? */
    {
        /* Initialize EMS Heap Manager: */
        if ( (error = emsInit(&midasEMSInit)) != OK )
            midasError(error);

        /* was EMS Heap Manager initialized? */
        if ( midasEMSInit == 1 )
        {
            useEMS = 1;                 /* yes, use EMS memory */
        }
        else
        {
            useEMS = 0;                 /* no, do not use EMS memory */
        }
    }
    else
#endif
    {
        midasEMSInit = 0;
        useEMS = 0;                     /* EMS disabled - do not use it */
    }



    if ( midasSDNumber == -1 )      /* has a Sound Device been selected? */
    {
        midasDetectSD();            /* attempt to detect Sound Device */
        if ( midasSD == NULL )
            midasError(errSDFailure);
    }
    else
    {
        /* use selected Sound Device: */
        midasSD = midasSoundDevices[midasSDNumber];

        /* Sound Device number was forced, but if no I/O port, IRQ, DMA or
           sound card type has been set, try to autodetect the values for this
           Sound Device. If detection fails, use default values: */

        if ( (midasSDPort == -1) && (midasSDIRQ == -1) &&
            (midasSDDMA == -1) && (midasSDCard == -1) )
            if ( (error = midasSD->Detect(&result)) != OK )
                midasError(error);
    }

    if ( midasSDPort != -1 )            /* has an I/O port been selected? */
        midasSD->port = midasSDPort;    /* if yes, set it to Sound Device */
    if ( midasSDIRQ != -1 )             /* SD IRQ number? */
        midasSD->IRQ = midasSDIRQ;      /* if yes, set it to Sound Device */
    if ( midasSDDMA != -1 )             /* SD DMA channel number? */
        midasSD->DMA = midasSDDMA;
    if ( midasSDCard != -1 )            /* sound card type? */
        midasSD->cardType = midasSDCard;

    /* initialize TempoTimer: */
    if ( (error = tmrInit()) != OK )
        midasError(error);

    midasTMRInit = 1;                 /* TempoTimer initialized */

    /* initialize Sound Device: */
    if ( (error = midasSD->Init(midasMixRate, midasOutputMode)) != OK )
        midasError(error);

    midasSDInit = 1;                  /* Sound Device initialized */

    /* start playing sound using the timer: */
    if ( (error = tmrPlaySD(midasSD)) != OK )
        midasError(error);

    midasTMRPlay = 1;
}



/****************************************************************************\
*
* Function:     void midasClose(void)
*
* Description:  Uninitializes MIDAS Sound System
*
\****************************************************************************/

void CALLING midasClose(void)
{
    int         error;

    /* if music is being played with timer, stop it: */
    if ( midasTMRMusic )
    {
        if ( (error = midasMP->SetUpdRateFunct(NULL)) != OK )
            midasUninitError(error);
        if ( (error = tmrStopMusic(midasPlayerNum)) != OK )
            midasUninitError(error);
        midasTMRMusic = 0;
    }

    /* if Module Player is playing, stop it: */
    if ( midasMPPlay )
    {
        if ( (error = midasMP->StopModule()) != OK )
            midasUninitError(error);
        midasMPPlay = 0;
    }

    /* if Module Player has been initialized, uninitialize it: */
    if ( midasMPInit )
    {
        if ( (error = midasMP->Close()) != OK )
            midasUninitError(error);
        midasMPInit = 0;
        midasMP = NULL;
    }

    /* if Sound Device channels are open, close them: */
    if ( midasSDChans )
    {
        if ( (error = midasSD->CloseChannels()) != OK )
            midasUninitError(error);
        midasSDChans = 0;
        midasChannels = 0;
    }

    /* if sound is being played, stop it: */
    if ( midasTMRPlay )
    {
        if ( (error = tmrStopSD()) != OK )
            midasUninitError(error);
        midasTMRPlay = 0;
    }

    /* if Sound Device is initialized, uninitialize it: */
    if ( midasSDInit )
    {
        if ( (error = midasSD->Close()) != OK )
            midasUninitError(error);
        midasSDInit = 0;
        midasSD = NULL;
    }

    /* if TempoTimer is initialized, uninitialize it: */
    if ( midasTMRInit )
    {
        if ( (error = tmrClose()) != OK )
            midasUninitError(error);
        midasTMRInit = 0;
    }

#ifndef NOEMS
    /* if EMS Heap Manager is initialized, uninitialize it: */
    if ( midasEMSInit )
    {
        if ( (error = emsClose()) != OK )
            midasUninitError(error);
        midasEMSInit = 0;
    }
#endif
}




/****************************************************************************\
*
* Function:     void midasSetDefaults(void)
*
* Description:  Initializes MIDAS Sound System variables to their default
*               states. MUST be the first MIDAS function called.
*
\****************************************************************************/

void CALLING midasSetDefaults(void)
{
    midasEMSInit = 0;                   /* EMS heap manager is not
                                           initialized yet */
    midasTMRInit = 0;                   /* TempoTimer is not initialized */
    midasTMRPlay = 0;                   /* Sound is not being played */
    midasSDInit = 0;                    /* Sound Device is not initialized */
    midasSDChans = 0;                   /* Sound Device channels are not
                                           open */
    midasMPInit = 0;                    /* Module Player is not initialized */
    midasMPPlay = 0;                    /* Module Player is not playing */
    midasTMRMusic = 0;                  /* Music is not being played with
                                           timer */
    midasChannels = 0;                  /* No channels opened */

    ptTempo = 1;                        /* enable ProTracker BPM tempos */
    usePanning = 1;                     /* enable ProTracker panning cmds */
    surround = 0;                       /* disable surround to save GUS mem */
    extendedOctaves = 0;                /* Disable extended octaves */
    useVDS = 1;                         /* use VDS if found */

    midasDisableEMS = 0;                /* do not disable EMS usage */
    midasSDNumber = -1;                 /* no Sound Device forced */
    midasSDPort = -1;                   /* no I/O port forced */
    midasSDIRQ = -1;                    /* no IRQ number forced */
    midasSDDMA = -1;                    /* no DMA channel number forced */
    midasSDCard = -1;                   /* no sound card type forced */
    midasOutputMode = 0;                /* no output mode forced */
    midasMixRate = 44100;               /* attempt to use 44100Hz mixing
                                           rate */
    midasAmplification = -1;            /* use default amplification level */

    midasSD = NULL;                     /* point midasSD and midasMP to */
    midasMP = NULL;                     /* NULL for safety */
}



#ifndef NOLOADERS



/****************************************************************************\
*
* Function:     mpModule *midasLoadModule(char *fileName, ModulePlayer *MP,
*                   int (*SaveSampleInfo)(ushort sdInstHandle, uchar *sample,
*                   ushort slength, ushort loopStart, ushort loopEnd );
*
* Description:  Loads a module file into memory.
*
* Input:        char *fileName          Pointer to module file name
*               ModulePlayer *MP        Pointer to the Module Player which
*                                       will be used for loading the module
*               int (*SaveSampleInfo)() Pointer to sample information saving
*                                       function. sdInstHandle = Sound Device
*                                       instrument handle, sample = pointer to
*                                       sample data, slength = sample length,
*                                       loopStart = sample loop start,
*                                       loopEnd = sample loop end. The
*                                       function must return a MIDAS error
*                                       code. NULL if no such function is
*                                       used.
*
* Returns:      Pointer to the loaded module structure
*
* Notes:        The only practical use at this point for SaveSampleInfo() are
*               the real VU-meters. To load a module and add the prepare the
*               VU meter information use:
*                   module = midasLoadModule(fileName, MP, &vuPrepare);
*               Note that the definition of SaveSampleInfo matches exactly
*               the prototype of vuPrepare().
*
\****************************************************************************/

mpModule * CALLING midasLoadModule(char *fileName, ModulePlayer *MP,
    int CALLING (*SaveSampleInfo)(ushort sdInstHandle, uchar *sample,
    ushort slength, ushort loopStart, ushort loopEnd))
{
    mpModule    *module;
    int         error;

    /* load module: */
    if ( (error = MP->LoadModule(fileName, midasSD, SaveSampleInfo,
        (mpModule**) &module)) != OK )
        midasError(error);

    return module;
}




/****************************************************************************\
*
* Function:     void midasFreeModule(mpModule *module);
*
* Description:  Deallocates a module from memory
*
* Input:        mpModule *module        Pointer to module to be deallocated
*
\****************************************************************************/

void CALLING midasFreeModule(mpModule *module)
{
    ModulePlayer    *MP;
    int             error;

    MP = module->MP;
    if ( (error = MP->FreeModule(module, midasSD)) != OK )
        midasError(error);
}


#endif



/****************************************************************************\
*
* Function:     void midasOpenChannels(int numChans);
*
* Description:  Opens Sound Device channels for sound and music output.
*
* Input:        int numChans            Number of channels to open
*
* Notes:        Channels opened with this function can be used for sound
*               playing, and modules played with midasPlayModule() will be
*               played through the last of these channels. This function is
*               provided so that the same number of channels can be open
*               the whole time throughout the execution of the program,
*               keeping the volume level constant. Note that you must ensure
*               that you open enough channels for all modules, otherwise
*               midasPlayModule() will fail.
*
\****************************************************************************/

void CALLING midasOpenChannels(int numChans)
{
    int         error;

    midasChannels = numChans;

    /* open Sound Device channels: */
    if ( (error = midasSD->OpenChannels(numChans)) != OK )
        midasError(error);
    midasSDChans = 1;

    /* set amplification level if forced: */
    if ( midasAmplification != -1 )
    {
        if ( (error = midasSD->SetAmplification(midasAmplification)) != OK )
            midasError(error);
    }
}




/****************************************************************************\
*
* Function:     void midasCloseChannels(void);
*
* Description:  Closes Sound Device channels opened with midasOpenChannels().
*               Do NOT call this function unless you have opened the sound
*               channels used yourself with midasOpenChannels().
*
\****************************************************************************/

void CALLING midasCloseChannels(void)
{
    int         error;

    /* Close Sound Device channels: */
    if ( (error = midasSD->CloseChannels()) != OK )
        midasError(error);
    midasSDChans = 0;
    midasChannels = 0;
}




/****************************************************************************\
*
* Function:     midasPlayModule(mpModule *module, int numEffectChns)
*
* Description:  Loads a module into memory, points midasMP to the correct
*               Module Player and starts playing it.
*
* Input:        mpModule *module        Module loaded with midasLoadModule()
*               int numEffectChns       Number of channels to open for sound
*                                       effects. Ignored if sound channels
*                                       have already been opened with
*                                       midasOpenChannels().
*
* Returns:      Pointer to module structure. This function can not fail,
*               as it will call midasError() to handle all error cases.
*
* Notes:        The Sound Device channels available for sound effects are the
*               _first_ numEffectChns channels. So, for example, if you use
*               midasPlayModule(module, 3), you can use channels 0-2 for sound
*               effects. If you already have opened channels with
*               midasOpenChannels(), the module will be played with the last
*               possible channels, so that the first channels will be
*               available for sound effects. Note that if not enough channels
*               are open this function will fail.
*
\****************************************************************************/

void CALLING midasPlayModule(mpModule *module, int numEffectChns)
{
    short       numChans;
    int         error;
    int         firstChannel;

    midasMP = module->MP;

    /* initialize module player: */
    if ( (error = midasMP->Init(midasSD)) != OK )
        midasError(error);
    midasMPInit = 1;

    numChans = module->numChans;

    /* Open Sound Device channels if not already open: */
    if ( midasChannels == 0 )
    {
        if ( (error = midasSD->OpenChannels(numChans + numEffectChns)) != OK )
            midasError(error);
        midasSDChans = 1;
        firstChannel = numEffectChns;

        /* set amplification level if forced: */
        if ( midasAmplification != -1 )
        {
            if ( (error = midasSD->SetAmplification(midasAmplification)) != OK )
                midasError(error);
        }
    }
    else
    {
        if ( midasChannels < numChans )
            midasError(errNoChannels);
        firstChannel = midasChannels - numChans;
    }

    /* Start playing the module using the last Sound Device channels and
       looping the whole song: */
    if ( (error = midasMP->PlayModule(module, firstChannel, numChans, 0,
        32767)) != OK )
        midasError(error);
    midasMPPlay = 1;

    /* start playing using the timer: */
    if ( (error = tmrPlayMusic((void*) midasMP->Play, &midasPlayerNum))
        != OK )
        midasError(error);
    if ( (error = midasMP->SetUpdRateFunct(&tmrSetUpdRate)) != OK )
        midasError(error);

    midasTMRMusic = 1;
}




/****************************************************************************\
*
* Function:     void midasStopModule(mpModule *module)
*
* Input:        mpModule *module        the module which is being played
*
* Description:  Stops playing a module and uninitializes the Module Player.
*               If sound channels were NOT opened through midasOpenChannels(),
*               but by letting midasPlayModule() open them, they will be
*               closed. Sound channels opened with midasOpenChannels() are NOT
*               closed and must be closed separately.
*
\****************************************************************************/

void CALLING midasStopModule(mpModule *module)
{
    int         error, i;

    midasMP = module->MP;

    /* Stop playing music with timer: */
    if ( (error = midasMP->SetUpdRateFunct(NULL)) != OK )
        midasError(error);
    if ( (error = tmrStopMusic(midasPlayerNum)) != OK )
        midasError(error);

    midasTMRMusic = 0;

    /* stop playing the module: */
    if ( (error = midasMP->StopModule()) != OK )
        midasError(error);
    midasMPPlay = 0;

    /* uninitialize Module Player: */
    if ( (error = midasMP->Close()) != OK )
        midasError(error);
    midasMPInit = 0;
    midasMP = NULL;                     /* point midasMP to NULL for safety */

    /* If Sound Device channels were not opened with midasOpenChannels(),
       close them: */
    if ( midasChannels == 0 )
    {
        if ( (error = midasSD->CloseChannels()) != OK )
            midasError(error);
        midasSDChans = 0;
    }
    else
    {
        /* Sound Device channels were originally opened with
           midasOpenChannels(). Now stop sounds from the channels used by
           the Module Player: */
        for ( i = (midasChannels - module->numChans); i < midasChannels; i++ )
        {
            if ( (error = midasSD->StopSound(i)) != OK )
                midasError(error);
            if ( (error = midasSD->SetVolume(i, 0)) != OK )
                midasError(error);
        }
    }
}
