/*      MIDAS.H
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

#ifndef __MIDAS_H
#define __MIDAS_H



/****************************************************************************\
*      NULL pointer definition (independence on library includes):
\****************************************************************************/

#ifndef NULL
    #define NULL 0L
#endif



/****************************************************************************\
*      MIDAS version defines
\****************************************************************************/

#define MVERSION 0.40a
#define MVERSTR "0.40a"
#define MVERNUM 0x0040



/****************************************************************************\
*      MIDAS header files:
\****************************************************************************/

#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#include "mglobals.h"
#include "mmem.h"
#include "file.h"
#include "sdevice.h"
#include "mplayer.h"
#include "s3m.h"
#include "mod.h"
#include "mtm.h"
#include "timer.h"
#include "ems.h"
#include "dma.h"
#include "dsm.h"
#include "vu.h"
#include "mparser.h"
#include "mconfig.h"
#include "vgatext.h"
#include "mutils.h"
#include "mm.h"




/****************************************************************************\
*      MIDAS global variables:
\****************************************************************************/

extern SoundDevice GLOBALVAR GUS;       /* Gravis UltraSound Sound Device */
extern SoundDevice GLOBALVAR PAS;       /* Pro Audio Spectrum Sound Device */
extern SoundDevice GLOBALVAR WSS;       /* Windows Sound System Sound Device*/
extern SoundDevice GLOBALVAR SB;        /* Sound Blaster Sound Device */
extern SoundDevice GLOBALVAR NSND;      /* No Sound Sound Device */

#define NUMSDEVICES 5                   /* total number of Sound Devices */
#define NUMMPLAYERS 3                   /* total number of Module Players */

extern ModulePlayer far GLOBALVAR mpS3M;    /* Scream Tracker 3 Module Player */
extern ModulePlayer far GLOBALVAR mpMOD;    /* Protracker Module Player */
extern ModulePlayer far GLOBALVAR mpMTM;    /* Multitracker Module Player */

extern SoundDevice GLOBALVAR *midasSD;  /* current Sound Device */
extern ModulePlayer GLOBALVAR *midasMP; /* current Module Player */

    /* array to pointers to all Sound Devices, in numbering and detection
       order - GUS is SD #1: */
extern SoundDevice * GLOBALVAR midasSoundDevices[NUMSDEVICES];

extern int GLOBALVAR midasDisableEMS;   /* 1 if EMS usage is disabled
                                           (default 0) */
extern int GLOBALVAR midasSDNumber;     /* Sound Device number (-1 for
                                           autodetect, default -1) */
extern int GLOBALVAR midasSDPort;       /* Sound Device I/O port number
                                           (-1 for autodetect or SD default,
                                           default -1) */
extern int GLOBALVAR midasSDIRQ;        /* Sound Device IRQ number (-1 for
                                           autodetect or SD default,
                                           default -1) */
extern int GLOBALVAR midasSDDMA;        /* Sound Device DMA channel number
                                           (-1 for autodetect or SD default,
                                           default -1) */
extern int GLOBALVAR midasSDCard;       /* Sound Device sound card type
                                           (-1 for autodetect or SD default,
                                           default -1) */
extern unsigned GLOBALVAR midasMixRate; /* Sound Device mixing rate */
extern unsigned GLOBALVAR midasOutputMode; /* Sound Device output mode force
                                           bits, default 0 (SD default) */
extern int GLOBALVAR midasAmplification;  /* Forced amplification level or -1
                                             for SD default (default -1) */
extern int GLOBALVAR midasChannels;     /* number of channels open or 0 if no
                                           channels have been opened using
                                           midasOpenChannels() */
extern int GLOBALVAR midasPlayerNum;    /* timer music player number */


extern int GLOBALVAR midasEMSInit;      /* is EMS heap manager initialized? */
extern int GLOBALVAR midasTMRInit;      /* is TempoTimer initialized? */
extern int GLOBALVAR midasTMRPlay;      /* is sound beind player with timer?*/
extern int GLOBALVAR midasSDInit;       /* is Sound Device initialized? */
extern int GLOBALVAR midasSDChans;      /* are Sound Device channels open? */
extern int GLOBALVAR midasMPInit;       /* is Module Player initialized? */
extern int GLOBALVAR midasMPPlay;       /* is Module Player playing? */
extern int GLOBALVAR midasTMRMusic;     /* is music being player with timer?*/




/****************************************************************************\
*      MIDAS constants:
\****************************************************************************/



/****************************************************************************\
*      MIDAS global functions from MIDAS.C:
\****************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************\
*
* Function:     void midasError(int errNum)
*
* Description:  Prints a MIDAS error message to stderr and exits to DOS
*
* Input:        int errNum              MIDAS error code
*
\****************************************************************************/

void CALLING midasError(int errNum);




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

void CALLING midasUninitError(int errNum);



/****************************************************************************\
*
* Function:     void midasDetectSD(void)
*
* Description:  Attempts to detect a Sound Device. Sets the global variable
*               midasSD to point to the detected Sound Device or NULL if no
*               Sound Device was detected
*
\****************************************************************************/

void CALLING midasDetectSD(void);



/****************************************************************************\
*
* Function:     void midasInit(void)
*
* Description:  Initializes MIDAS Sound System
*
\****************************************************************************/

void CALLING midasInit(void);



/****************************************************************************\
*
* Function:     void midasClose(void)
*
* Description:  Uninitializes MIDAS Sound System
*
\****************************************************************************/

void CALLING midasClose(void);



/****************************************************************************\
*
* Function:     void midasSetDefaults(void)
*
* Description:  Initializes MIDAS Sound System variables to their default
*               states. MUST be the first MIDAS function to be called.
*
\****************************************************************************/

void CALLING midasSetDefaults(void);



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
    ushort slength, ushort loopStart, ushort loopEnd));




/****************************************************************************\
*
* Function:     void midasFreeModule(mpModule *module);
*
* Description:  Deallocates a module from memory
*
* Input:        mpModule *module        Pointer to module to be deallocated
*
\****************************************************************************/

void CALLING midasFreeModule(mpModule *module);




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

void CALLING midasOpenChannels(int numChans);




/****************************************************************************\
*
* Function:     void midasCloseChannels(void);
*
* Description:  Closes Sound Device channels opened with midasOpenChannels().
*               Do NOT call this function unless you have opened the sound
*               channels used yourself with midasOpenChannels().
*
\****************************************************************************/

void CALLING midasCloseChannels(void);




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

void CALLING midasPlayModule(mpModule *module, int numEffectChns);




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

void CALLING midasStopModule(mpModule *module);


#ifdef __cplusplus
}
#endif


#endif
