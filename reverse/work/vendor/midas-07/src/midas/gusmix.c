/*      gusmix.c
 *
 * GUS Software Mixing Sound Device
 *
 * $Id: gusmix.c,v 1.2 1997/03/05 16:50:17 pekangas Exp $
 *
 * Copyright 1996,1997 Housemarque Inc.
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#ifdef __DJGPP__
#include <internal.h>
#include <go32.h>
#include <sys/farptr.h>
#endif

#include <dos.h>
#include "lang.h"
#include "mtypes.h"
#include "errors.h"
#include "sdevice.h"
#include "mglobals.h"
#include "dsm.h"
#include "dma.h"
#include "mutils.h"
#include "mixsd.h"

RCSID(const char *gusmix_rcsid = "$Id: gusmix.c,v 1.2 1997/03/05 16:50:17 pekangas Exp $";)

/* Define to get border color debug code: */
/*#define GUSBORDERS*/

/* Define to get debug output on screen: */
/*#define GUSDEBUG */

/* Start address of the buffer in GUS memory. Note! The whole buffer must fit
   inside the first 64k! */
#define STARTADDRESS 32

/* Number of bytes of data to copy before the beginning and after the end of
   the buffer to reduce clicks: (in practise this should be unnecessary, as
   the frequency constant is always an integer and the GF1 shouldn't
   interpolate) */
#define UNCLICKCOPY 0

#ifdef GUSDEBUG
#define DEBUG_CODE(x) x
#else
#define DEBUG_CODE(x)
#endif

#ifdef GUSDEBUG
#include <stdio.h>
#include "vgatext.h"
#endif

#ifndef __DJGPP__
static void (__interrupt __far *oldIRQ)();
#endif

static volatile int dmaActive = 0;      /* DMA transfer is active */
static volatile unsigned writePos;      /* buffer write position */
static unsigned mode;                   /* actual output mode */
static unsigned rate;                   /* the real mixing rate */
static unsigned numChans;               /* the number of channels we opened */
static unsigned intNum;                 /* the interrupt number */

static dmaBuffer gusBuffer;             /* dummy DMA buffer */

/* Hack: These come from mixsd.c: */
extern unsigned dmaPos;
extern unsigned mixPos;
extern dmaBuffer buffer;
extern uchar *bufferPtr;

/* Defined at the end of the file: */
extern SoundDevice mGusMix;


/* GUS mixing rates for each number of open channels, from 14 to 32: */
static unsigned chanRates[19] =
    { 44100, 41160, 38587, 36317, 34300, 32494, 30870, 29400, 28063, 26843,
      25725, 24696, 23746, 22866, 22050, 21289, 20580, 19916, 19293 };





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

void outpw(unsigned port, unsigned value);
#pragma aux outpw = \
    "out    dx,ax" \
    parm [edx] [eax] \
    modify exact [];

unsigned inpw(unsigned port);
#pragma aux inpw = \
    "xor    eax,eax" \
    "in     ax,dx" \
    parm [edx] \
    value [eax] \
    modify exact [eax];

void EnableInterrupts(void);
#pragma aux EnableInterrupts = "sti" modify exact[];

void DisableInterrupts(void);
#pragma aux DisableInterrupts = "cli" modify exact[];

/* Set DS to ES: */
void SetDStoES(void);
#pragma aux SetDStoES = \
    "   mov     ax,ds" \
    "   mov     es,ax" \
    modify exact [eax];

#else

#define EnableInterrupts() ENABLE()
#define DisableInterrupts() DISABLE()

#endif


/* Set border color: */
#ifdef GUSBORDERS

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



/****************************************************************************\
*       enum mGusMixFunctIDs
*       --------------------
* Description:  ID numbers for GUS mixing SD functions
\****************************************************************************/

enum mGusMixFunctIDs
{
    ID_mGusMixDetect = ID_gdc,
    ID_mGusMixInit,
    ID_mGusMixClose,
    ID_mGusMixStartPlay,
    ID_mGusMixPlay
};


/* Prototypes for the ISR: */
#ifdef __DJGPP__
int mGusMixISR(void);
#else
void __interrupt __far mGusMixISR(void);
#endif



/****************************************************************************\
*
* Function:     void gusDelay(void)
*
* Description:  Standard GUS delay
*
\****************************************************************************/

static void mGusDelay(void)
{
    int         i;
    for ( i = 0; i < 8; i++ )
        inp(0x84);
}




/****************************************************************************\
*
* Function:     void mGusWriteB(unsigned reg, unsigned value)
*
* Description:  Writes a byte register
*
* Input:        unsigned reg            register number
*               unsigned value          value
*
\****************************************************************************/

void mGusWriteB(unsigned reg, unsigned value)
{
    outp(mGusMix.port + 0x103, reg);
    outp(mGusMix.port + 0x105, value);
}



/****************************************************************************\
*
* Function:     void mGusWriteW(unsigned reg, unsigned value)
*
* Description:  Writes a word register
*
* Input:        unsigned reg            register number
*               unsigned value          value
*
\****************************************************************************/

void mGusWriteW(unsigned reg, unsigned value)
{
    outp(mGusMix.port + 0x103, reg);
    outpw(mGusMix.port + 0x104, value);
}



/****************************************************************************\
*
* Function:     unsigned mGusReadB(unsigned reg)
*
* Description:  Reads a byte register
*
* Input:        unsigned reg            register number
*
* Returns:      Register value
*
\****************************************************************************/

unsigned mGusReadB(unsigned reg)
{
    outp(mGusMix.port + 0x103, reg);
    return inp(mGusMix.port + 0x105);
}



/****************************************************************************\
*
* Function:     unsigned mGusReadW(unsigned reg)
*
* Description:  Reads a word register
*
* Input:        unsigned reg            register number
*
* Returns:      Register value
*
\****************************************************************************/

unsigned mGusReadW(unsigned reg)
{
    outp(mGusMix.port + 0x103, reg);
    return inpw(mGusMix.port + 0x104);
}



/****************************************************************************\
*
* Function:     void mGusReset(void)
*
* Description:  Resets the GF1
*
\****************************************************************************/

static void mGusReset(void)
{
    mGusWriteB(0x4C, 0);                /* Reset, hold GF1 in reset state */
    mGusDelay();
    mGusDelay();
    mGusWriteB(0x4C, 0x01);             /* Reset, let GF1 run */
    mGusDelay();
    mGusDelay();
}




/****************************************************************************\
*
* Function:     void mGusSelectVoice(unsigned voice)
*
* Description:  Selects the voice to access
*
* Input:        unsigned voice          voice number
*
\****************************************************************************/

void CALLING mGusSelectVoice(unsigned voice)
{
    outp(mGusMix.port+0x102, voice);
}



/****************************************************************************\
*
* Function:     int mGusMixDetect(int *result)
*
* Description:  Attempts to autodetect the sound device
*
* Input:        int *result             1 if detected, 0 if not
*
* Returns:      MIDAS error code. Detection result is written to *result.
*
\****************************************************************************/

int CALLING mGusMixDetect(int *result)
{
    char        *ultrasnd;
    static char str[8];
    char        *p, *d;
    int         len;
    long        val;

    SetBorder(15);

    DEBUG_CODE(printf("mGusMixDetect\n"));

/* A helpful macro: */
#define NOGUS { DEBUG_CODE(printf("Failed\n")); *result = 0; return OK; }

    /* Get the ULTRASND environment variable. If we didn't get it, there is
       no card: */
    if ( (ultrasnd = mGetEnv("ULTRASND")) == NULL )
        NOGUS

    len = mStrLength(ultrasnd);
    if ( len < 10 )
        NOGUS

    /* Now we'll need to parse it */

    /* First get the port: */
    p = ultrasnd; d = str;
    d[0] = p[0]; d[1] = p[1]; d[2] = p[2]; d[3] = 0;
    len -= 4;   /* skip comma */
    p += 4;
    if ( (val = mHex2Long(d)) < 1 )
        NOGUS
    mGusMix.port = (unsigned) val;

    /* Get playback DMA channel number: */
    mGusMix.DMA = (*p) - '0';
    if ( mGusMix.DMA > 7 )
        NOGUS
    p += 4;     /* skip recording DMA */
    len -= 4;

    if ( len <= 0 )
        NOGUS

    /* Get GF1 IRQ: */
    mGusMix.IRQ = (*p) - '0'; p++; len--;
    if ( (len > 0) && ((*p) >= '0') && ((*p) <= '9') )
        mGusMix.IRQ = (10 * mGusMix.IRQ) + ((*p) - '0');
    if ( (mGusMix.IRQ < 1) || (mGusMix.IRQ > 15) )
        NOGUS

    SetBorder(14);

    /* Now check that we really have a GUS */

    mGusReset();

    /* Poke 0x55 to address 0: */
    mGusWriteB(0x44, 0);
    mGusWriteW(0x43, 0);
    outp(mGusMix.port + 0x107, 0x55);

    /* Poke 0xAA to address 1: */
    mGusWriteW(0x43, 1);
    outp(mGusMix.port + 0x107, 0xAA);

    /* Read back and check: */
    mGusWriteW(0x43, 0);
    if ( inp(mGusMix.port + 0x107) != 0x55 )
        NOGUS
    mGusWriteW(0x43, 1);
    if ( inp(mGusMix.port + 0x107) != 0xAA )
        NOGUS

    SetBorder(0);

    /* Fine, ULTRASND was OK and we could access the memory - GUS is
       probably OK */

    *result = 1;
    return OK;
}



/****************************************************************************\
*
* Function:     int mGusMixInit(unsigned mixRate, unsigned outputMode)
*
* Description:  Initializes the Sound Device
*
* Input:        unsigned mixRate        mixing rate
*               unsigned outputMode     output mode
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mGusMixInit(unsigned mixRate, unsigned outputMode)
{
    unsigned    i;
    int         error;
    unsigned    endAddr;
    unsigned    startAddr;
    unsigned    voiceControl;

    /* First, figure out the output mode: */
    if ( outputMode & sd8bit )
        mode = sd8bit;
    else
        mode = sd16bit;
    if ( outputMode & sdMono )
        mode |= sdMono;
    else
        mode |= sdStereo;

    /* Now find out the number of open channels that best matches the desired
       mixing rate: */
    for ( i = 14; i <= 32; i++ )
    {
        if ( chanRates[i-14] <= mixRate )
            break;
    }
    /* Yeah, we can't go much below 20kHz this way, but we'll need to set the
       increment on both channels to 2, and so this is our only way to
       control the output rate */

    numChans = i;
    rate = chanRates[numChans-14];

    /* Reset the GUS: */
    mGusReset();

    /* Disable line input and output, enable DMA/IRQ latches: */
    outp(mGusMix.port, 11);

    /* Set number of active voices to 32: */
    mGusWriteB(0x0E, 31 | 0xC0);

    /* Clear all 32 channels: */
    for ( i = 0; i < 32; i++ )
    {
        mGusSelectVoice(i);

        mGusWriteB(0, 3);               /* voice control: stop voice */
        mGusWriteW(9, 0x0500);          /* volume: zero volume */
        mGusWriteB(12, 8);              /* pan to center */
        mGusWriteB(13, 3);              /* disable volume ramping */
        mGusWriteB(6, 0x3F);            /* ramp rate */
    }

    /* Initialize generic DOS mixing sound device stuff with our output mode
       and mixing rate, but don't let it handle the DMA: */
    if ( (error = mixsdInit(rate, mode, -1)) != OK )
        PASSERROR(ID_mGusMixInit)

    mGusWriteB(0x4C, 7);                /* reset GF1, enable GF1, DACs and
                                           master IRQ */

    /* Set up IRQ: */
    if ( mGusMix.IRQ > 7 )
        intNum = 0x70 - 8 + mGusMix.IRQ;
    else
        intNum = 8 + mGusMix.IRQ;
#ifdef __DJGPP__
    if ( _install_irq(intNum, mGusMixISR) != 0 )
    {
        ERROR(errUndefined, ID_mGusMixInit);
        return errUndefined;
    }
#else
    oldIRQ = _dos_getvect(intNum);
    _dos_setvect(intNum, mGusMixISR);
#endif

    /* Send specific EOI for the IRQ: */
    if ( mGusMix.IRQ > 7 )
        outp(0xA0, 0x60 | (mGusMix.IRQ & 7));
    else
        outp(0x20, 0x60 | (mGusMix.IRQ & 7));

    /* Enable it: */
    if ( mGusMix.IRQ > 7 )
        outp(0xA1, inp(0xA1) & (~(1 << (mGusMix.IRQ - 8))));
    else
        outp(0x21, inp(0x21) & (~(1 << mGusMix.IRQ)));

    /* The GUS buffer is at address 0 of the memory, and length is
       dmaBufferSize. Now clear it. (the length is always below 64k) */

    mGusWriteW(0x44, 0);                /* address upper bits to 0 */

    for ( i = (STARTADDRESS - 32); i < (STARTADDRESS + dmaBufferSize + 32);
        i++ )
    {
        mGusWriteW(0x43, i);            /* address lower bits */
        outp(mGusMix.port + 0x107, 0);  /* write 0 */
    }

    /* Open the correct number of channels to get our desired mixing rate: */
    mGusWriteB(0x0E, 0xC0 | (numChans-1));

    /* Calculate sample start and end addresses: */
    if ( mode & sd16bit )
    {
        startAddr = STARTADDRESS / 2;
        endAddr = ((STARTADDRESS + dmaBufferSize) / 2) - 1;
    }
    else
    {
        startAddr = STARTADDRESS;
        endAddr = STARTADDRESS + dmaBufferSize - 1;
    }

    DEBUG_CODE(printf("startAddr = %u, endAddr = %u\n", startAddr, endAddr));

    /* Determine the correct value for Voice Control register: */
    if ( mode & sd16bit )
        voiceControl = 4 | 8;                   /* 16-bit, loop */
    else
        voiceControl = 8;                       /* loop */

    /* For mono, set the Frequency Control register of channel 0 to 1, for
       stereo, channels 0 and 1 to 2. This ensures that the GF1 won't
       interpolate the data, and interleaved stereo data gets played
       correctly. */

    /* Set up the channels: */
    if ( mode & sdMono )
    {
        mGusSelectVoice(0);
        mGusWriteW(1, 1 << 10);                     /* Frequency Control */
        mGusWriteW(4, endAddr >> 7);                /* End Address high */
        mGusWriteW(5, (endAddr << 9) & 0xFFFF);     /* End Address low */
        mGusWriteW(2, startAddr >> 7);              /* Start Address high */
        mGusWriteW(3, (startAddr << 9) & 0xFFFF);   /* Start Address low */
        mGusWriteW(10, startAddr >> 7);             /* Current Address high */
        mGusWriteW(11, (startAddr << 9) & 0xFFFF);  /* Current Address low */
        mGusDelay();
        mGusWriteW(10, startAddr >> 7);             /* Current Address high */
        mGusWriteW(11, (startAddr << 9) & 0xFFFF);  /* Current Address low */
        mGusWriteB(0, voiceControl);
        mGusDelay();
        mGusWriteB(0, voiceControl);
        mGusWriteW(9, 0xEF00);                      /* volume to maximum */
        mGusDelay();
        mGusWriteW(9, 0xEF00);                      /* volume to maximum */
    }
    else
    {
        mGusSelectVoice(0);
        mGusWriteW(1, 2 << 10);                     /* Frequency Control */
        mGusWriteW(4, (endAddr - 1) >> 7);          /* End Address high */
        mGusWriteW(5, ((endAddr - 1) << 9) & 0xFFFF);  /* End Address low */
        mGusWriteW(2, startAddr >> 7);              /* Start Address high */
        mGusWriteW(3, (startAddr << 9) & 0xFFFF);   /* Start Address low */
        mGusWriteW(10, startAddr >> 7);             /* Current Address high */
        mGusWriteW(11, (startAddr << 9) & 0xFFFF);  /* Current Address low */
        mGusDelay();
        mGusWriteW(10, startAddr >> 7);             /* Current Address high */
        mGusWriteW(11, (startAddr << 9) & 0xFFFF);  /* Current Address low */
        mGusWriteB(0, voiceControl);
        mGusDelay();
        mGusWriteB(0, voiceControl);
        mGusWriteW(9, 0xEF00);                      /* volume to maximum */
        mGusDelay();
        mGusWriteW(9, 0xEF00);                      /* volume to maximum */
        mGusWriteB(12, 0);                          /* pan to extreme left */

        mGusSelectVoice(1);
        mGusWriteW(1, 2 << 10);                     /* Frequency Control */
        mGusWriteW(4, endAddr >> 7);                /* End Address high */
        mGusWriteW(5, (endAddr << 9) & 0xFFFF);     /* End Address low */
        mGusWriteW(2, (startAddr + 1) >> 7);        /* Start Address high */
        mGusWriteW(3, ((startAddr + 1) << 9) & 0xFFFF);/* Start Address low */
        mGusWriteW(10, (startAddr + 1) >> 7);       /* Current Address high */
        mGusWriteW(11, ((startAddr + 1) << 9) & 0xFFFF); /* Curr. Addr. low */
        mGusDelay();
        mGusWriteW(10, (startAddr + 1) >> 7);       /* Current Address high */
        mGusWriteW(11, ((startAddr + 1) << 9) & 0xFFFF); /* Curr. Addr. low */
        mGusWriteB(0, voiceControl);
        mGusDelay();
        mGusWriteB(0, voiceControl);
        mGusWriteW(9, 0xEF00);                      /* volume to maximum */
        mGusDelay();
        mGusWriteW(9, 0xEF00);                      /* volume to maximum */
        mGusWriteB(12, 15);                         /* pan to extreme right */

        /* Reset the Current Addresses once again, to get the positions as
           close to each other as possible: */
        mGusSelectVoice(0);
        mGusWriteW(10, startAddr >> 7);
        mGusWriteW(11, (startAddr << 9) & 0xFFFF); mGusDelay();
        mGusWriteW(10, startAddr >> 7);
        mGusWriteW(11, (startAddr << 9) & 0xFFFF);
        mGusSelectVoice(1);
        mGusWriteW(10, (startAddr + 1) >> 7);
        mGusWriteW(11, ((startAddr + 1) << 9) & 0xFFFF); mGusDelay();
        mGusWriteW(10, (startAddr + 1) >> 7);
        mGusWriteW(11, ((startAddr + 1) << 9) & 0xFFFF);
    }

    /* Disable all GUS DMA: */
    dmaStop(mGusMix.DMA);
    mGusWriteB(0x41, 0);

    /* Enable GUS line input and output, and DMA and IRQ latches: */
    outp(mGusMix.port, 8);

    dmaActive = 0;
    writePos = 0;

    return OK;
}




/****************************************************************************\
*
* Function:     int mGusMixClose(void)
*
* Description:  Uninitializes the Sound Device
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mGusMixClose(void)
{
    int         error;

    /* Stop all GUS DMA: */
    dmaStop(mGusMix.DMA);
    mGusWriteB(0x41, 0);

    mGusReset();

    /* Enable line in and out: */
    mGusWriteB(mGusMix.port, 0);

    /* Remove GUS IRQ: */
#ifdef __DJGPP__
    _remove_irq(intNum);
#else
    /* Restore old interrupt vector: */
    _dos_setvect(intNum, oldIRQ);
#endif

    /* Disable it: */
    if ( mGusMix.IRQ > 7 )
        outp(0xA1, inp(0xA1) | (1 << (mGusMix.IRQ - 8)));
    else
        outp(0x21, inp(0x21) | (1 << mGusMix.IRQ));

    /* Take care of common uninitialization: */
    if ( (error = mixsdClose()) != OK )
        PASSERROR(ID_mGusMixClose);

    return OK;
}



/****************************************************************************\
*
* Function:     int CALLING mGusMixStartPlay(void)
*
* Description:  Reads and updates the DMA position for mixsd
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING mGusMixStartPlay(void)
{
    unsigned    position;
    unsigned    pos2;

    /* Get current playback position for channel 0: */
    mGusSelectVoice(0);
    do
    {
        position = (mGusReadW(0x8A) & 0x1FFF) << 7; /* current address high */
        position |= mGusReadW(0x8B) >> 9;            /* current address low */
        if ( mode & sd16bit )
            position *= 2;
        mGusDelay();
        pos2 = (mGusReadW(0x8A) & 0x1FFF) << 7; /* current address high */
        pos2 |= mGusReadW(0x8B) >> 9;            /* current address low */
        if ( mode & sd16bit )
            pos2 *= 2;
    } while ( ((pos2 - position) > 2) || (position < STARTADDRESS) ||
        (position >= (STARTADDRESS + dmaBufferSize)) );

    dmaPos = position - STARTADDRESS;

    return OK;
}




/****************************************************************************\
*
* Function:     void mGusMixUpload(void)
*
* Description:  Start uploading data with DMA, if there is something left
*               to upload
*
\****************************************************************************/

void mGusMixUpload(void)
{
    unsigned    numBytes;
    unsigned    fixMixPos;
#if UNCLICKCOPY > 0
    unsigned    a, i;
    uchar       *p;
#endif
    unsigned    gusAddr;
    unsigned    dmaControl;
    static unsigned rejectcount = 0;

#ifdef GUSDEBUG
    vgaWriteByte(5, 2, writePos >> 8, 0x0F);
    vgaWriteByte(7, 2, (writePos & 0xFF), 0x0F);
    vgaWriteByte(9, 2, mixPos >> 8, 0x1F);
    vgaWriteByte(11, 2, (mixPos & 0xFF), 0x1F);
#endif

    /* Calculate the amount of free space left: */
    fixMixPos = mixPos & (~31);
    if ( fixMixPos >= writePos )
        numBytes = fixMixPos - writePos;
    else
        numBytes = dmaBufferSize - writePos + fixMixPos;

    if ( numBytes < 64 )
    {
        SetBorder(0);
        dmaActive = 0;
        rejectcount++;
        DEBUG_CODE(vgaWriteByte(1, 2, rejectcount, 0x2F));
        return;
    }
    dmaActive = 1;

    /* Make sure we won't go past the buffer end: */
    if ( (writePos + numBytes) >= dmaBufferSize )
        numBytes = dmaBufferSize - writePos;

    /* Stop all DMA transfer: */
    dmaStop(mGusMix.DMA);
    mGusWriteB(0x41, 0);
    mGusDelay();
    mGusWriteB(0x41, 0);

#if UNCLICKCOPY > 0
    /* If we will be uploading to the beginning of the buffer, write a few
       samples from the beginning to the area past buffer end, to prevent
       loop clicks: */
    if ( writePos == 0 )
    {
        p = bufferPtr;
        a = STARTADDRESS + dmaBufferSize;
        mGusWriteB(0x44, 0);            /* address upper bits to 0 */
        if ( mode & sd8bit )
        {
            for ( i = 0; i < UNCLICKCOPY; i++ )
            {
                mGusWriteW(0x43, a);        /* address lower bits */
                outp(mGusMix.port + 0x107, (*(p++)) ^ 0x80);
                a++;
            }
        }
        else
        {
            for ( i = 0; i < UNCLICKCOPY; i++ )
            {
                mGusWriteW(0x43, a);        /* address lower bits */
                outp(mGusMix.port + 0x107, *(p++));
                a++;
            }
        }
    }

    /* Similarily, if we will be uploading to the very end of the buffer,
       write some data from the end of the buffer to the area immediately
       before loop start: */
    if ( (writePos + numBytes) >= dmaBufferSize )
    {
        p = bufferPtr + dmaBufferSize - UNCLICKCOPY;
        a = STARTADDRESS - UNCLICKCOPY;
        mGusWriteB(0x44, 0);            /* address upper bits to 0 */
        if ( mode & sd8bit )
        {
            for ( i = 0; i < UNCLICKCOPY; i++ )
            {
                mGusWriteW(0x43, a);        /* address lower bits */
                outp(mGusMix.port + 0x107, (*(p++)) ^ 0x80);
                a++;
            }
        }
        else
        {
            for ( i = 0; i < UNCLICKCOPY; i++ )
            {
                mGusWriteW(0x43, a);        /* address lower bits */
                outp(mGusMix.port + 0x107, *(p++));
                a++;
            }
        }
    }
#endif /* #if UNCLICKCOPY > 0 */


    /* We'll now start uploading. First prepare a dummy DMA buffer for the
       transfer: */
    gusBuffer.startAddr = buffer.startAddr + writePos;
    gusBuffer.bufferLen = numBytes;
    gusBuffer.channel = mGusMix.DMA;

    /* Set GUS DMA transfer address: (always inside the first 256k block) */
    if ( mGusMix.DMA > 3 )
        gusAddr = (STARTADDRESS + writePos) >> 5;
    else
        gusAddr = (STARTADDRESS + writePos) >> 4;
    mGusWriteW(0x42, gusAddr);

    /* Prepare value for DMA control register: */
/*00101001*/
    dmaControl = 0x21;                  /* Enable DMA, direction write, rate
                                           divider 2 (325kHz), enable IRQ */
    if ( mGusMix.DMA > 3 )
        dmaControl |= 4;                /* 16-bit DMA channel */
    if ( mode & sd16bit )
        dmaControl |= 0x40;             /* 16-bit data */
    else
        dmaControl |= 0x80;             /* 8-bit data, invert MSB */

    /* Start GUS DMA: */
    mGusWriteB(0x41, dmaControl);
    mGusDelay();
    mGusWriteB(0x41, dmaControl);

    /* And start the DMA controller: */
    dmaPlayBuffer(&gusBuffer, mGusMix.DMA, 0);

    writePos += numBytes;
    if ( writePos >= dmaBufferSize )
        writePos = 0;

    SetBorder(1);
}




/****************************************************************************\
*
* Function:     int mGusMixPlay(int *callMP)
*
* Description:  Mixes sound data and uploads it to the GUS
*
* Input:        int *callMP             pointer to music player calling flag
*
* Returns:      MIDAS error code. If enough data was mixed for one updating
*               round and music player should be called, 1 is written to
*               *callMP, otherwise 0 is written there. Note that if music
*               player can be called, mixsdPlay() should be called again
*               with a new check for music playing to ensure the DMA buffer
*               gets filled with new data.
*
\****************************************************************************/

int CALLING mGusMixPlay(int *callMP)
{
    int         error;

    /* Call the generic play function - mix data etc: */
    if ( (error = mixsdPlay(callMP)) != OK )
        PASSERROR(ID_mGusMixPlay)

    /* We probably have something new to upload. If the DMA isn't active at
       the moment, start it. Otherwise the IRQ will handle this. */
    DisableInterrupts();
    if ( !dmaActive )
    {
        dmaActive = 1;
        EnableInterrupts();
        mGusMixUpload();
    }
    else
        EnableInterrupts();

    return OK;
}



/****************************************************************************\
*
* Function:     int mGusMixISR(void)
*
* Description:  The Interrupt Service Routine
*
\****************************************************************************/

#ifdef __DJGPP__
int mGusMixISR(void)
#else
void __interrupt __far mGusMixISR(void)
#endif
{
    unsigned    irqSource;
    int         dmaIRQ;
/*    static unsigned    i = 14; */
    unsigned    voiceIRQ;
    ulong       ignoreVT, ignoreRamp, chbit;
    unsigned    temp;
    int         maxTotCount = 16;
    int         maxVocCount;
    static unsigned dmairqcount = 0;

/*    i++;
    SetBorder(i); */

    /* Set DS to ES as well: (stupid Watcom doesn't to this, and then assumes
       ES is valid) */
#ifndef __DJGPP__
    SetDStoES();
#endif

    /* Handle all IRQs: */
    dmaIRQ = 0;
    while ( maxTotCount )
    {
        /* Get IRQ source: */
        irqSource = inp(mGusMix.port + 6);
        maxTotCount--;

/*        SetBorder(i);
        i = i + 1; */

        if ( irqSource == 0 )
            break;

        DEBUG_CODE(vgaWriteByte(1, 1, irqSource, 0x1F));

        if ( irqSource & 0x03 )
        {
            /* MIDI transmit or receive IRQ */
            DEBUG_CODE(vgaWriteByte(3, 1, inp(mGusMix.port + 0x100), 0x2F));
            if ( irqSource & 0x02 )
                inp(mGusMix.port + 0x101);
        }

        if ( irqSource & 0x0C )
        {
            /* Timer 1 or 2 IRQ */
            DEBUG_CODE(vgaWriteByte(5, 1, mGusReadB(0x45), 0x3F));
            mGusWriteB(0x45, 0);
        }

        if ( irqSource & 0x60 )
        {
            /* Handle channel IRQs: */
            ignoreVT = 0;
            ignoreRamp = 0;
            maxVocCount = 16;
            while ( maxVocCount )
            {
                maxVocCount--;
                /* Get voice number and IRQ type: */
                voiceIRQ = mGusReadB(0x8F);
                chbit = 1 << (voiceIRQ & 0x1F);

                DEBUG_CODE(vgaWriteByte(7, 1, voiceIRQ, 0x4F));

                if ( (voiceIRQ & 0x40) && !(ignoreRamp & chbit) )
                {
                    /* volume ramp IRQ */
                    ignoreRamp |= chbit;
                    mGusSelectVoice(voiceIRQ & 0x1F);
                    mGusReadB(0x8D);
                    mGusWriteB(0x0D, 3);
                    mGusDelay();
                    mGusWriteB(0x0D, 3);
                }

                if ( (voiceIRQ & 0x80) && !(ignoreVT & chbit) )
                {
                    /* Wavetable IRQ */
                    ignoreRamp |= chbit;
                    mGusSelectVoice(voiceIRQ & 0x1F);
                    temp = mGusReadB(0x80);
                    mGusWriteB(0, (temp & (~0xA0)));
                    mGusReadB(0x8D);
                }
            }
        }

        if ( irqSource & 0x80 )
        {
            /* DMA IRQ */
            temp = mGusReadB(0x41);
            DEBUG_CODE(vgaWriteByte(9, 1, temp, 0x1F));
            if ( temp & 0x40 )
                dmaIRQ = 1;
            DEBUG_CODE(vgaWriteByte(11, 1, mGusReadB(0x49), 0x2F));
        }
    }

    /* Check if we got a DMA Terminal Count IRQ. If yes, handle it. We are
       not interested in other IRQs here. */
    if ( dmaIRQ && dmaActive )
    {
        dmairqcount++;
        DEBUG_CODE(vgaWriteByte(3, 2, dmairqcount, 0x4F));
        mGusMixUpload();
    }

    /* Acknowledge the IRQ: */
    outp(0x20, 0x20);
    if ( mGusMix.IRQ > 7 )
        outp(0xA0, 0x20);

    EnableInterrupts();

#ifdef __DJGPP__
    return 0;                           /* don't chain */
#endif
}



static char     *cardName = "Gravis UltraSound (software mixing)";

static unsigned portAddresses[6] =
    { 0x210, 0x220, 0x230, 0x240, 0x250, 0x260 };



    /* The Sound Device structure: */

SoundDevice     mGusMix = {
    0,                                  /* tempoPoll = 0 */
    sdUsePort | sdUseIRQ | sdUseDMA | sdUseMixRate | sdUseOutputMode |
        sdUseDSM,                       /* configBits */
    0x220,                              /* port */
    7,                                  /* IRQ */
    1,                                  /* DMA */
    1,                                  /* cardType */
    1,                                  /* numCardTypes */
    sdMono | sdStereo | sd8bit | sd16bit,       /* modes */

    "Gravis UltraSound Software Mixing Sound Device",    /* name */
    &cardName,                          /* cardNames */
    6,                                  /* numPortAddresses */
    portAddresses,                      /* portAddresses */

    &mGusMixDetect,
    &mGusMixInit,
    &mGusMixClose,
    &dsmGetMixRate,
    &mixsdGetMode,
    &mixsdOpenChannels,
    &dsmCloseChannels,
    &dsmClearChannels,
    &dsmMute,
    &dsmPause,
    &dsmSetMasterVolume,
    &dsmGetMasterVolume,
    &mixsdSetAmplification,
    &mixsdGetAmplification,
    &dsmPlaySound,
    &dsmReleaseSound,
    &dsmStopSound,
    &dsmSetRate,
    &dsmGetRate,
    &dsmSetVolume,
    &dsmGetVolume,
    &dsmSetSample,
    &dsmGetSample,
    &dsmSetPosition,
    &dsmGetPosition,
    &dsmGetDirection,
    &dsmSetPanning,
    &dsmGetPanning,
    &dsmMuteChannel,
    &dsmAddSample,
    &dsmRemoveSample,
    &mixsdSetUpdRate,
    &mGusMixStartPlay,
    &mGusMixPlay
#ifdef SUPPORTSTREAMS
    ,
    &dsmStartStream,
    &dsmStopStream,
    &dsmSetLoopCallback,
    &dsmSetStreamWritePosition
#endif
};




/*
 * $Log: gusmix.c,v $
 * Revision 1.2  1997/03/05 16:50:17  pekangas
 * Minor border color debug code changes
 *
 * Revision 1.1  1997/02/27 15:58:55  pekangas
 * Initial revision
 *
*/
