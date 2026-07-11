/*      MM.C
 *
 * Routines for .MM module playing using the simplified MIDAS API
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include "midas.h"


/* Macro to normalize a 16-bit segment:offset far pointer: */
#ifdef __REALMODE__
#define NORMALIZE(x) ((void*) ((((ulong) x) & 0x000FL) + \
    ((((ulong) x) & 0xFFF0L) << 12) + (((ulong) x) & 0xFFFF0000L)))
#else
#define NORMALIZE(x) (x)
#endif



/****************************************************************************\
*
* Function:     mpModule midasPrepareMM(uchar *mm, ModulePlayer *MP);
*
* Description:  Prepares a .MM module for playing using Module Player *MP.
*
* Input:        uchar *mm               Pointer to the .MM module
*               ModulePlayer *MP        Pointer to the Module Player which
*                                       will be used for playing the module
*
* Returns:      Pointer to the prepared module structure, which can be
*               played with midasPlayModule().
*
* Note:         This function should be used with the simple MIDAS
*               programming interface, MIDAS.C.
*               This function modifies the start of the .MM module in
*               memory, and may thus be called ONLY ONCE for each module.
*
\****************************************************************************/

mpModule * CALLING midasPrepareMM(uchar *mm, ModulePlayer *MP)
{
    mpModule    *module;
    int         error;
    int         i;
    uchar       *mmptr;
    unsigned    a;
    uchar       b;
    mpInstrument *inst;
    uchar       *sample;

    /* Point module to module header: */
    module = (mpModule*) mm;
    mmptr = mm + sizeof(mpModule);

    /* Point module->MP to correct Module Player: */
    module->MP = MP;

    /* Point module->orders to song data: */
    module->orders = (uchar*) mmptr;
    mmptr += module->songLength;

    /* Point module->insts to instrument headers: */
    module->insts = (mpInstrument*) mmptr;
    mmptr += module->numInsts * sizeof(mpInstrument);

    /* Allocate memory for pattern pointers: */
    if ( (error = memAlloc(module->numPatts * sizeof(mpPattern*),
        (void**) &module->patterns)) != OK )
        midasError(error);

    /* Point all patterns to correct pattern data: */
    for ( i = 0; i < module->numPatts; i++ )
    {
        module->patterns[i] = (mpPattern*) mmptr;
        mmptr += module->patterns[i]->length + sizeof(mpPattern);
        mmptr = NORMALIZE(mmptr);
    }

    /* Point module->instsUsed to instrument used data: */
    module->instsUsed = (uchar*) mmptr;
    mmptr += module->numInsts;

    /* Point all samples to correct sample data, convert samples to raw
       format and add instruments to Sound Device: */
    for ( i = 0; i < module->numInsts; i++ )
    {
        mmptr = NORMALIZE(mmptr);

        inst = &module->insts[i];
        if ( (module->instsUsed[i] == 1) && (inst->length != 0) )
        {
            sample = inst->sample = (uchar*) mmptr;
            mmptr += inst->length;

            b = 0;
            for ( a = 0; a < inst->length; a++ )
            {
                b = b + sample[a];
                sample[a] = b;
            }

            error = midasSD->AddInstrument(sample, smp8bit, inst->length,
                inst->loopStart, inst->loopEnd, inst->volume, inst->looping,
                0, &inst->sdInstHandle);
            if ( error != OK )
                midasError(error);
        }
        else
            inst->sdInstHandle = 0;
    }
    return module;
}




/****************************************************************************\
*
* Function:     void midasFreeMM(mpModule *module);
*
* Description:  Deallocates a module that has been prepared for playing with
*               midasPrepareMM(). Deallocates allocated structures and
*               removes the instruments from the Sound Device. Note that
*               the .MM module in memory will NOT be deallocated.
*
* Input:        mpModule *module        module to be deallocated.
*
\****************************************************************************/

void CALLING midasFreeMM(mpModule *module)
{
    int         error, i;

    /* Deallocate pattern pointers: */
    if ( (error = memFree(module->patterns)) != OK )
        midasError(error);

    /* Remove instruments from Sound Device: */
    for ( i = 0; i < module->numInsts; i++ )
    {
        if ( module->insts[i].sdInstHandle != 0 )
        {
            if ( (error = midasSD->RemInstrument(
                module->insts[i].sdInstHandle)) != OK )
                midasError(error);
        }
    }
}
