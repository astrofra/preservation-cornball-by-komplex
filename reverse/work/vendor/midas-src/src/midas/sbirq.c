/*      sbirq.c
 *
 * Sound Blaster IRQ handler wrappers (backwards or what? :)
 *
 * $Id: sbirq.c,v 1.2 1997/07/31 10:56:56 pekangas Exp $
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

RCSID(const char *sbirq_rcsid = "$Id: sbirq.c,v 1.2 1997/07/31 10:56:56 pekangas Exp $";)


#ifndef __DJGPP__
static void (__interrupt __far *oldIRQ)();
#endif

static void (*sbIRQ)(void) = NULL;



#ifdef __DJGPP__
int sbISR(void)
#else
void __interrupt __far sbISR(void)
#endif
{
    if ( sbIRQ != NULL )
        sbIRQ();
#ifdef __DJGPP__
    return 0;
#endif
}


void CALLING sbSetIRQ(unsigned intnum, void (*handler)(void))
{
    sbIRQ = handler;
#ifdef __DJGPP__
    _install_irq(intnum, sbISR);
#else
    oldIRQ = _dos_getvect(intnum);
    _dos_setvect(intnum, sbISR);
#endif
}



void CALLING sbRemoveIRQ(unsigned intnum)
{
#ifdef __DJGPP__
    _remove_irq(intnum);
#else
    /* Restore old interrupt vector: */
    _dos_setvect(intnum, oldIRQ);
#endif
}
