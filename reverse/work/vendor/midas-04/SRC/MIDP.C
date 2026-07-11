/*      MIDP.C
 *
 * MIDAS Module Player v1.00b
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
#include <dir.h>
#include <conio.h>
#include <dos.h>
#include <alloc.h>
#include <string.h>
#include <process.h>
#include <ctype.h>
#include <time.h>

#include "midas.h"
#include "vgatext.h"
#include "diverr.h"

#define MIDPVERSION 1.00b
#define MIDPVERSTR "1.00b"
#define MIDPVERNUM 0x0100

char            *title =
"MIDAS Module Player v" MIDPVERSTR ", Copyright 1995 Petteri Kangaslampi & "
"Jarno Paananen\n";

char            *usage =
"Usage:\tMIDP\t[options] <filenames> [options]\n\n"
"Options:\n"
"-r     Reconfigure                      -S     Immediate DOS shell\n"
"-sx    Sound Device x (1=GUS, 2=PAS,    -y     Disable screen syncronization"
"\n"
"       3=WSS, 4=SB, 5=No Sound)         -Y     Enable screen synch. in shell"
"\n"
"-cx    Sound card type x                -Lx    Song loops before next song\n"
"-pxxx  Use I/O port xxx (HEX)           -O     Scramble module playing order"
"\n"
"-ix    Use IRQ x                        -nxx   Default panning +/- xx \n"
"-dx    Use DMA x                        -t     Disable Protracker BPM tempos"
"\n"
"-mxxx  Mixing rate xxx Hz               -g     Disable Protracker panning\n"
"-e     Disable EMS usage                -x     Enable extended octaves\n"
"-u     Enable surround sound            -b     Disable Virtual DMA usage\n"
"-v     Disable real VU meters           -M     Display memory information\n"
"-oxx   Output mode (8=8-bit, 1=16-bit   -ax    Force amplification level x\n"
"       s=stereo, m=mono)";

char            *scrtop =
"\xFF\x4FMIDAS Module Player v" MIDPVERSTR " - Copyright 1995 Petteri Kangaslampi & "
"Jarno Paananen"
"\xFF\x0FÞ\xFF\x7F\x7F\x4Eß\xFF\x08Ý"
"\xFF\x0FÞ\xFF\x78Ú\x7F\x4CÄ\xFF\x7F¿\xFF\x08Ý"
"\xFF\x0FÞ\xFF\x78³\xFF\x7FModule:\x7F\x1F Type:\x7F\x21 ³\xFF\x08Ý"
"\xFF\x0FÞ\xFF\x78³\xFF\x7FMixing Rate:\x7F\x0C Mixing Mode:\x7F\x1B Time:"
"\x7F\x08 ³\xFF\x08Ý"
"\xFF\x0FÞ\xFF\x78³\xFF\x7FLength:    Position:    Pattern:    Row:    Tempo:"
"     Speed:    Vol:       ³\xFF\x08Ý"
"\xFF\x0FÞ\xFF\x78À\xFF\x7F\x7F\x4CÄÙ\xFF\x08Ý";


char    *help =
"                MIDAS Module Player keys:\n"
"                ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n"
"\n"
"Alt-X           Quick exit\n"
"Left/Right      Next / Previous position\n"
"Up/Down         Select active channel\n"
"Esc             Exit with fade out\n"
"+/-             Increase / Decrease master volume\n"
"D               Jump to DOS shell\n"
"1-9, 0          Toggle channels on/off\n"
"TAB             One/two column instrument list\n"
"Page Up/Down    Scroll instrument list up/down\n"
",/.             Adjust active channel panning left / right\n"
"M               Set active channel panning to middle\n"
"L               Set active channel panning to left\n"
"R               Set active channel panning to right\n"
"U               Set active channel panning to surround\n"
"T               Toggle active channel on/off\n"
"P               Pause module. Press P to continue\n"
"Space           Mute playing. Press Space again to un-mute\n"
"N               Next module\n"
"V               Show MIDAS/MIDP version information\n"
"F               Show free memory information\n"
"A               Amplification adjust. Use ,/. to\n"
"                divide/multiply by 2, +/- to\n"
"                add/substract 16, V to toggle meter mode\n"
"                and A to toggle automatic limits.\n"
"F8              Toggle screen blank on/off\n";

    /* pointers to all Module Players: */
ModulePlayer    *modulePlayers[NUMMPLAYERS] =
    { &mpS3M,
      &mpMOD,
      &mpMTM };



#define MAXNAMES 256                    /* maximum number of file names */


SoundDevice     *SD;
ModulePlayer    *MP;
ulong           free1, free2;
mpModule        *mod;
int             numChans;
ushort          scrSync;
int             immShell = 0;
int             sync = 1, shellSync = 0;
volatile ulong  frameCnt;
mpInformation   *info;
ushort          actch = 0;
char            masterVol = 64;
char            exitFlag = 0;
char            fadeOut;
short           loopCnt = 0;
ushort          defPanning;
int             useDefPanning = 0;
char            *fNames[MAXNAMES];
time_t          startTime, pauseTime = 0, pauseStart;
int             muted = 0, paused = 0;
int             numFNames;
int             fileNumber;
int             noNext;
int             isArchive;              /* is file archive */
int             arcType;                /* archive type */
int             decompressed;           /* 1 if already decompressed */
char            *decompName = NULL;     /* name of decompressed module file */
char            *configFileName;        /* configuration file name */
int             reconfigure = 0;        /* 1 if force reconfiguration */
unsigned        amplification = 64;
int             instAdd = 0;            /* instrument display offset */
int             instMode = 0;           /* instrument display mode */

char            *tempDir;               /* temporary directory for
                                           decompression */
int             scrambleOrder = 0;
int             showUsage = 0;
int             showMem = 0;            /* show memory information? */
int             realVU = 1;             /* use real VU-meters */
int             realVUDisp = 1;         /* display real VU-meters */
int             scrBlank = 0;           /* is screen blanked? */

char            *modTypes[3] = {        /* module type strings */
    { "Scream Tracker ]I[" },
    { "Protracker" },
    { "Multitracker"} };

char            *notes[13] = {          /* note strings */
    "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};

#define DECOMPMEM 350000                /* number of bytes of memory required
                                           for decompression */
#define NUMARCEXTS 4
char            *arcExtensions[NUMARCEXTS] =
    { ".ZIP", ".MDZ", ".S3Z", ".MTZ" };

char            Channels[32];           /* channel mute flags */
uchar           oldInsts[32];           /* old instrument values for all
                                           channels */
struct text_info  textInfo;             /* text mode information */


/****************************************************************************\
*
* Function:     void Error(char *msg)
*
* Description:  Prints an error message to stderr, uninitializes MIDAS and
*               exits to DOS
*
* Input:        char *msg               error message
*
\****************************************************************************/

void Error(char *msg)
{
    textmode(C80);
    fprintf(stderr, "Error: %s\n", msg);
    midasClose();
    exit(EXIT_FAILURE);
}




/****************************************************************************\
*
* Function:     void toggleChannel(char channel)
*
* Description:  Toggles a channel mute on/off
*
* Input:        char channel            channel number
*
\****************************************************************************/

void toggleChannel(char channel)
{
    if ( channel < numChans )
    {
        Channels[channel] ^= 1;
        SD->MuteChannel(channel, Channels[channel]);
    }
}




/****************************************************************************\
*
* Function:     void prevr(void)
*
* Description:  preVR() routine for Timer, increments frame counter
*
\****************************************************************************/

void prevr(void)
{
    frameCnt++;
}




/****************************************************************************\
*
* Function:     void CheckHeap(void)
*
* Description:  Checks that the heap is OK, and displays a warning if not.
*               Also displays amount of free memory and amount of memory used
*               if the command line option "-M" was entered.
*
\****************************************************************************/

void CheckHeap(void)
{
    free2 = coreleft();

    if ( showMem )
        cprintf("%lu bytes memory free - %lu bytes used.\r\n",
            free2, free1-free2);

    if ( heapcheck() != _HEAPOK )
        cputs("HEAP CORRUPTED - PREPARE FOR SYSTEM CRASH!\r\n");
}




/****************************************************************************\
*
* Function:     void ShowFree(void)
*
* Description:  Displays amount of free memory and amount of memory used,
*               and checks for heap corruption.
*
\****************************************************************************/

void ShowFree(void)
{
    free2 = coreleft();

    cprintf("%lu bytes memory free - %lu bytes used.\r\n",
        free2, free1-free2);

    if ( heapcheck() != _HEAPOK )
        cputs("HEAP CORRUPTED - PREPARE FOR SYSTEM CRASH!\r\n");
}




/****************************************************************************\
*
* Function:     void dumpheap(void)
*
* Description:  Lists all blocks in heap
*
\****************************************************************************/

void dumpheap(void)
{
    struct heapinfo hinfo;

    if ( heapcheck() == _HEAPOK )
        cputs("Heap OK\r\n");
    else
        cputs("HEAP CORRUPTED!\r\n");

    hinfo.ptr = NULL;

    while ( heapwalk(&hinfo) == _HEAPOK )
    {
        cprintf("%p: %06u - ", hinfo.ptr, hinfo.size);
        if ( hinfo.in_use == 1 )
            cprintf("USED\r\n");
        else
            cprintf("FREE\r\n");
    }
}




/****************************************************************************\
*
* Function:     void dumpfree(void)
*
* Description:  Lists all free blocks in heap
*
\****************************************************************************/

void dumpfree(void)
{
    struct heapinfo hinfo;

    if ( heapcheck() == _HEAPOK )
        cputs("Heap OK\r\n");
    else
        cputs("HEAP CORRUPTED!\r\n");

    hinfo.ptr = NULL;

    while ( heapwalk(&hinfo) == _HEAPOK )
    {
        if ( hinfo.in_use != 1 )
            cprintf("%p: %06u\r\n", hinfo.ptr, hinfo.size);
    }
}




/****************************************************************************\
*
* Function:     void ParseOption(char *option)
*
* Description:  Parses one command line option
*
* Input:        char *option            command line option (without the
*                                       leading '-' or '/'
*
\****************************************************************************/

void ParseOption(char *option)
{
    switch ( option[0] )
    {
        case 'r':
            reconfigure = 1;
            break;

        case 'S':
            immShell = 1;
            break;

        case 'Y':
            shellSync = 1;
            break;

        case 'y':
            sync = 0;
            break;

        case 'L':
            loopCnt = mDec2Long(&option[1]);
            if ( loopCnt < 1 )
                loopCnt = 1;
            break;

        case 'O':
            scrambleOrder = 1;
            break;

        case 'n':
            defPanning = mDec2Long(&option[1]);
            useDefPanning = 1;
            break;

        case '?':
        case 'h':
            showUsage = 1;
            break;

        case 'M':
            showMem = 1;
            break;

        case 'v':
            realVU = 0;
            break;

        default:
            midasParseOption(option);
            break;
    }
}



/****************************************************************************\
*
* Function:     void ParseCmdLine(int argc, char *argv[])
*
* Description:  Parses command line options
*
* Input:        int argc                argc from main()
*               char *argv[]            argv from main()
*
\****************************************************************************/

void ParseCmdLine(int argc, char *argv[])
{
    int         i, error;

    numFNames = 0;

    for ( i = 1; i < argc; i++ )
    {
        if ( (argv[i][0] == '-') || (argv[i][0] == '/') )
        {
            ParseOption(&argv[i][1]);
        }
        else
        {
            if ( numFNames >= MAXNAMES )
                Error("Too many file names");

            fNames[numFNames] = argv[i];
            numFNames++;
        }
    }
}



/****************************************************************************\
*
* Function:     void ParseOptionString(char *optStr)
*
* Description:  Parses a string of command line options (eg. "MIDP"
*               environment variable)
*
* Input:        const char *optStr      String of options. Not modified.
*
\****************************************************************************/

void ParseOptionString(const char *optStr)
{
    char        *str, *opt;
    int         spos, slen, stopparse, error;

    slen = mStrLength(optStr);

    /* allocate memory for a copy of the string: */
    if ( (error = memAlloc(slen+1, (void**) &str)) != OK )
        midasError(error);

    /* copy environment string to midasenv: */
    mStrCopy(str, optStr);

    spos = 0;                       /* search position = 0 */
    opt = NULL;                     /* current option string = NULL */
    stopparse = 0;

    /* parse the whole environment string: */
    while ( !stopparse )
    {
        switch ( str[spos] )
        {
            case ' ':
                /* current character is space - change it to '\0' and
                    parse this option string if it exists*/
                str[spos] = 0;
                if ( opt != NULL )
                    ParseOption(opt);

                opt = NULL;         /* no option string */
                spos++;             /* next character */
                break;

            case 0:
                /* Current character is '\0' - end. Parse option string
                    if it exists and stop parsing. */
                if ( (opt != NULL) && (*opt != 0) )
                    ParseOption(opt);
                stopparse = 1;
                break;

            case '-':
            case '/':
                /* Current character is '-' or '/' - option string starts
                    from next character */
                spos++;
                opt = &str[spos];
                break;

            default:
                /* some normal character - continue parsing from next
                    character */
                spos++;
        }
    }

    if ( (error = memFree(str)) != OK )
        midasError(error);
}




/****************************************************************************\
*
* Function:     void InitMIDAS(void)
*
* Description:  Initializes MIDAS Sound System
*
\****************************************************************************/

void InitMIDAS(void)
{
    int         error;

    /* Get screen synchronization value if Timer should be synchronized to
       screen: */
    if ( sync )
    {
        if ( (error = tmrGetScrSync(&scrSync)) != OK )
            midasError(error);
    }

    midasInit();

    SD = midasSD;                       /* current Sound Device */

    if ( realVU )
    {
        /* initialize real VU-meters: */
        if ( (error = vuInit()) != OK )
            midasError(error);
    }

    /* Synchronize timer to screen, if synchronization is used. prevr() will
       be called before each retrace: */
    if ( sync )
    {
        if ( (error = tmrSyncScr(scrSync, &prevr, NULL, NULL)) != OK )
            midasError(error);
    }

    cprintf("MIDAS Sound System v" MVERSTR " succesfully initialized.\r\n"
        "Playing through %s\r\n", SD->cardNames[SD->cardType-1]);
    if ( SD->configBits != 0 )
    {
        cprintf("Using ");
        if ( SD->configBits & sdUsePort )
            cprintf("port %Xh", (unsigned) SD->port);
        if ( SD->configBits & sdUseIRQ )
            cprintf(", IRQ %i", (int) SD->IRQ);
        if ( SD->configBits & sdUseDMA )
            cprintf(" and DMA %i", (int) SD->DMA);
    }
    cprintf("\r\n");

    CheckHeap();
}





/****************************************************************************\
*
* Function:     void CloseMIDAS(void)
*
* Description:  Uninitializes MIDAS Sound System
*
\****************************************************************************/

void CloseMIDAS(void)
{
    int         error;

    /* Remove screen synchronization if used: */
    if ( sync )
    {
        if ( (error = tmrStopScrSync()) != OK )
            midasError(error);
    }

    if ( realVU )
    {
        /* Uninitialize real VU-meters: */
        if ( (error = vuClose()) != OK )
            midasUninitError(error);
    }

    midasClose();

    cprintf("MIDAS Sound System succesfully uninitialized\r\n");
}




/****************************************************************************\
*
* Function:     void WaitVR(void)
*
* Description:  Waits for Vertical Retrace
*
\****************************************************************************/

void WaitVR(void)
{
asm     mov     dx,03DAh
wvr:
asm {   in      al,dx
        test    al,8
        jz      wvr
}
}




/****************************************************************************\
*
* Function:     void WaitDE(void)
*
* Description:  Waits for Display Enable
*
\****************************************************************************/

void WaitDE(void)
{
asm     mov     dx,03DAh
wde:
asm {   in      al,dx
        test    al,1
        jnz     wde
}
}




/****************************************************************************\
*
* Function:     void InitScreen(void)
*
* Description:  Initializes MIDAS Module Player screen
*
\****************************************************************************/

void InitScreen(void)
{
    frameCnt = 0;
    textmode(C4350);
    clrscr();
    vgaWriteText(1, 1, scrtop);
    vgaWriteText(1, 8, "\xFF\x0FÞ\xFF\x78\x7F\x4EÜ\xFF\x08Ý");
    window(1, 42, 80, 50);
    gotoxy(1, 1);
    textattr(0x07);
}




/****************************************************************************\
*
* Function:     void DrawInstNames(void)
*
* Description:  Draws the instrument names to the screen. Used after a module
*               has been loaded
*
\****************************************************************************/

void DrawInstNames(void)
{
    int         i, x, y;
    int         maxInst;
    int         nameLen, len;
    char        chstr[34];
    char        str[34];
    char        c;

    for ( i = 0; i < numChans; i++ )
    {
        if ( i < 9 )
            c = i + '1';
        else
            c = i + 'A' - 9;
        chstr[i] = c;
    }
    chstr[numChans] = 0;

    if ( instMode == 0 )
    {
        maxInst = 2 * (29 - numChans) + instAdd;
        nameLen = 33 - numChans;

        for ( y = 11+numChans; y < 40; y++ )
            vgaWriteText(40, y, "\xFF\x70³");
    }
    else
    {
        maxInst = 29 - numChans + instAdd;
        nameLen = 72 - numChans;
    }

    for ( i = instAdd; i < maxInst; i++ )
    {
        x = 3;
        y = 11 + numChans + i - instAdd;

        len = nameLen;
        if ( y >= 40 )
        {
            x += 38;
            y -= (29 - numChans);
            len++;
        }

        if ( i < mod->numInsts )
        {
            if ( mod->instsUsed[i] == 1 )
            {
                vgaWriteStr(x, y, &chstr[0], 0x70, numChans+4);
                x += numChans + 1;
                vgaWriteByte(x, y, i+1, 0x70);
                vgaWriteStr(x+3, y, &mod->insts[i].iname[0], 0x70, len);
                if ( instMode == 1 )
                {
                    sprintf(&str[0],"Len:%5u Vol:%2u",
                        mod->insts[i].length, mod->insts[i].volume);
                    vgaWriteStr(x+34, y, &str[0], 0x70, 45-x);
                    if ( mod->insts[i].looping != 0 )
                    {
                        sprintf(&str[0],"Rep:%u-%u",
                            mod->insts[i].loopStart, mod->insts[i].loopEnd);
                        vgaWriteStr(x+51, y, &str[0], 0x70, 28-x);
                    }
                }
            }
            else
            {
                vgaWriteStr(x, y, &chstr[0], 0x78, numChans+4);
                x += numChans + 1;
                vgaWriteByte(x, y, i+1, 0x78);
                vgaWriteStr(x+3, y, &mod->insts[i].iname[0], 0x78, len);
            }
        }
        else
        {
            vgaWriteStr(x, y, "", 0x70, len + numChans + 4);
        }
    }

}




/****************************************************************************\
*
* Function:     void DrawScreen(void)
*
* Description:  Draws MIDAS Module Player screen. Used after module has
*               been loaded.
*
\****************************************************************************/

void DrawScreen(void)
{
    int         i, x, y, len;
    char        c;
    char        str[32];
    char        chstr[18];
    ushort      mode, mixRate;
    int         error;

    vgaWriteText(1, 1, scrtop);
    vgaWriteText(1, 8, "\xFF\x0FÞ\xFF\x78Ú\x7F\x4CÄ\xFF\x7F¿\xFF\x08Ý");
    for ( i = 0; i < numChans; i++ )
        vgaWriteText(1, 9+i, "\xFF\x0FÞ\xFF\x78³\xFF\x70   ³\x7F\x11 ³   ³  ³"
            "\x7F\x0E ³\x7F\x20þ\xFF\x7F³\xFF\x08Ý");
    y = 9+numChans;
    vgaWriteText(1, y, "\xFF\x0FÞ\xFF\x78À\xFF\x7F\x7F\x4CÄÙ\xFF\x08Ý");
    vgaWriteText(1, y+1, "\xFF\x0FÞ\xFF\x78Ú\x7F\x4CÄ\xFF\x7F¿\xFF\x08Ý");
    y+=2;
    for ( ; y < 40; y++ )
        vgaWriteText(1, y, "\xFF\x0FÞ\xFF\x78³\xFF\x70\x7F\x25 ³\x7F\x26 "
            "\xFF\x7F³\xFF\x08Ý");
    vgaWriteText(1, 40, "\xFF\x0FÞ\xFF\x78À\xFF\x7F\x7F\x4CÄÙ\xFF\x08Ý");
    vgaWriteText(1, 41, "\xFF\x0FÞ\xFF\x78\x7F\x4EÜ\xFF\x08Ý");

    sprintf(&str[0], "%s Module", modTypes[mod->IDnum]);
    vgaWriteStr(46, 4, str, 0x70, 33);

    vgaWriteStr(10, 4, &mod->songName[0], 0x70, 31);

    DrawInstNames();

    vgaWriteByte(10, 6, mod->songLength, 0x70);

    if ( (error = SD->GetMixRate(&mixRate)) != OK )
        midasError(error);
    sprintf(&str[0], "%uHz", mixRate);
    vgaWriteStr(15, 5, str, 0x70, 7);

    if ( (error = SD->GetMode(&mode)) != OK )
        midasError(error);

    if ( mode & sd16bit )
        mStrCopy(&str[0], "16-bit ");
    else
        mStrCopy(&str[0], "8-bit ");

    if ( mode & sdStereo )
        mStrAppend(&str[0], "Stereo ");
    else
        mStrAppend(&str[0], "Mono ");

    vgaWriteStr(39, 5, str, 0x70, 26);
}




/****************************************************************************\
*
* Function:     unsigned GetRealVU(int channel)
*
* Description:  Gets real VU-meter value for a channel
*
* Input:        int channel             channel number
*
* Returns:      VU-meter value for channel
*
\****************************************************************************/

unsigned GetRealVU(int channel)
{
    int         error;
    ulong       rate;
    ushort      pos;
    ushort      inst;
    uchar       volume;
    ushort      meter;

    /* Get playing rate for channel: */
    if ( (error = SD->GetRate(channel, &rate)) != OK )
        midasError(error);

    /* Get playing position for channel: */
    if ( (error = SD->GetPosition(channel, &pos)) != OK )
        midasError(error);

    /* Get current instrument handle for channel: */
    if ( (error = SD->GetInstrument(channel, &inst)) != OK )
        midasError(error);

    /* Get current volume for channel: */
    if ( (error = SD->GetVolume(channel, &volume)) != OK )
        midasError(error);

    if ( (rate != 0) && (inst != 0) )
    {
        if ( (error = vuMeter(inst, rate, pos, (volume * masterVol) / 64,
            &meter)) != OK )
            midasError(error);
    }
    else
        meter = 0;

    return meter;
}




/****************************************************************************\
*
* Function:     void UpdScreen(void)
*
* Description:  Updates MIDAS Module Player screen
*
\****************************************************************************/

void UpdScreen(void)
{
    char        str[32];
    int         i, x, y;
    char        *iname;
    mpChanInfo  *chan;
    int         numInsts;
    short       pan;
    ulong       rate;
    int         error;
    ushort      meter, pos;
    time_t      currTime;
    int         hour, min, sec;

    vgaWriteByte(23, 6, info->pos, 0x70);
    vgaWriteByte(35, 6, info->pattern, 0x70);
    vgaWriteByte(43, 6, info->row, 0x70);
    sprintf(&str[0], "%3u", (unsigned) info->BPM);
    vgaWriteStr(53, 6, &str[0], 0x70, 3);
    vgaWriteByte(64, 6, info->speed, 0x70);
    sprintf(&str[0], "%2u", masterVol);
    vgaWriteStr(72, 6, &str[0], 0x70, 3);

    if ( !paused )
    {
        currTime = time(NULL) - startTime - pauseTime;
        hour = currTime / 3600;
        min = ((currTime - 3600*hour) / 60) % 60;
        sec = currTime % 60;
        sprintf(&str[0], "%2i:%02i:%02i", hour, min, sec);
        vgaWriteStr(71, 5, &str[0], 0x70, 8);
    }
    else
        vgaWriteStr(71, 5, "-PAUSED-", 0x70, 8);

    numInsts = mod->numInsts;

    for ( i = 0; i < numChans; i++ )
    {
        chan = &info->chans[i];

        if ( oldInsts[i] != 0 )
        {
            x = 3+i;
            y = 11 + numChans + (oldInsts[i]-1) - instAdd;
            if ( (y >= 40) && (instMode == 0) )
            {
                x += 38;
                y -= (29 - numChans);
            }
            if ( (y < 40) && (oldInsts[i] > instAdd) )
            {
                str[1] = 0;
                if ( i < 9 )
                    str[0] = i + '1';
                else
                    str[0] = (i-9) + 'A';
                vgaWriteStr(x, y, &str[0], 0x70, 1);
            }
        }

        if ( (Channels[i] == 0) && (!muted) && (!paused) )
        {
            if ( (chan->instrument != 0) && (chan->instrument <= numInsts) )
            {
                x = 3+i;
                y = 11 + numChans + (chan->instrument-1) - instAdd;
                if ( (y >= 40) && (instMode == 0) )
                {
                    x += 38;
                    y -= (29 - numChans);
                }
                if ( (y < 40) && (chan->instrument > instAdd) )
                {
                    str[1] = 0;
                    if ( i < 9 )
                        str[0] = i + '1';
                    else
                        str[0] = (i-9) + 'A';
                    vgaWriteStr(x, y, &str[0], 0x7E, 1);
                }

                vgaWriteByte(7, 9+i, chan->instrument, 0x70);

                iname = &mod->insts[chan->instrument-1].iname[0];

                vgaWriteStr(10, 9+i, iname, 0x70, 14);

                oldInsts[i] = chan->instrument;

                if ( realVU )
                    meter = GetRealVU(i);
                else
                    meter = chan->volumebar;

                if ( (chan->note < 254) && ( (chan->note & 15) < 12) )
                {
                    mStrCopy(&str[0], notes[chan->note & 15]);
                    str[2] = (chan->note >> 4) + '0';
                    str[3] = 0;
                    vgaWriteStr(25, 9+i, &str[0], 0x70, 3);
                }
                else
                    vgaWriteStr(25, 9+i, "", 0x70, 3);
            }
            else
                meter = 0;

            vgaWriteByte(29, 9+i, chan->volume, 0x70);
            if ( chan->commandname[0] != 0 )
            {
                sprintf(&str[0], "%s %02X", chan->commandname,
                    (int) chan->infobyte);
                vgaWriteStr(32, 9+i, &str[0], 0x70, 14);
            }
            else
                vgaWriteStr(32, 9+i, "", 0x70, 14);

            mStrCopy(&str[0], "\xFF\x7A\x7F\x01þ\xFF\x70\x7F\x01þ");
            str[3] = meter >> 1;
            str[8] = 32 - (meter >> 1);
            vgaWriteText(47, 9+i, &str[0]);
        }
        else
            vgaWriteText(7, 9+i,
                "\xFF\x70\x7F\x11 ³   ³  ³\x7F\x0E ³\x7F\x20þ");

        if ( (error = SD->GetPanning(i, &pan)) != OK )
            midasError(error);

        switch ( pan )
        {
            case panLeft:
                mStrCopy(&str[0], "LFT");
                break;

            case panRight:
                mStrCopy(&str[0], "RGT");
                break;

            case panMiddle:
                mStrCopy(&str[0], "MID");
                break;

            case panSurround:
                mStrCopy(&str[0], "SUR");
                break;

            default:
                sprintf(&str[0], "%3i", pan);
        }
        if ( i != actch )
            vgaWriteStr(3, i+9, &str[0], 0x70, 3);
        else
            vgaWriteStr(3, i+9, &str[0], 0x07, 3);
    }
}



/****************************************************************************\
*
* Function:     void WaitFrame(void)
*
* Description:  Waits for next frame, either by using VGA hardware or by
*               waiting for the frame counter to change, depending on
*               whether screen synchronization is used or not
*
\****************************************************************************/

void WaitFrame(void)
{
    ulong       oldcnt = frameCnt;

    if ( sync )
        while ( frameCnt == oldcnt );
    else
    {
        WaitDE();
        WaitVR();
        frameCnt++;
    }
}



/****************************************************************************\
*
* Function:     void SetBlank(int blankState)
*
* Description:  Sets screen blanking state
*
* Input:        int blankState          blanking state: 1 = blanked, 0 = not
*
\****************************************************************************/

void SetBlank(int blankState)
{
    outp(0x03C4, 1);
    switch ( blankState )
    {
        case 1:
            scrBlank = 1;
            outp(0x3C5, inp(0x3C5) | 32);
            break;

        case 0:
            scrBlank = 0;
            outp(0x3C5, inp(0x3C5) & (255 - 32));
            break;
    }
}




/****************************************************************************\
*
* Function:     void DOSshell(void)
*
* Description:  Jumps to DOS shell
*
\****************************************************************************/

void DOSshell(void)
{
    char        *comspec;
    char        *dir;
    int         disk, error;

    if ( (!shellSync) && sync )
        tmrStopScrSync();

    /* restore old text mode: */
    if ( !immShell )
    {
        textmode(textInfo.currmode);
        SetBlank(0);
    }

    if ( (error = memAlloc(MAXDIR, (void**) &dir)) != OK )
        Error(errorMsg[error]);

    disk = getdisk();
    getcurdir(0, dir);

    comspec=mGetEnv("COMSPEC");
    spawnl(P_WAIT, comspec, NULL);

    /* save text mode information, including mode: */
    if ( !immShell )
    {
        gettextinfo(&textInfo);
        InitScreen();
        DrawScreen();
    }

    setdisk(disk);
    chdir("\\");
    chdir(dir);

    if ( (error = memFree(dir)) != OK )
        Error(errorMsg[error]);

    if ( (!shellSync) && sync )
        tmrSyncScr(scrSync, &prevr, NULL, NULL);
    CheckHeap();

}




/****************************************************************************\
*
* Function:     void prepare(int fNum)
*
* Description:  Prepares for playing a module file, setting variables
*               isArchive and decompressed as necessary.
*
* Input:        int fNum                number of file name
*
\****************************************************************************/

void prepare(int fNum)
{
    int         i;
    char        ext[_MAX_EXT];

    /* get file name extension: */
    fnsplit(fNames[fNum], NULL, NULL, NULL, &ext[0]);

    isArchive = 0;

    /* Search through known archive extensions. If a match is found, the file
       is an archive. */
    for ( i = 0; i < NUMARCEXTS; i++ )
        if ( stricmp(&ext[0], arcExtensions[i]) == 0 )
            isArchive = 1;

    decompressed = 0;
}




/****************************************************************************\
*
* Function:     void decompress(char *fileName)
*
* Description:  Decompresses a file and sets decompName to decompressed file
*               name. The archive is assumed to contain a single file, with
*               the same name as the archive and any extension.
*
* Input:        char *fileName          pointer to archive file name
*
\****************************************************************************/

void decompress(char *fileName)
{
    int         error;
    char        name[_MAX_FNAME];
    struct ffblk ffb;
    char        *comspec;

    fnsplit(fileName, NULL, NULL, &name[0], NULL);

    if ( decompName == NULL )
    {
        if ( (error = memAlloc(_MAX_PATH, (void**) &decompName)) != OK )
            Error(errorMsg[error]);
    }

    comspec=mGetEnv("COMSPEC");

    if ( spawnlp(P_WAIT, comspec, "/c", "/c", "PKUNZIP", "-o", fileName,
        tempDir, "> NUL", NULL) != 0 )
        Error("Unable to decompress file");

    mStrCopy(decompName, tempDir);
    mStrAppend(decompName, &name[0]);
    mStrAppend(decompName, ".*");

    if ( findfirst(decompName, &ffb, 0) != 0 )
        Error("Unable to find decompressed file");

    mStrCopy(decompName, tempDir);
    mStrAppend(decompName, &ffb.ff_name[0]);
    decompressed = 1;
}



/****************************************************************************\
*
* Function:     void CreateMeter(ushort meter, char *str)
*
* Description:  Creates a VU-meter string for vgaWriteText()
*
* Input:        ushort meter            VU-meter value (0-31)
*               char *str               Pointer to meter string
*
\****************************************************************************/

void CreateMeter(ushort meter, char *str)
{
    ushort      mleft;
    ushort      numb;
    char        ss[10];

    mleft = meter;

    if ( mleft > 0 )
    {
        if ( mleft > 26 )
            numb = 26;
        else
            numb = mleft;
        mleft -= numb;
        mStrCopy(&str[0], "\xFF\x7A\x7F\x01þ");
        str[3] = numb;
    }
    else
    {
        mStrCopy(&str[0], "\xFF\x70\x7F\x1Fþ");
        return;
    }

    if ( mleft > 0 )
    {
        if ( mleft > 4 )
            numb = 4;
        else
            numb = mleft;
        mleft -= numb;
        mStrCopy(&str[5], "\xFF\x7E\x7F\x01þ");
        str[8] = numb;
    }

    if ( mleft > 0 )
    {
        mStrCopy(&str[10], "\xFF\x7C\x7F\x01þ");
        str[13] = mleft;
        mleft = 0;
    }
    else
    {
        mleft = 31-meter;
        mStrCopy(&ss[0], "\xFF\x70\x7F\x01þ");
        ss[3] = mleft;
        mStrAppend(&str[0], &ss[0]);
    }
}



/****************************************************************************\
*
* Function:     void AdjustAmplify(void)
*
* Description:  Adjust sound amplification level and display main
*               VU-meters
*
\****************************************************************************/

void AdjustAmplify(void)
{
    int         y;
    int         error;
    int         done = 0;
    char        key;
    char        str[80];
    ushort      leftMeter, rightMeter;
    unsigned    amplification;
    int         meterMode;
    int         chan, mval;
    short       panning;
    int         leftOver, rightOver;
    ushort      sdMode;
    long        loCount, roCount;
    int         autoLimit = 0;
    int         maxAmplify, maxMeter;
    unsigned    prevAmp;

    vgaWriteText(17, 35, "\xFF\x7FÞ\x7F\x2Cß\xFF\x78Ý");
    vgaWriteText(17, 36, "\xFF\x7FÞ\xFF\x70\x7F\x07 Current Amplification "
        "Level:\x7F\x09 \xFF\x78Ý");
    vgaWriteText(17, 37, "\xFF\x7FÞ\x7F\x2C \xFF\x78Ý");
    vgaWriteText(17, 38, "\xFF\x7FÞ\xFF\x70  Left:\x7F\x25 \xFF\x78Ý");
    vgaWriteText(17, 39, "\xFF\x7FÞ\xFF\x70 Right:\x7F\x25 \xFF\x78Ý");
    vgaWriteText(17, 40, "\xFF\x7FÞ\xFF\x78\x7F\x2CÜÝ");

    if ( (error = SD->GetAmplification(&amplification)) != OK )
        midasError(error);

    prevAmp = amplification;

    if ( (error = SD->GetMode(&sdMode)) != OK )
        midasError(error);

    if ( realVU == 1 )
    {
        meterMode = 1;
    }
    else
    {
        if ( (SD != &GUS) && (SD != &NSND) )
            meterMode = 2;
        else
            meterMode = 0;
    }

    rightOver = leftOver = 0;

    while ( !done )
    {
        WaitFrame();

        /* Set amplification level if changed: */
        if ( amplification != prevAmp )
        {
            if ( (error = SD->SetAmplification(amplification)) != OK )
                midasError(error);
            prevAmp = amplification;
        }

        /* display amplification level: */
        sprintf(&str[0], "%u", amplification);
        vgaWriteStr(54, 36, &str[0], 0x70, 6);

        /* calculate VU-meter values: */
        switch ( meterMode )
        {
            case 0:
                leftMeter = rightMeter = 0;
                break;

            case 1:
                leftMeter = rightMeter = 0;
                for ( chan = 0; chan < numChans; chan++ )
                {
                    mval = GetRealVU(chan);

                    if ( (error = SD->GetPanning(chan, &panning)) != OK )
                        midasError(error);

                    if ( (panning == panMiddle) || (panning == panSurround) ||
                        ((sdMode & sdStereo) == 0) )
                    {
                        leftMeter += mval;
                        rightMeter += mval;
                    }
                    else
                    {
                        if ( panning < 0 )
                        {
                            leftMeter += mval;
                            rightMeter += ((64+panning) * mval) / 64;
                        }
                        else
                        {
                            leftMeter += ((64-panning) * mval) / 64;
                            rightMeter += mval;
                        }
                    }
                }

                if ( leftMeter > rightMeter )
                    maxMeter = leftMeter / numChans;
                else
                    maxMeter = rightMeter / numChans;

                leftMeter = (long) leftMeter * (long) amplification / 64L /
                    (long) numChans;
                rightMeter = (long) rightMeter * (long) amplification / 64L /
                    (long) numChans;
                break;

            case 2:
                if ( (error = dsmGetMainVU(&leftMeter, &rightMeter)) != OK )
                    midasError(error);
                maxMeter = 0;
                break;
        }

        if ( maxMeter > 0 )
        {
            maxAmplify = 64 * 64 / (maxMeter + 1);
        }
        else
            maxAmplify = 32767;


        if ( leftMeter >= 63 )
        {
            leftMeter = 63;
            leftOver = 1;
            loCount = frameCnt;
        }
        if ( rightMeter >= 63 )
        {
            rightMeter = 63;
            rightOver = 1;
            roCount = frameCnt;
        }

        /* draw left channel VU-meter: */
        CreateMeter(leftMeter >> 1, &str[0]);
        vgaWriteText(26, 38, &str[0]);
        if ( leftOver == 1 )
        {
            vgaWriteStr(59, 38, "®¯", 0x7C, 2);
            if ( (frameCnt - loCount) > 25 )
                leftOver = 0;
        }
        else
        {
            vgaWriteStr(59, 38, "®¯", 0x70, 2);
        }

        /* draw right channel VU-meter: */
        CreateMeter(rightMeter >> 1, &str[0]);
        vgaWriteText(26, 39, &str[0]);
        if ( rightOver == 1 )
        {
            vgaWriteStr(59, 39, "®¯", 0x7C, 2);
            if ( (frameCnt - roCount) > 25 )
                rightOver = 0;
        }
        else
        {
            vgaWriteStr(59, 39, "®¯", 0x70, 2);
        }

        if ( (autoLimit == 1) && (amplification > maxAmplify) )
            amplification = maxAmplify;


        if ( kbhit() )
        {
            key = getch();                  /* read keypress */
            switch ( toupper(key) )
            {
                case 13:                    /* Enter */
                case 27:                    /* Escape */
                    done = 1;
                    break;

                case '+':                   /* '+' - increase by 16 */
                    if ( amplification < 64000 )
                        amplification += 16;
                        if ( amplification > 64000 ) amplification = 64000;
                    break;

                case '-':                   /* '-' - decrease by 16 */
                    if ( amplification > 0 )
                        amplification -= 16;
                        if ( amplification > 64000 ) amplification = 0;
                    break;

                case '.':                   /* '.' - double */
                    if ( amplification < 32000 )
                        amplification *= 2;
                    break;

                case ',':                   /* ',' - halve */
                    if ( amplification > 1 )
                        amplification /= 2;
                    break;

                case ' ':                   /* Space - restore to 64 */
                    amplification = 64;
                    break;

                case 'V':                   /* 'V' - change meter mode */
                    if ( meterMode != 0 )
                    {
                        if ( meterMode == 1 )
                        {
                            if ( (SD != &GUS) && (SD != &NSND) )
                                meterMode = 2;
                        }
                        else
                        {
                            if ( realVU == 1 )
                                meterMode = 1;
                        }
                    }

                    switch ( meterMode )
                    {
                        case 0:
                            cprintf("VU meter mode 0 - no meters\r\n");
                            break;

                        case 1:
                            cprintf("VU meter mode 1 - channel meters\r\n");
                            break;

                        case 2:
                            cprintf("VU meter mode 2 - mixer output\r\n");
                            break;


                    }

                    break;

                case 'A':                   /* 'A' - toggle autolimit */
                    autoLimit ^= 1;
                    if ( autoLimit )
                        cprintf("Automatic limits on\r\n");
                    else
                        cprintf("Automatic limits off\r\n");
                    break;
            }
        }
    }

    DrawScreen();
}




/****************************************************************************\
*
* Function:     void HelpText(void)
*
* Description:  Shows the key help text
*
\****************************************************************************/

void HelpText(void)
{
    int         i, j, k;
    char        *huu, *haa;

    vgaWriteText(10, 6, "\xFF\x7FÞ\x7F\x3Cß\xFF\x78Ý");
    for (i = 7; i < 35; i++)
        vgaWriteText(10, i, "\xFF\x7FÞ\x7F\x3C \xFF\x78Ý");

    vgaWriteText(10, 35, "\xFF\x7FÞ\xFF\x78\x7F\x3CÜÝ");

    huu = help;
    i = 7;

    while(*huu != 0)
    {
        for( j = 0; huu[j] != 0x0a; j++ );
        haa = huu;
        huu += j;
        *huu = 0;
        vgaWriteStr(12, i++, haa, 0x70, j);
        *huu = 0x0a;
        huu++;
    }

    getch();
    DrawScreen();
}



/****************************************************************************\
*
* Function:     void HandleKeys(void)
*
* Description:  Handles the keypresses
*
\****************************************************************************/

void HandleKeys(void)
{
    char        key;
    short       panning;
    int         error;
    ushort      temp;

    key = getch();

    if ( !key )
    {
        switch ( getch() )
        {
            case 45:            /* Alt-X */
                exitFlag = 1;
                break;

            case 77:            /* Right arrow */
                MP->SetPosition(info->pos + 1);
                break;

            case 75:            /* Left arrow */
                MP->SetPosition(info->pos - 1);
                break;

            case 72:            /* Up arrow */
                if ( actch > 0 )
                    actch--;
                break;

            case 80:            /* Down arrow */
                if ( actch < (numChans-1) )
                    actch++;
                break;

            case 81:            /* Page down */
                if ( instAdd < (mod->numInsts-1) )
                {
                    instAdd++;
                    DrawInstNames();
                }
                break;

            case 73:            /* Page up */
                if ( instAdd > 0 )
                {
                    instAdd--;
                    DrawInstNames();
                }
                break;

            case 59:            /* F1 */
                HelpText();
                break;

            case 66:            /* F8 */
                SetBlank(scrBlank ^ 1);
                break;
        }
    }
    else
    {
        switch ( toupper(key) )
        {
            case 27:
                fadeOut = 1;
                noNext = 1;
                break;

            case 'S':
                while ( !kbhit() )
                {
                    if ( (error = SD->GetPosition(actch, &temp)) != OK )
                        midasError(error);
                    cprintf("Ch: %X, Pos: %04X\r\n", actch, temp);
                }
                getch();
                break;

            case '+':
                if ( masterVol != 64 )
                {
                    masterVol++;
                    SD->SetMasterVolume(masterVol);
                }
                break;

            case '-':
                if ( masterVol != 0 )
                {
                    masterVol--;
                    SD->SetMasterVolume(masterVol);
                }
                break;

            case 'D':
                DOSshell();
                break;

            case '0': toggleChannel(9); break;
            case '1': toggleChannel(0); break;
            case '2': toggleChannel(1); break;
            case '3': toggleChannel(2); break;
            case '4': toggleChannel(3); break;
            case '5': toggleChannel(4); break;
            case '6': toggleChannel(5); break;
            case '7': toggleChannel(6); break;
            case '8': toggleChannel(7); break;
            case '9': toggleChannel(8); break;

            case 'T': toggleChannel(actch); break;

            case ',':
                if ( (error = SD->GetPanning(actch, &panning)) != OK )
                    midasError(error);
                if ( (panning > -64) && (panning <= 64) )
                    if ( (error = SD->SetPanning(actch, panning-1)) != OK )
                        midasError(error);
                break;

            case '.':
                if ( (error = SD->GetPanning(actch, &panning)) != OK )
                    midasError(error);
                if ( (panning < 64) && (panning >= -64) )
                    if ( (error = SD->SetPanning(actch, panning+1)) != OK )
                        midasError(error);
                break;

            case 'M':
                if ( (error = SD->SetPanning(actch, panMiddle)) != OK )
                    midasError(error);
                break;

            case 'U':
                if ( (error = SD->SetPanning(actch, panSurround)) != OK )
                    midasError(error);
                break;

            case 'L':
                if ( (error = SD->SetPanning(actch, panLeft)) != OK )
                    midasError(error);
                break;

            case 'R':
                if ( (error = SD->SetPanning(actch, panRight)) != OK )
                    midasError(error);
                break;

            case 'F':
                ShowFree();
                break;

            case 'B':
                dumpfree();
                break;

            case 'H':
                HelpText();
                break;

            case 'W':
                dumpheap();
                break;

            case 'N':
                fadeOut = 1;
                break;

            case 'A':
                AdjustAmplify();
                break;

            case 'P':
                paused ^= 1;
                SD->Pause(paused);
                if ( paused == 1 )
                    pauseStart = time(NULL);
                else
                    pauseTime += time(NULL) - pauseStart;
                break;

            case ' ':
                muted ^= 1;
                SD->Mute(muted);
                break;

            case 'V':
                cputs("MIDAS Module Player v" MIDPVERSTR ", compiled on "
                __DATE__ ", " __TIME__ "\r\n");
                cputs("Using MIDAS Sound System v" MVERSTR "\r\n");
                cprintf("%s\r\n", SD->ID);
                #ifdef DEBUG
                cputs("DEBUG mode enabled\r\n");
                #endif
                break;

            case 9:
                instMode ^= 1;
                DrawInstNames();
                break;
        }
    }
}




/****************************************************************************\
*
* Function:     mpModule *PlayModule(char *fileName)
*
* Description:  Detects the module file type and loads and starts playing it
*
* Input:        char *fileName          module file name
*
* Returns:      Pointer to module structure
*
\****************************************************************************/

mpModule *PlayModule(char *fileName)
{
    uchar       *header;
    fileHandle  f;
    mpModule    *module;
    int         error, mpNum, recognized;

    /* allocate memory for module header: */
    if ( (error = memAlloc(MPHDRSIZE, (void**) &header)) != OK )
        midasError(error);

    /* open module file: */
    if ( (error = fileOpen(fileName, fileOpenRead, &f)) != OK )
        midasError(error);

    /* read MPHDRSIZE bytes of module header: */
    if ( (error = fileRead(f, header, MPHDRSIZE)) != OK )
        midasError(error);

    if ( (error = fileClose(f)) != OK )
        midasError(error);

    /* Search through all Module Players to find one that recognizes the
       file header: */
    mpNum = 0; MP = NULL;
    while ( (mpNum < NUMMPLAYERS) && (MP == NULL) )
    {
        if ( (error = modulePlayers[mpNum]->Identify(header, &recognized))
            != OK )
            midasError(error);
        if ( recognized )
            MP = modulePlayers[mpNum];
        mpNum++;
    }

    /* deallocate module header: */
    if ( (error = memFree(header)) != OK )
        midasError(error);

    if ( MP == NULL )
        Error("Unknown module format");

    /* reset amplification level to 64: */
    amplification = 64;

    /* reset active channel number to 0: */
    actch = 0;

    /* reset instrument display offset to 0: */
    instAdd = 0;

    /* load the module file using correct Module Player: */
    if ( realVU )
        module = midasLoadModule(fileName, MP, &vuPrepare);
    else
        module = midasLoadModule(fileName, MP, NULL);

    CheckHeap();

    /* start playing the module: */
    midasPlayModule(module, 0);

    CheckHeap();

    return module;
}




/****************************************************************************\
*
* Function:     void FreeModule(mpModule *module)
*
* Description:  Deallocates a module and possible VU meter information.
*
* Input:        mpModule *module        module to be deallocated
*
\****************************************************************************/

void FreeModule(mpModule *module)
{
    int         i, error, insthdl;

    if ( realVU )
    {
        /* Deallocate VU meter information for all instruments: */
        for ( i = 0; i < module->numInsts; i++ )
        {
            insthdl = module->insts[i].sdInstHandle;
            if ( insthdl != 0 )
            {
                if ( (error = vuRemove(insthdl)) != OK )
                    midasError(error);
            }
        }
    }

    /* Deallocate module: */
    midasFreeModule(module);
}




/****************************************************************************\
*
* Function:     void SetDefaultPanning(void)
*
* Description:  Sets default panning values to channels if necessary
*
\****************************************************************************/

void SetDefaultPanning(void)
{
    int         i, error;

    if ( (useDefPanning) && ((defPanning < 64) || (defPanning == 100))  )
    {
        for ( i = 0; i < numChans; i++ )
        {
            /* Default panning is used. If 100, set channel to surround: */
            if ( defPanning == 100 )
            {
                if ( (error = SD->SetPanning(i, panSurround)) != OK )
                    midasError(error);
            }
            else
            {
                /* If channel is panned to left, set it to -defPanning,
                   otherwise set it to defPanning */
                if ( mod->chanSettings[i] == panLeft )
                {
                    if ( (error = SD->SetPanning(i, -defPanning)) != OK )
                        midasError(error);
                }
                else
                {
                    if ( mod->chanSettings[i] == panRight )
                    {
                        if ( (error = SD->SetPanning(i, defPanning)) != OK )
                            midasError(error);
                    }
                }
            }
        }
    }

}


/****************************************************************************\
*
* Function:     void PlayInteractive(char *fName)
*
* Description:  Plays a module file, updating the display, processing
*               keypresses etc.
*
* Input:        char *fName             module file name
*
\****************************************************************************/

void PlayInteractive(char *fName)
{
    int         stop, i, error, nextf;

    cprintf("Loading \"%s\"\r\n", fName);

    /* load module: */
    mod = PlayModule(fName);
    startTime = time(NULL);

    numChans = mod->numChans;

    SetDefaultPanning();

    /* Prepare screen display: */
    DrawScreen();

    for ( i = 0; i < 32; i++ )
    {
        oldInsts[i] = 0;
        Channels[i] = 0;
    }

    stop = 0;
    fadeOut = 0;
    masterVol = 64;
    if ( (error = SD->SetMasterVolume(64)) != OK )
        midasError(error);

    if ( (loopCnt == 0) && (numFNames != 1) )
        loopCnt = 1;

    if ( isArchive )
    {
        if ( remove(fName) != 0 )
            Error("Unable to delete decompressed file");
        decompressed = 0;
    }

    cprintf ("Playing %d-channel %s Module \"%s\"\r\n", numChans,
        modTypes[mod->IDnum], &mod->songName[0]);
    CheckHeap();

    if ( numFNames != 1 )
    {
        if ( fileNumber < (numFNames-1) )
            nextf = fileNumber + 1;
        else
            nextf = 0;

        prepare(nextf);

        if ( isArchive )
        {
            if ( coreleft() >= DECOMPMEM )
            {
                cprintf("Decompressing \"%s\"\r\n", fNames[nextf]);
                decompress(fNames[nextf]);
            }
            else
                cprintf("Not enough free memory to decompress next module "
                        "file while playing\r\n");
        }
    }

    while ( (!stop) && (!exitFlag) )
    {
        WaitFrame();                    /* wait for next frame */

        /* Read Module Player information: */
        if ( (error = MP->GetInformation(&info)) != OK )
            midasError(error);

        if ( loopCnt != 0 )
        {
            if ( info->loopCnt >= loopCnt )
                fadeOut = 1;
        }

        UpdScreen();                    /* update screen */

        if ( fadeOut )
        {
            if ( masterVol > 0 )
            {
                masterVol--;
                if ( (error = SD->SetMasterVolume(masterVol)) != OK )
                    midasError(error);
            }
            else
            {
                stop = 1;
                if ( (error = SD->SetMasterVolume(0)) != OK )
                    midasError(error);
            }
        }

        if( kbhit() )
            HandleKeys();
    }

    /* stop playing: */
    midasStopModule(mod);
    FreeModule(mod);

    CheckHeap();

    if ( noNext )
        exitFlag = 1;
}





int main(int argc, char *argv[])
{
    int         error, i, n;
    int         cfExists;
    char        *temp;

    deInit();

    /* save text mode information, including mode: */
    gettextinfo(&textInfo);

    puts(title);
    if  ( argc < 2 )
    {
        puts(usage);
        exit(EXIT_SUCCESS);
    }

    free1 = coreleft();
    startTime = time(NULL);

    /* Allocate memory for configuration file name: */
    if ( (error = memAlloc(MAXPATH, (void**) &configFileName)) != OK )
        midasError(error);

    /* Construct configuration file name from the executable file name
       (argv[0]) by replacing the last 3 characters ("EXE") with "CFG". */
    mStrCopy(configFileName, argv[0]);
    i = mStrLength(configFileName);
    mMemCopy(&configFileName[i-3], "CFG", 3);

    midasSetDefaults();
    /* Check if the configuration file exists. If it does, load it, otherwise
       reconfigure: */
    if ( (error = fileExists(configFileName, &cfExists)) != OK )
        midasError(error);
    if ( cfExists )
        midasLoadConfig(configFileName);
    else
        reconfigure = 1;

    /* Parse "MIDAS" environment string: */
    midasParseEnvironment();

    /* Parse "MIDP" environment string: */
    temp = mGetEnv("MIDP");
    if ( temp != NULL )
        ParseOptionString(temp);

    /* Parse command line arguments: */
    ParseCmdLine(argc, argv);

    if ( (numFNames == 0) || showUsage )
    {
        puts(usage);
        exit(EXIT_SUCCESS);
    }

    if ( reconfigure )
    {
        if ( midasConfig() == 0 )
        {
            textmode(textInfo.currmode);
            clrscr();
            exit(EXIT_SUCCESS);
        }
        midasSaveConfig(configFileName);
    }

    if ( scrambleOrder )
    {
        randomize();
        for ( i = 0; i < numFNames; i++ )
        {
            n = random(numFNames);
            temp = fNames[i];
            fNames[i] = fNames[n];
            fNames[n] = temp;
        }
    }

    /* allocate memory for decompression directory name string: */
    if ( (error = memAlloc(MAXPATH, (void*) &tempDir)) != OK )
        Error(errorMsg[error]);

    /* if environment variable "TEMP" is set, use it, otherwise use "C:\" */
    temp = mGetEnv("TEMP");
    if ( temp != NULL )
    {
        /* "TEMP" environment string found. Copy it to tempDir, and if the
           last character is not '\', append one to the end. */
        mStrCopy(tempDir, temp);
        if ( tempDir[mStrLength(tempDir)-1] != '\\' )
            mStrAppend(tempDir, "\\");
    }
    else
    {
        /* No "TEMP" environment string found - use "C:\" */
        mStrCopy(tempDir, "C:\\");
    }


    if ( !immShell )
        InitScreen();
    InitMIDAS();

    if ( immShell )
    {
        prepare(0);

        if ( isArchive && (!decompressed) )
        {
            cprintf("Decompressing \"%s\"\r\n", fNames[0]);
            decompress(fNames[0]);
            mod = PlayModule(decompName);
        }
        else
            mod = PlayModule(fNames[0]);

        CheckHeap();

        /* load module: */
        numChans = mod->numChans;
        SetDefaultPanning();

        if ( isArchive )
        {
            if ( remove(decompName) != 0 )
                Error("Unable to delete decompressed file");
            decompressed = 0;
        }

        DOSshell();

        /* stop playing: */
        midasStopModule(mod);
        FreeModule(mod);
        CheckHeap();

        exitFlag = 1;
    }

    if ( numFNames == 1 )
        noNext = 1;

    fileNumber = 0;
    prepare(fileNumber);

    while( !exitFlag )
    {
        pauseTime = 0;
        paused = 0;
        muted = 0;

        if ( isArchive )
        {
            if ( decompressed )
                PlayInteractive(decompName);
            else
            {
                cprintf("Decompressing \"%s\"\r\n", fNames[fileNumber]);
                decompress(fNames[fileNumber]);
                PlayInteractive(decompName);
            }
        }
        else
            PlayInteractive(fNames[fileNumber]);

        fileNumber++;
        if ( fileNumber >= numFNames )
            fileNumber = 0;
    }

    if ( (decompressed) && (decompName != NULL) )
        if ( remove(decompName) != 0 )
            Error("Unable to delete decompressed file");

    if ( decompName != NULL )
        if ( (error = memFree(decompName)) != OK )
            Error(errorMsg[error]);

    CloseMIDAS();

    if ( (error = memFree(tempDir)) != OK )
        Error(errorMsg[error]);

    if ( (error = memFree(configFileName)) != OK )
        midasError(error);

    /* restore old text mode: */
    if ( !immShell )
    {
        textmode(textInfo.currmode);
        SetBlank(0);
    }

    CheckHeap();

#ifdef DEBUG
    errPrintList();
#endif

    deClose();

    return 0;
}
