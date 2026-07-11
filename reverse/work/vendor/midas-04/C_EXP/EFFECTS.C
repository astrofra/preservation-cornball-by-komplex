/*      EFFECTS.C
 *
 * Example on how to play simultaneous music and sound effects
 * using MIDAS Sound System
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
#include <conio.h>
#ifdef __BORLANDC__
#include <alloc.h>
#endif
#include "midas.h"


/* number of sound effect channels: */
#define FXCHANNELS 2

/* maximum number of channels in music: */
#define MAXMUSCHANNELS 8

/* sound effect playing rate: */
#define FXRATE 11025



char            *usage =
"Usage:\tEFFECTS\t<module> <effect #1> <effect #2> <looping effect #3>";

unsigned        fxChannel = 0;
ModulePlayer    *MP;

    /* pointers to all Module Players: */
ModulePlayer    *modulePlayers[NUMMPLAYERS] =
    { &mpS3M,
      &mpMOD,
      &mpMTM };

    /* module type strings: */
char            *moduleTypeStr[NUMMPLAYERS] = {
    { "Scream Tracker ]I[" },
    { "Protracker" },
    { "Multitracker"} };




/****************************************************************************\
*
* Function:     void Error(char *msg)
*
* Description:  Prints an error message, uninitializes MIDAS and exits to DOS
*
* Input:        char *msg               error message
*
\****************************************************************************/

void Error(char *msg)
{
    printf("Error: %s\n", msg);
    midasClose();
    exit(EXIT_FAILURE);
}




/****************************************************************************\
*
* Function:     unsigned LoadEffect(char *fileName, int looping)
*
* Description:  Loads a raw effect sample that can be used with PlayEffect().
*
* Input:        char *fileName          name of sample file
*               int looping             1 if the sample is looping, 0 if not
*
* Returns:      Instrument handle that can be used with PlayEffect() and
*               FreeEffect().
*
\****************************************************************************/

unsigned LoadEffect(char *fileName, int looping)
{
    ushort      instHandle;             /* sound device instrument handle */
    int         error;
    fileHandle  f;
    long        smpLength;              /* sample length */
    uchar       *smpBuf;                /* sample loading buffer */

    /* open sound effect file: */
    if ( (error = fileOpen(fileName, fileOpenRead, &f)) != OK )
        midasError(error);

    /* get file length: */
    if ( (error = fileGetSize(f, &smpLength)) != OK )
        midasError(error);

    /* check that sample length is not too long: */
    if ( smpLength > SMPMAX )
        midasError(errInvalidInst);

    /* allocate memory for sample loading buffer: */
    if ( (error = memAlloc(smpLength, (void**) &smpBuf)) != OK )
        midasError(error);

    /* load sample: */
    if ( (error = fileRead(f, smpBuf, smpLength)) != OK )
        midasError(error);

    /* close sample file: */
    if ( (error = fileClose(f)) != OK )
        midasError(error);

    /* Add sample to Sound Device list and get instrument handle to
       instHandle: */
    if ( looping )
    {
        error = midasSD->AddInstrument(smpBuf, smp8bit, smpLength, 0,
            smpLength, 64, 1, 1, &instHandle);
    }
    else
    {
        error = midasSD->AddInstrument(smpBuf, smp8bit, smpLength, 0,
            0, 64, 0, 1, &instHandle);
    }

    if ( error != OK )
        midasError(error);

    /* deallocate sample allocation buffer: */
    if ( (error = memFree(smpBuf)) != OK )
        midasError(error);

    /* return instrument handle: */
    return instHandle;
}




/****************************************************************************\
*
* Function:     void FreeEffect(unsigned instHandle)
*
* Description:  Deallocates a sound effect
*
* Input:        unsigned instHandle     effect instrument handle returned by
*                                       LoadEffect()
*
\****************************************************************************/

void FreeEffect(unsigned instHandle)
{
    int         error;

    /* remove instrument from Sound Device list: */
    if ( (error = midasSD->RemInstrument(instHandle)) != OK )
        midasError(error);
}




/****************************************************************************\
*
* Function:     void PlayEffect(ushort instHandle, ulong rate, ushort volume,
*                   short panning)
*
* Description:  Plays a sound effect
*
* Input:        ushort instHandle       effect instrument handle, returned by
*                                           LoadEffect().
*               ulong rate              effect sampling rate, in Hz
*               ushort volume           effect playing volume, 0-64
*               short panning           effect panning (see enum sdPanning in
*                                           SDEVICE.H)
*
\****************************************************************************/

void PlayEffect(ushort instHandle, ulong rate, ushort volume,
    short panning)
{
    int         error;

    /* set effect instrument to current effect channel: */
    if ( (error = midasSD->SetInstrument(fxChannel, instHandle)) != OK )
        midasError(error);

    /* set effect volume: */
    if ( (error = midasSD->SetVolume(fxChannel, volume)) != OK )
        midasError(error);

    /* set effect panning: */
    if ( (error = midasSD->SetPanning(fxChannel, panning)) != OK )
        midasError(error);

    /* start playing effect: */
    if ( (error = midasSD->PlaySound(fxChannel, rate)) != OK )
        midasError(error);

    fxChannel++;                        /* channel for next effect */
    if ( fxChannel >= FXCHANNELS )
        fxChannel = 0;
}




/****************************************************************************\
*
* Function:     mpModule *NewModule(char *fileName)
*
* Description:  Detects the type of a module and starts playing it
*
* Input:        char *fileName          module file name
*
\****************************************************************************/

mpModule *NewModule(char *fileName)
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

    /* load the module file using correct Module Player: */
    module = midasLoadModule(fileName, MP, NULL);

    /* check that the module does not have too many channels: */
    if ( module->numChans > MAXMUSCHANNELS )
        Error("Too many channels in module");

    /* start playing the module: */
    midasPlayModule(module, 0);

    return module;
}




/****************************************************************************\
*
* Function:     void StopModule(mpModule *module)
*
* Description:  Stops playing a module and deallocates it
*
* Input:        mpModule *module        pointer to module structure
*
\****************************************************************************/

void StopModule(mpModule *module)
{
    midasStopModule(module);
    midasFreeModule(module);
}




int main(int argc, char *argv[])
{
    mpModule    *mod;                   /* pointer to current module struct */
    unsigned    effect1, effect2;       /* sound effect instrument handles */
    int         effect3;
    int         quit = 0;
    int         error, i, isConfig;
    unsigned    masterVolume = 64;      /* music master volume */
    unsigned    defAmplify;             /* default amplification */
    unsigned    amplification;          /* current, amplification */
    char        fileName[80];
#ifdef __BORLANDC__
    ulong       free1 = coreleft();
#endif

    /* argv[0] is the program name, argv[1] the module filename, argv[2]
       and argv[3] are the effect file names. */

    /* Check that there are exactly four arguments: */
    if  ( argc != 5 )
    {
        puts(usage);
        exit(EXIT_SUCCESS);
    }

    /* Check that the configuration file exists: */
    if ( (error = fileExists("MIDAS.CFG", &isConfig)) != OK )
        midasError(error);
    if ( !isConfig )
    {
        puts("Configuration file not found - run MSETUP.EXE");
        exit(EXIT_FAILURE);
    }

    midasSetDefaults();                 /* set MIDAS defaults */
    midasLoadConfig("MIDAS.CFG");       /* load configuration */

    midasInit();                        /* initialize MIDAS Sound System */

    /* Open channels for music and sound effects. The first FXCHANNELS
       channels will always be free for playing effects: */
    midasOpenChannels(FXCHANNELS + MAXMUSCHANNELS);

    /* Get Sound Device default amplification value: */
    if ( (error = midasSD->GetAmplification(&defAmplify)) != OK )
        midasError(error);
    amplification = defAmplify;

    printf("defAmplify = %i\n", defAmplify);

    /* Load module and start playing: */
    mod = NewModule(argv[1]);

    /* Load sound effect samples and store the instrument handles to the
       table effects[]: */
    effect1 = LoadEffect(argv[2], 0);
    effect2 = LoadEffect(argv[3], 0);
    effect3 = LoadEffect(argv[4], 1);

    puts("Keys:\n"
         "        1,2,3   Play effect\n"
         "        Enter   New module\n"
         "        +,-     Adjust music volume\n"
         "        Esc     Exit");

#ifdef __BORLANDC__
    printf("Memory used: %lu bytes\n", free1 - coreleft());
#endif

    while ( !quit )
    {
        switch ( getch() )
        {
            case 27:    /* Escape - quit */
                quit = 1;
                break;

            case 13:    /* Enter - new module */
                puts("Enter new module file name:");
                gets(&fileName[0]);
                StopModule(mod);
                mod = NewModule(&fileName[0]);
#ifdef __BORLANDC__
                printf("Memory used: %lu bytes\n", free1 - coreleft());
#endif
                break;

            case '1':   /* '1' - play first effect */
                PlayEffect(effect1, FXRATE, 64, -40);
                break;

            case '2':   /* '2' - play second effect */
                PlayEffect(effect2, FXRATE, 64, 40);
                break;

            case '3':   /* '3' - play third effect */
                PlayEffect(effect3, FXRATE, 64, panMiddle);
                break;

            case '+':   /* '+' - increase music volume */
                if ( masterVolume < 64 )
                {
                    masterVolume += 4;
                    if ( (error = midasMP->SetMasterVolume(masterVolume))
                        != OK )
                        midasError(error);
                }

                /* Calculate the amplification value that corresponds to the
                   current decrease in volume (in respect to the Sound Device
                   default amplification value): */
                amplification = defAmplify * 64L*(MAXMUSCHANNELS+FXCHANNELS) /
                    (MAXMUSCHANNELS * masterVolume + FXCHANNELS * 64);
                if ( (error = midasSD->SetAmplification(amplification)) != OK)
                    midasError(error);

                printf("Music volume: %02i, amplification %02i\n",
                    masterVolume, amplification);
                break;

            case '-':
                if ( masterVolume > 8 )
                {
                    masterVolume -= 4;
                    if ( (error = midasMP->SetMasterVolume(masterVolume))
                        != OK )
                        midasError(error);
                }

                /* Calculate the amplification value that corresponds to the
                   current decrease in volume (in respect to the Sound Device
                   default amplification value): */
                amplification = defAmplify * 64L*(MAXMUSCHANNELS+FXCHANNELS) /
                    (MAXMUSCHANNELS * masterVolume + FXCHANNELS * 64);
                if ( (error = midasSD->SetAmplification(amplification)) != OK)
                    midasError(error);

                printf("Music volume: %02i, amplification %02i\n",
                    masterVolume, amplification);
                break;
        }
    }
    StopModule(mod);
    FreeEffect(effect1);                /* deallocate effect #1 */
    FreeEffect(effect2);                /* deallocate effect #2 */
    FreeEffect(effect3);                /* deallocate effect #3 */
    midasClose();                       /* uninitialize MIDAS */

#ifdef __BORLANDC__
    printf("Memory used: %lu bytes\n", free1 - coreleft());
#endif

    return 0;
}
