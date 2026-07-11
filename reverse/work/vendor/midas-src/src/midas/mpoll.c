/*      mpoll.c
 *
 * Functions for polling MIDAS Digital Audio System in a thread
 *
 * $Id: mpoll.c,v 1.19 1997/07/31 10:56:52 pekangas Exp $
 *
 * Copyright 1996,1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#if defined(__WATCOMC__)|| defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#endif

#include "midas.h"

#ifdef __UNIX__
#include <unistd.h>
#include <pthread.h>
#endif

RCSID(const char *mpoll_rcsid = "$Id: mpoll.c,v 1.19 1997/07/31 10:56:52 pekangas Exp $";)

#ifdef __UNIX__
    static pthread_t            pollThread;
    static pthread_attr_t       thrAttr;
    static struct sched_param   param;
    static pthread_mutex_t      mutex;
    static pthread_mutexattr_t  attr = { PTHREAD_MUTEX_RECURSIVE_NP };
#endif


#ifdef __WIN32__
static CRITICAL_SECTION pollCriticalSection;
#endif    



/****************************************************************************\
*
* Function:     void PollMIDAS(void)
*
* Description:  Polls MIDAS Digital Audio System
*
\****************************************************************************/

static volatile int stopPolling = 0;
static volatile unsigned pollSleep = 50;

void PollMIDAS(void)
{
    int         error;
    int         callMP;

    LockPollThread();

    if ( !midasSDInit )
        return;

    if ( (error = midasSD->StartPlay()) != OK )
        midasError(error);
    do
    {
        if ( (error = midasSD->Play(&callMP)) != OK )
            midasError(error);
        if ( callMP )
        {
            if ( midasGMPInit )
            {
                if ( (error = gmpPlay()) != OK )
                    midasError(error);
            }
        }
    } while ( callMP && (midasSD->tempoPoll == 0) && !stopPolling);

    UnlockPollThread();
}




#ifdef __UNIX__
void *PollerThread(void *dummy)
#else
#ifdef __WC32__
static void PollerThread(void *dummy)
#else
static void __cdecl PollerThread(void *dummy)
#endif
#endif
{
    dummy = dummy;

#ifdef __WIN32__
    /* We'd better make the player thread's priority still above normal: */
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
#endif

    while ( stopPolling == 0 )
    {
        PollMIDAS();
#ifdef __UNIX__
	usleep(pollSleep * 1000);
#else
        Sleep(pollSleep);
#endif
    }

    stopPolling = 0;

#ifdef __UNIX__
    pthread_exit(0);
    return NULL;
#else
    _endthread();
#endif
}



/****************************************************************************\
*
* Function:     void StartPlayThread(unsigned pollPeriod)
*
* Description:  Starts polling MIDAS in a thread
*
* Input:        unsigned pollPeriod     polling period (delay between two
*                                       polling loops) in milliseconds
*
\****************************************************************************/

void StartPlayThread(unsigned pollPeriod)
{
#ifdef __WIN32__
#ifdef __WC32__
    int         pollThread;
#else
    unsigned long pollThread;
#endif
#else
    int code;
#endif
    pollSleep = pollPeriod;


    /* Start polling MIDAS in a thread: */
#ifdef __UNIX__
    pthread_attr_init(&thrAttr);
    pthread_attr_setschedpolicy(&thrAttr, SCHED_RR);
    param.sched_priority = sched_get_priority_max(SCHED_RR);
    pthread_attr_setschedparam(&thrAttr, &param);
    code = pthread_create(&pollThread, &thrAttr, PollerThread, NULL);
    if ( code )
        midasErrorExit("StartPlayThread: Couldn't create player thread!");
#else
#ifdef __WC32__
    pollThread = _beginthread(PollerThread, NULL, 4096, NULL);
    if ( pollThread == -1 )
        midasErrorExit("StartPlayThread: Couldn't create player thread!");
#else
    pollThread = _beginthread(PollerThread, 4096, NULL);
    if ( pollThread == -1 )
        midasErrorExit("StartPlayThread: Couldn't create player thread!");
#endif
#endif
}




/****************************************************************************\
*
* Function:     void StopPlayThread(void)
*
* Description:  Stops polling MIDAS in a thread
*
\****************************************************************************/

void StopPlayThread(void)
{
    /* Ugly but works */

    stopPolling = 1;
    while ( stopPolling )
#ifdef __UNIX__
        usleep(pollSleep*500);
#else
        Sleep(pollSleep/2);
#endif        
}




/****************************************************************************\
*
* Function:     void InitThreads(void)
*
* Description:  Initializes the MIDAS thread polling system and creates the
*               polling thread critical section object.
*
\****************************************************************************/

void InitThreads(void)
{
#ifdef __WIN32__    
    InitializeCriticalSection(&pollCriticalSection);
#else
#ifdef __UNIX__
    pthread_mutex_init(&mutex, &attr);
#else
    TBD!
#endif
#endif        
}




/****************************************************************************\
*
* Function:     void LockPollThread(void)
*
* Description:  Locks the poller thread by entering a critical section. This
*               function is used to ensure that only one thread accesses
*               critical MIDAS parts at a time.
*
\****************************************************************************/

void LockPollThread(void)
{
#ifdef __WIN32__
    EnterCriticalSection(&pollCriticalSection);
#else
#ifdef __UNIX__
    pthread_mutex_lock(&mutex);
#else
    TBD!
#endif
#endif       
}




/****************************************************************************\
*
* Function:     void UnlockPollThread(void)
*
* Description:  Unlocks the polling thread.
*
\****************************************************************************/

void UnlockPollThread(void)
{
#ifdef __WIN32__
    LeaveCriticalSection(&pollCriticalSection);
#else
#ifdef __UNIX__
    pthread_mutex_unlock(&mutex);
#else
    TBD!
#endif
#endif        
}
    

/*
 * $Log: mpoll.c,v $
 * Revision 1.19  1997/07/31 10:56:52  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.18  1997/06/10 23:28:36  jpaana
 * If run as root, sets higher priority to the player thread
 *
 * Revision 1.17  1997/06/10 19:55:10  jpaana
 * Changed detection of Win32 system in the beginning of the file
 *
 * Revision 1.16  1997/06/05 20:18:49  pekangas
 * Added preliminary support for interpolating mixing (mono only at the
 * moment)
 *
 * Revision 1.15  1997/06/02 00:54:16  jpaana
 * Changed most __LINUX__ defines to __UNIX__ for generic *nix porting
 *
 * Revision 1.14  1997/05/23 14:59:44  jpaana
 * Added (Un)LockPollThread-functions for Linux
 *
 * Revision 1.13  1997/05/07 17:14:53  pekangas
 * Added a lot of new thread synchronization code, mainly to minimize the
 * cases where MIDAS state may be modified when the player thread is active
 * and vice versa. Added MIDASlock() and MIDASunlock() to the API.
 *
 * Revision 1.12  1997/05/03 22:19:30  jpaana
 * Fixed a warning.
 *
 * Revision 1.11  1997/02/08 14:54:30  jpaana
 * Added an early return path from the polling thread, necessary on Linux for
 * now
 *
 * Revision 1.10  1997/02/07 15:15:33  pekangas
 * Fixed to compile for Linux with the new build system
 *
 * Revision 1.9  1997/01/26 23:29:45  jpaana
 * Small fixes for Linux
 *
 * Revision 1.8  1997/01/16 18:41:59  pekangas
 * Changed copyright messages to Housemarque
 *
 * Revision 1.7  1996/09/28 09:00:16  jpaana
 * Still fixes for Linux
 *
 * Revision 1.6  1996/09/28 08:12:40  jpaana
 * Fixed return value of pthread_create
 *
 * Revision 1.5  1996/09/28 06:50:36  jpaana
 * Fixed for Linuxthreads-0.4
 *
 * Revision 1.4  1996/09/21 17:40:45  jpaana
 * fixed some warnings
 *
 * Revision 1.3  1996/09/21 17:18:01  jpaana
 * Added Linux-stuff
 *
 * Revision 1.2  1996/09/02 20:19:30  pekangas
 * Changed to use _beginthread with Visual C as well
 *
 * Revision 1.1  1996/08/06 20:36:37  pekangas
 * Initial revision
 *
*/
