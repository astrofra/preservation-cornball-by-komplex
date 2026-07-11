/*      dostimer.c
 *
 * MIDAS Digital Audio System timer for MS-DOS
 *
 * $Id: dostimer.c,v 1.13 1997/07/31 10:56:36 pekangas Exp $
 *
 * Copyright 1996,1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#ifdef __DJGPP__
#include <internal.h>
#endif

#include <dos.h>
#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#include "sdevice.h"
#include "timer.h"
#include "mglobals.h"
#include "dpmi.h"

RCSID(const char *dostimer_rcsid = "$Id: dostimer.c,v 1.13 1997/07/31 10:56:36 pekangas Exp $";)

/*#define TIMERBORDERS*/

/* Time between to screen interrupts is 96.5% of total frame time - the
   interrupt comes somewhat before the Vertical Retrace actually starts: */
#define FRAMETIME 965

/* Timer mode: */
#define TIMERMODE 0x30

/* Maximum # of music players: */
#define MAXPLAYERS 16


#ifndef __DJGPP__
static void (__interrupt __far *oldTimer)();
#endif

static volatile int tmrState;
static volatile int playSD;

static volatile long sysTmrCount;       /* system timer counter */
static volatile long playTmrCount;      /* initial player timer count */
static volatile long playCount;         /* player timer count */
static volatile SoundDevice *sdev;      /* current SD */

static int CALLING (*musicPlayers[MAXPLAYERS])(void);    /* music players */

static volatile int playSD;             /* 1 if sound should be played */
static volatile int plTimer;            /* 1 if player-timer is active */
static volatile int plError;            /* music playing error code */

static volatile long userCount;         /* user timer count */
static volatile long userTmrCount;      /* initial value for userCount */
static volatile long userPreVrCount;    /* count after interrupt, before the
					   start of retrace */

static void CALLING (*preVR)();         /* pre-VR function */
static void CALLING (*immVR)();         /* immVR() */
static void CALLING (*inVR)();          /* inVR() */

static volatile int userTimer;          /* do we have the user timer? */
static volatile int userTimerActive;    /* is the user timer active? */
static volatile int userPlayer;         /* synchronize player to user
					   timer? */
static volatile int userSyncScreen;     /* synchronize user timer to screen
					   refresh? */

static volatile int tmrState;           /* timer state */

static volatile int winMode = 0;        /* are we in Win95 compatibility
					   mode? */


static uchar    playFpuState[108];      /* FPU state saving area for player
                                           interrupt */




/****************************************************************************\
*       enum tmrStates
*       --------------
* Description:  Possible timer states
\****************************************************************************/

enum tmrStates
{
    tmrSystem = 0,                      /* system timer */
    tmrPlayer,                          /* music player */
    tmrUser                             /* user timer */
};



#ifndef __DJGPP__

void outp(unsigned port, unsigned value);
#pragma aux outp = \
    "out    dx,al" \
    parm [edx] [eax] \
    modify exact [];

unsigned inp(unsigned port);
#pragma aux inp = \
    "xor    eax,eax" \
    "in     al,dx" \
    parm [edx] \
    value [eax] \
    modify exact [eax];

void DoSTI(void);
#pragma aux DoSTI = "sti" modify exact[];

void DoCLI(void);
#pragma aux DoCLI = "cli" modify exact[];

void SendEOI(void);
#pragma aux SendEOI = \
    "   mov     al,20h" \
    "   out     20h,al" \
    modify exact[eax];

void SavePlayerFPU(void);
#pragma aux SavePlayerFPU = \
    "   fsave   playFpuState" \
    modify exact [8087];

void RestorePlayerFPU(void);
#pragma aux RestorePlayerFPU = \
    "   frstor  playFpuState" \
    modify exact [8087];

#else

#define SendEOI() outportb(0x20, 0x20)
#define DoSTI() ENABLE()
#define DoCLI() DISABLE()

void SavePlayerFPU(void)
{
    __asm__ ("fsave _playFpuState");
}

void RestorePlayerFPU(void)
{
    __asm__ ("frstor _playFpuState");
}


#endif


/* Set border color: */
#ifdef TIMERBORDERS

#ifdef __DJGPP__

void SetBorder(int color)
{
    inportb(0x3DA);
    outportb(0x3C0, 0x31);
    outportb(0x3C0, (unsigned char) color);
}

#else

void SetBorder(int color);
#pragma aux SetBorder = \
    "   mov     dx,03DAh" \
    "   in      al,dx" \
    "   mov     dx,03C0h" \
    "   mov     al,31h" \
    "   out     dx,al" \
    "   mov     al,bl" \
    "   out     dx,al" \
    parm [ebx] \
    modify exact [eax edx];

#endif

#else

#define SetBorder(x)

#endif



#ifdef __DJGPP__

static void WaitNextVR(void)
{
    while ( inportb(0x3DA) & 8 ) ;
    while ( !(inportb(0x3DA) & 8) ) ;
}

static void WaitVR(void)
{
    while ( !(inportb(0x3DA) & 8) ) ;
}

static void WaitNoVR(void)
{
    while ( inportb(0x3DA) & 8 ) ;
}

#else

/* Wait for next Vertical Retrace: */
void WaitNextVR(void);
#pragma aux WaitNextVR = \
    "   mov     dx,03DAh" \
    "n: in      al,dx" \
    "   test    al,8" \
    "   jnz     n" \
    "v: in      al,dx" \
    "   test    al,8" \
    "   jz      v" \
    modify exact [eax edx];

/* Wait for Vertical Retrace: */
void WaitVR(void);
#pragma aux WaitVR = \
    "   mov     dx,03DAh" \
    "w: in      al,dx" \
    "   test    al,8" \
    "   jz      w" \
    modify exact [eax edx];

/* Wait for no Vertical Retrace: */
void WaitNoVR(void);
#pragma aux WaitNoVR = \
    "   mov     dx,03DAh" \
    "w: in      al,dx" \
    "   test    al,8" \
    "   jnz     w" \
    modify exact [eax edx];

/* Set DS to ES: */
void SetDStoES(void);
#pragma aux SetDStoES = \
    "   mov     ax,ds" \
    "   mov     es,ax" \
    modify exact [eax];


#endif



/****************************************************************************\
*
* Function:     void SetCount(unsigned count)
*
* Description:  Sets new timer count
*
* Input:        unsigned count          new timer count
*
\****************************************************************************/

static void SetCount(unsigned count)
{
    outp(0x43, TIMERMODE);
    outp(0x40, count & 0xFF);
    outp(0x40, (count >> 8) & 0xFF);
}



/****************************************************************************\
*
* Function:     void NextTimer(void)
*
* Description:  Sets everything up for the next timer interrupt
*
\****************************************************************************/

static void NextTimer(void)
{
    if ( userTimer )
    {
	/* We have the user timer */
        if ( playSD && (!winMode) )
        {
            if ( playCount < userCount )
            {
                if ( playCount < 10 )
                    playCount = 10;

                /* Player interrupt, please */
                tmrState = tmrPlayer;
                SetCount(playCount);
                return;
            }
        }

        if ( userCount < 10 )
            userCount = 10;

        /* Screen interrupt, please: */
        tmrState = tmrUser;
        SetCount(userCount);
        return;
    }

    if ( playSD )
    {
        if ( playCount < 10 )
            playCount = 10;

        /* Player interrupt: */
        tmrState = tmrPlayer;
        SetCount(playCount);
        return;
    }

    /* System timer: */
    tmrState = tmrSystem;
    SetCount(0);
}


#ifndef __DJGPP__

void CallDOSInt(void);
#pragma aux CallDOSInt = \
    "pushfd" \
    "call fword oldTimer" \
    modify exact [];
#endif


#ifdef __DJGPP__
static volatile int needCallDOS;
#endif


/****************************************************************************\
*
* Function:     void CheckSystemTimer(void)
*
* Description:  Calls the system timer if necessary, send EOI otherwise
*
\****************************************************************************/

void CheckSystemTimer(void)
{
    DoSTI();

    if ( sysTmrCount < 0x10000 )
    {
#ifdef __DJGPP__
        needCallDOS = 0;
#endif
        SendEOI();
        return;
    }

    while ( sysTmrCount >= 0x10000 )
    {
        sysTmrCount -= 0x10000;
#ifdef __DJGPP__
        needCallDOS = 1;
#else
        CallDOSInt();
#endif
    }
}




/****************************************************************************\
*
* Function:     void PollMIDAS(void)
*
* Description:  Polls MIDAS
*
\****************************************************************************/

void PollMIDAS(void)
{
    static int callMP;
    int         i;

    SavePlayerFPU();

    if ( (playSD) && (plError == OK) )
    {
        /* Prepare SD for playing: */
        if ( (plError = sdev->StartPlay()) != OK )
        {
            plTimer = 0;
            return;
        }

        do
        {
            /* Poll Sound Device: */
            if ( (plError = sdev->Play(&callMP)) != OK )
            {
                plTimer = 0;
                return;
            }

            if ( callMP )
            {
                /* Call all music players: */
                for ( i = 0; i < MAXPLAYERS; i++ )
                {
                    if ( musicPlayers[i] != NULL )
                    {
                        if ( (plError = (*musicPlayers[i])()) != OK )
                        {
                            plTimer = 0;
                            return;
                        }
                    }
                }
            }
        } while ( callMP && (sdev->tempoPoll == 0) );
    }

    RestorePlayerFPU();
}


/****************************************************************************\
*
* Function:     void __interrupt __far tmrISR(void)
*
* Description:  The timer ISR
*
\****************************************************************************/

#ifdef __DJGPP__
int tmrISR(void)
#else
void __interrupt __far tmrISR(void)
#endif
{
    int         chainDOS = 1;
    long        oldPlayCount;

    /* Set DS to ES as well: (stupid Watcom doesn't to this, and then assumes
       ES is valid) */
#ifndef __DJGPP__
    SetDStoES();
#endif

    SetBorder(9);

#ifdef __DJGPP__
    needCallDOS = 0;
#endif

    switch ( tmrState )
    {
        case tmrUser:
            SetBorder(15);
            DoCLI();                    /* Disable interrupts here */

            if ( userTimerActive )
            {		
                /* PANIC, user timer still active! */
                playCount -= userCount + userPreVrCount;
                sysTmrCount += userCount + userPreVrCount;
                userCount = userTmrCount;
                NextTimer();
                SendEOI();
                DoSTI();
                chainDOS = 0;
                break;
            }

	    userTimerActive = 1;
	    SetBorder(14);

	    if ( userSyncScreen )
	    {
		/* The user timer is synchronized to the screen. First make
		   sure we are not in Vertical Retrace and call the preVR
		   callback: */
		WaitNoVR();
		if ( preVR != NULL )
		    (*preVR)();
		
                /* Update timer counters now: */
                sysTmrCount += userCount + userPreVrCount;
                if ( userPlayer )
                    playCount = playTmrCount;
                else
                    playCount -= userCount + userPreVrCount;
                userCount = userTmrCount;

		/* Wait for the Vertical Retrace to start, call the immVR
		   callback and restart timer: */
		WaitVR();
		if ( immVR != NULL )
		    (*immVR)();
		NextTimer();
		CheckSystemTimer();
	    }
	    else
	    {
		/* The user timer is not synchronized to screen. Update
		   counters, restart timer and call the first two
		   callbacks: */
                sysTmrCount += userCount + userPreVrCount;
                if ( userPlayer )
                    playCount = playTmrCount;
                else
                    playCount -= userCount + userPreVrCount;
                userCount = userTmrCount;
		
		NextTimer();
		CheckSystemTimer();

		if ( preVR != NULL )
		    (*preVR)();
		if ( immVR != NULL )
		    (*immVR)();		
	    }

	    /* Now poll MIDAS if we are running in Win95 compatibility mode: */
	    if ( winMode && playSD )
	    {
		if ( sdev->tempoPoll )
		{
		    while ( playCount < 0 )
		    {
			PollMIDAS();
			playCount += playTmrCount;
		    }
		}
		else
		    PollMIDAS();
	    }

	    /* Now we just need to call the last callback and we're done: */
	    if ( inVR != NULL )
		(*inVR)();
	    chainDOS = 0;
	    userTimerActive = 0;
	    SetBorder(0);
	    break;

        case tmrPlayer:
            SetBorder(2);
            if ( plTimer )
            {
                /* Player timer active - panic! */
                userCount -= playCount;
                sysTmrCount += playCount;
                playCount = playTmrCount;
                NextTimer();
                SendEOI();
                DoSTI();
                chainDOS = 0;
                break;
            }
  
            plTimer = 1;

	    /* Update timer counters. If player is synchronized to the user
	       interrupt, make sure the user interrupt comes first: */
            userCount -= playCount;
            sysTmrCount += playCount;
            if ( userPlayer )
                playCount = 0xFFFF;
            else
                playCount = playTmrCount;
            NextTimer();

            if ( userTimer )
            {
                /* If we have a user timer, we'll only do the system
                   timer check in the user timer. If the user timer is
                   used, we'll need to acknowledge the player timer as
                   soon as possible, to make sure that the user timer can
                   interrupt the player interrupt. If we'd call the DOS
                   timer afterwards, we couldn't acknowledge the interrupt
                   here - sending two EOIs to the PIC is a Bad Thing, and
                   might result in missed interrupts.

                   Why not call the DOS timer here then? Before, we did just
                   that, but we can't do it with the DJGPP Allegro IRQ
                   handling code, at least not without starting to mess
                   around in an unconfortably low level.

                   No need to worry though, the system timer will be called
                   often enough by the user timer alone, and we already
                   updated the counter above.
                */
                DoSTI();
                SendEOI();
#ifdef __DJGPP__		
                needCallDOS = 0;
#endif		
            }
            else
            {
                CheckSystemTimer();
            }
            chainDOS = 0;

            oldPlayCount = playTmrCount;

            PollMIDAS();

            /* Check if player timer rate has been updated: */
            if ( (sdev->tempoPoll == 1) && (playTmrCount != oldPlayCount))
            {
                playCount = playTmrCount;
                if ( tmrState == tmrPlayer )
                    NextTimer();
            }

            plTimer = 0;
            chainDOS = 0;
            break;

        case tmrSystem:
        default:
            /* The system timer - set rate to 18.2Hz and chain to DOS: */
            SetCount(0);
            chainDOS = 1;
            break;
    }

    SetBorder(0);

#ifdef __DJGPP__
    if ( chainDOS )
        return 1;
    return needCallDOS;
#else
    if ( chainDOS )
        _chain_intr(oldTimer);
#endif
}



/****************************************************************************\
*
* Function:     int tmrInit(void);
*
* Description:  Initializes the timer
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING tmrInit(void)
{
    static dpmiRealCallRegs regs;
    
    tmrState = tmrSystem;               /* system timer only */
    playSD = 0;
    userTimer = 0;
    userTimerActive = 0;
    userPlayer = 0;
    userSyncScreen = 0;
    playFpuState[0] = 0;                /* hack for DJGPP */

    /* Check if we are running under Windows 3.x or 95: */
    if ( !winMode )
    {
        regs.rSS = regs.rSP = 0;
        regs.rEAX = 0x160A;
        dpmiRealModeInt(0x2F, &regs);
        if ( (regs.rEAX & 0xFFFF) == 0 )
            winMode = 1;
    }

    /* Check if we are running under Windows NT: */
    if ( !winMode )
    {
        regs.rSS = regs.rSP = 0;
        regs.rEAX = 0x3306;
        dpmiRealModeInt(0x21, &regs);
        if ( (regs.rEBX & 0xFFFF) == 0x3205 )
            winMode = 1;
    }    

    /* Get old timer interrupt and set our own: */
#ifdef __DJGPP__
    if ( _install_irq(8, tmrISR) != 0 )
    {
        ERROR(errUndefined, ID_tmrInit);
        return errUndefined;
    }
#else
    oldTimer = _dos_getvect(8);
    _dos_setvect(8, tmrISR);
#endif

    /* Restart timer at 18.2Hz: */
/*    SetCount(0);*/

    return OK;
}




/****************************************************************************\
*
* Function:     int tmrClose(void);
*
* Description:  Uninitializes the timer.
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING tmrClose(void)
{
    /* Set DOS default timer mode and 18.2Hz rate: */
    outp(0x43, 0x36);
    outp(0x40, 0);
    outp(0x40, 0);

#ifdef __DJGPP__
    _remove_irq(8);
#else
    /* Restore old interrupt vector: */
    _dos_setvect(8, oldTimer);
#endif

    /* Set DOS default timer mode and 18.2Hz rate: (sometimes it seems to be
       necessary to do this twice) */
    outp(0x43, 0x36);
    outp(0x40, 0);
    outp(0x40, 0);

    return OK;
}




/****************************************************************************\
*
* Function:     int tmrGetScreenRefreshRate(unsigned *refreshRate)
*
* Description:  Tries to determine the screen refresh rate for the current
*               display mode. The refresh rate can be used with
*               tmrSetUserCallbacks() to synchronize the user callbacks to
*               screen refresh.
*
* Input:        unsigned *refreshRate   pointer to target refresh rate
*                                       variable.
*
* Returns:      MIDAS error code. Screen refresh rate is written to
*               *refreshRate. If the refresh rate could not be determined
*               successfully (for example, running under Win95), *refreshRate
*               is set to 0.
*
*               The refresh rate is returned in milliHertz, ie. 1000*Hz.
*               70Hz refresh rate becomes 70000 etc.
*
\****************************************************************************/

int CALLING tmrGetScreenRefreshRate(unsigned *refreshRate)
{
    int         failCount = 0, success = 0;
    long        count1, count2, prevCount = 0, count = 0;

    if ( winMode )
    {
	/* Win95 compatibility mode - we won't be able to determine the
	   refresh rate: */
	*refreshRate = 0;
	return OK;
    }

    /* Try to be safe. Nasty for possible music playback, but the only way
       to get reliable results: */
    DoCLI();

    while ( (failCount < 4) && (success != 1) )
    {
        WaitNextVR();
        outp(0x43, 0x36);
        outp(0x40, 0);
        outp(0x40, 0);
        WaitNextVR();
        outp(0x43, 0);
        count1 = inp(0x40);
        count1 |= (inp(0x40)) << 8;
        count1 = 0x10000-count1;

        WaitNextVR();
        outp(0x43, 0x36);
        outp(0x40, 0);
        outp(0x40, 0);
        WaitNextVR();
        outp(0x43, 0);
        count2 = inp(0x40);
        count2 |= (inp(0x40)) << 8;
        count2 = 0x10000-count2;

        if ( ((count2 - count1) > 2) || ((count2 - count1) < -2) )
        {
            failCount++;
        }
        else
        {
            count = count1 >> 1;
            if ( ((prevCount - count) <= 2) && ((prevCount - count) >= -2) )
                success = 1;
            else
            {
                prevCount = count;
                failCount++;
            }
        }
    }

    if ( success )
    {
	/* We got the framerate! */
	*refreshRate = 1193180000 / count;

	/* Check that the refresh rate is reasonable - if above 250Hz or below
	   35Hz, we'll reject it as a bogus value: */
	if ( (*refreshRate > 250000) || (*refreshRate < 35000) )
	{
	    *refreshRate = 0;
	    winMode = 1;
	}
    }
    else
    {
	/* We couldn't determine the frame rate - return 0 and set the timer
	   to Win95 compatibility mode: */
	*refreshRate = 0;
	winMode = 1;
    }

    DoSTI();

    return OK;
}




/****************************************************************************\
*
* Function:     int tmrPlaySD(SoundDevice *SD);
*
* Description:  Starts playing sound with a Sound Device ie. calling its
*               Play() function in the update rate, which is set to
*               50Hz.
*
* Input:        SoundDevice *SD         Sound Device that will be used
*
* Returns:      MIDAS error code.
*
\****************************************************************************/

int CALLING tmrPlaySD(SoundDevice *SD)
{
    int         i;

    sdev = SD;

    /* Reset all music player pointers to NULL: */
    for ( i = 0; i < MAXPLAYERS; i++ )
        musicPlayers[i] = NULL;

    if ( !sdev->tempoPoll )
    {
	if ( userTimer && (!winMode) )
        {
	    /* We have a user timer callback and the SoundDevice is not
	       tempoPolling - poll MIDAS in synchronization to the user
	       timer, at 1/4th into each cycle: */
	    userPlayer = 1;
	    playTmrCount = playCount = userTmrCount / 4;
        }
        else
        {
	    /* The Sound Device is not tempoPolling, and we have no user
	       timer - just poll at 100Hz: */
            playTmrCount = playCount = 1193180 / 100;
            userPlayer = 0;
        }
    }
    else
    {
	/* We have a tempoPolling Sound Device, so we can't synchronize
	   polling to the user timer. Start at 50Hz: */
        playTmrCount = playCount = 1193180 / 50;
        userPlayer = 0;
    }

    plTimer = 0;
    plError = 0;

    /* Start playing: */
    DoCLI();

    /* If we are running just the system timer, change it to player: */
    if ( tmrState == tmrSystem )
    {
        tmrState = tmrPlayer;
        SetCount(playCount);
        sysTmrCount = 0;
    }

    playSD = 1;

    DoSTI();

    return OK;
}




/****************************************************************************\
*
* Function:     int tmrStopSD(void);
*
* Description:  Stops playing sound with the Sound Device.
*
* Returns:      MIDAS error code.
*
\****************************************************************************/

int CALLING tmrStopSD(void)
{
    DoCLI();

    playSD = 0;

    if ( userTimer )
    {
        /* No user timer - only system timer now: */
        tmrState = tmrSystem;
        SetCount(0);
    }

    DoSTI();

    return OK;
}




/****************************************************************************\
*
* Function:     int tmrPlayMusic(void *play, int *playerNum);
*
* Description:  Starts playing music with the timer.
*
* Input:        void *play              Pointer to music playing function,
*                                       must return MIDAS error codes
*               int *playerNum          Pointer to player number, used
*                                       for stopping music
*
* Returns:      MIDAS error code. Player number is written to *playerNum.
*
* Notes:        There can be a maximum of 16 music players active at the
*               same time.
*
\****************************************************************************/

int CALLING tmrPlayMusic(int CALLING (*play)(), int *playerNum)
{
    int         i;


    /* Try to find a free music player: */
    for ( i = 0; i < MAXPLAYERS; i++ )
    {
        if ( musicPlayers[i] == NULL )
            break;
    }

    if ( i >= MAXPLAYERS )
    {
        /* No free player found: */
        ERROR(errOutOfResources, ID_tmrPlayMusic);
        return errOutOfResources;
    }

    /* Free player found - store the pointer and return player ID: */
    musicPlayers[i] = play;
    *playerNum = i;

    return OK;
}




/****************************************************************************\
*
* Function:     int tmrStopMusic(int playerNum);
*
* Description:  Stops playing music with the timer.
*
* Input:        int playerNum           Number of player to be stopped.
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING tmrStopMusic(int playerNum)
{
    musicPlayers[playerNum] = NULL;

    return OK;
}




/****************************************************************************\
*
* Function:     int tmrSetUserCallbacks(unsigned rate, int syncScreen,
*                   void (*preVR)(), void (*immVR)(), void (*inVR)())
*
* Description:  Sets timer user callback functions and the callback rate
*
* Input:        unsigned rate           callback timer rate, in milliHertz
*                                       (1000 * Hz, 50Hz = 50000 etc)
*               int syncScreen          1 if the callbacks should be
*                                       synchronized to screen refresh, 0 if
*                                       not.
*               void (*preVR)()         preVR callback function, called
*                                       immediately BEFORE Vertical Retrace
*                                       if syncScreen is 1
*               void (*immVR)()         immVR callback function, called
*                                       immediately AFTER VR start if
*                                       syncScreen is 1
*               void (*inVR)()          inVR callback function, called later
*                                       DURING VR if syncScreen is 1.
*
* Returns:      MIDAS error code.
*
* Notes:        If any of preVR, immVR or inVR is NULL, that callback is
*               simply ignored. If syncScreen is 1, the user callback timer
*               is synchronized to screen refresh, if possible. In that case,
*               "rate" MUST be the refresh rate for that display mode,
*               returned by tmrGetScreenRefreshRate(). If syncScreen is 0,
*               the callback functions are simply called one after another.
*
*               In any case, the preVR() and immVR() callback functions must
*               be as short as possible, to avoid timing problems. They should
*               not do more than just set a few hardware register and update
*               some counter. inVR() can take somewhat longer time, and can
*               be used, for example, for setting the VGA palette. However,
*               to avoid possible music tempo problems, it should never take
*               more than about one fourth of the total time between two
*               interrupts.
*
*               The callback rate should be between 50Hz and 200Hz.
*
\****************************************************************************/

int CALLING tmrSetUserCallbacks(unsigned rate, int syncScreen,
				void CALLING (*_preVR)(),
				void CALLING (*_immVR)(),
				void CALLING (*_inVR)())
{
    /* We don't want to get disturbed right now: */
    DoCLI();

    /* Save the function pointers: */
    preVR = _preVR;
    immVR = _immVR;
    inVR = _inVR;

    if ( syncScreen && (!winMode) )
    {
        userTmrCount = FRAMETIME * (1193180000 / rate) / 1000;
        userPreVrCount = (1193180000 / rate) - userTmrCount;
        userSyncScreen = 1;
    }
    else
    {
        userTmrCount = 1193180000 / rate;
        userPreVrCount = 0;
        userSyncScreen = 0;
    }

    /* If we are synchronizing to screen, wait for retrace and restart: */
    if ( syncScreen )
    {
        userCount = userTmrCount;
        tmrState = tmrUser;
        userTimer = 1;
        WaitNextVR();
        SetCount(userCount);
    }
    else
    {
        /* Not synchronizing - restart if we have only the system timer: */
        userTimer = 1;
        userCount = userTmrCount;
        if ( tmrState == tmrSystem )
        {
            tmrState = tmrUser;
            SetCount(userCount);
            sysTmrCount = 0;
        }
    }

    /* If we aren't running in Win95 compatibility mode, synchronize
       possible music playback to user timer if possible: */
    if ( playSD )
    {
        if ( (!winMode) && (!sdev->tempoPoll) )
        {
            playCount = playTmrCount = userTmrCount / 4;
            userPlayer = 1;
        }
    }

    DoSTI();

    return OK;
}




/****************************************************************************\
*
* Function:     int tmrRemoveUserCallbacks(void)
*
* Description:  Removes user timer callbacks and stops the user timer
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING tmrRemoveUserCallbacks(void)
{
    DoCLI();

    if ( userPlayer )
    {
	/* Music playback is synchronized to user timer - restart at 100Hz: */
	playCount = playTmrCount = 1193180 / 100;
	userPlayer = 0;
    }

    userTimer = userTimerActive = 0;
    userSyncScreen = 0;
    NextTimer();

    DoSTI();

    return OK;
}




/****************************************************************************\
*
* Function:     int tmrSetUpdRate(unsigned updRate);
*
* Description:  Sets the timer update rate, ie. the rate at which the music
*               playing routines are called
*
* Input:        unsigned updRate        updating rate, in 100*Hz (5000=50Hz)
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING tmrSetUpdRate(unsigned updRate)
{
    /* Only change if tempopolling: */
    if ( sdev->tempoPoll )
    {
        playTmrCount = 119318000 / updRate;
    }

    return OK;
}









/*
 * $Log: dostimer.c,v $
 * Revision 1.13  1997/07/31 10:56:36  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.12  1997/07/27 17:28:17  jpaana
 * Fixed for DJGPP
 *
 * Revision 1.11  1997/07/11 23:38:46  jpaana
 * Fixed a typo
 *
 * Revision 1.10  1997/06/20 10:09:09  pekangas
 * Added saving of FPU state
 *
 * Revision 1.9  1997/03/05 16:47:57  pekangas
 * Partly rewritten, added user callbacks withou screen synchronization
 *
 * Revision 1.8  1997/02/27 15:59:40  pekangas
 * Added DJGPP support through Allegro IRQ handling code
 *
 * Revision 1.6  1997/01/16 18:41:59  pekangas
 * Changed copyright messages to Housemarque
 *
 * Revision 1.5  1996/10/13 16:53:07  pekangas
 * The timer ISR now sets ES to DS
 *
 * Revision 1.4  1996/08/06 18:46:09  pekangas
 * Removed border colors
 *
 * Revision 1.3  1996/08/06 16:51:36  pekangas
 * Fixed SB and timer conflicts
 *
 * Revision 1.2  1996/08/04 18:08:21  pekangas
 * Fixed a nasty bug in tmrGetScrSync - interrupts were left disabled
 *
 * Revision 1.1  1996/06/06 19:28:17  pekangas
 * Initial revision
 *
*/
