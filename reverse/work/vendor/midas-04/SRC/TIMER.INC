;*	TIMER.INC
;*
;* TempoTimer, v1.20
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*


GLOBAL  LANG tmrActive : word


;/***************************************************************************\
;*
;* Function:     int tmrGetScrSync(ushort *scrSync);
;*
;* Description:  Calculates the screen synchronization value for timer
;*
;* Input:        ushort *scrSync         pointer to screen synchronization
;*                                       value
;*
;* Returns:      MIDAS error code.
;*               Screen syncronization value used with tmrSyncScr() is stored
;*               in *scrSync.
;*
;\***************************************************************************/

GLOBAL  LANG tmrGetScrSync : far




;/***************************************************************************\
;*
;* Function:     int tmrInit(void);
;*
;* Description:  Initializes TempoTimer.
;*
;* Returns:      MIDAS error code
;*
;\***************************************************************************/

GLOBAL  LANG tmrInit : far




;/***************************************************************************\
;*
;* Function:     int tmrClose(void);
;*
;* Description:  Uninitializes TempoTimer. MUST be called if and ONLY if
;*               tmrInit() has been called.
;*
;* Returns:      MIDAS error code
;*
;\***************************************************************************/

GLOBAL  LANG tmrClose : far




;/***************************************************************************\
;*
;* Function:    int tmrPlaySD(SoundDevice *SD);
;*
;* Description: Starts playing sound with a Sound Device ie. calling its
;*              Play() function in the update rate, which is set to
;*              50Hz.
;*
;* Input:       SoundDevice *SD         Sound Device that will be used
;*
;* Returns:     MIDAS error code.
;*
;\***************************************************************************/

GLOBAL  LANG tmrPlaySD : far




;/***************************************************************************\
;*
;* Function:    int tmrStopSD(void);
;*
;* Description: Stops playing sound with the Sound Device.
;*
;* Returns:     MIDAS error code.
;*
;\***************************************************************************/

GLOBAL  LANG tmrStopSD : far




;/***************************************************************************\
;*
;* Function:    int tmrPlayMusic(void (*play)(), int *playerNum);
;*
;* Description: Starts playing music with the timer.
;*
;* Input:       void (*play)()          Music playing function
;*              int *playerNum          Pointer to player number, used
;*                                      for stopping music
;*
;* Returns:     MIDAS error code. Player number is written to *playerNum.
;*
;* Notes:       There can be a maximum of 16 music players active at the
;*              same time.
;*
;\****************************************************************************/

GLOBAL  LANG tmrPlayMusic : far




;/***************************************************************************\
;*
;* Function:    int tmrStopMusic(int playerNum);
;*
;* Description: Stops playing music with the timer.
;*
;* Input:       int playerNum           Number of player to be stopped.
;*
;* Returns:     MIDAS error code
;*
;\***************************************************************************/

GLOBAL  LANG tmrStopMusic : far




;/***************************************************************************\
;*
;* Function:     int tmrSyncScr(ushort sync, void (*preVR)(), void (*immVR)(),
;*               void (*inVR)());
;*
;* Description:  Synchronizes the timer to screen refresh.
;*
;* Input:        ushort sync             Screen synchronization value returned
;*                                       by tmrGetScrSync().
;*               void (*preVR)()         Pointer to the routine that will be
;*                                       called BEFORE Vertical Retrace
;*               void (*immVR)()         Pointer to the routine that will be
;*                                       called immediately after Vertical
;*                                       Retrace starts
;*               void (*inVR)()          Pointer to the routine that will be
;*                                       called some time during Vertical
;*                                       Retrace
;*
;* Returns:      MIDAS error code
;*
;* Notes:        preVR() and immVR() functions must be as short as possible
;*               and do nothing else than update counters or set some VGA
;*               registers to avoid timer synchronization problems. inVR()
;*               can take a longer time and can be used for, for example,
;*               setting the palette.
;*
;\***************************************************************************/

GLOBAL  LANG tmrSyncScr : far




;/***************************************************************************\
;*
;* Function:     int tmrStopScrSync(void);
;*
;* Description:  Stops synchronizing the timer to the screen.
;*
;* Returns:      MIDAS error code
;*
;\***************************************************************************/

GLOBAL  LANG tmrStopScrSync : far



;/***************************************************************************\
;*
;* Function:     int tmrSetUpdRate(ushort updRate);
;*
;* Description:  Sets the timer update rate, ie. the rate at which the music
;*               playing routine is called
;*
;* Input:        ushort updRate          updating rate, in 100*Hz (5000=50Hz)
;*
;* Returns:      MIDAS error code
;*
;\***************************************************************************/

GLOBAL  LANG tmrSetUpdRate : far



;/***************************************************************************\
;*       enum tmrFunctIDs
;*       ----------------
;* Description:  ID numbers for TempoTimer functions
;\***************************************************************************/

ENUM    tmrFunctIDs \
        ID_tmrGetScrSync = ID_tmr, \
        ID_tmrInit, \
        ID_tmrClose, \
        ID_tmrPlaySD, \
        ID_tmrStopSD, \
        ID_tmrPlayMusic, \
        ID_tmrStopMusic, \
        ID_tmrSyncScr, \
        ID_tmrStopScrSync, \
        ID_tmrSetUpdRate
