/*      loadtest.c
 *
 * MIDAS module loading test
 *
 * $Id: loadtest.c,v 1.1.2.1 1997/09/22 14:49:31 pekangas Exp $
 *
 * Copyright 1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <malloc.h>
#include "midasdll.h"
#include "midas.h"
#include "testutil.h"



void MIDASerror(void)
{
    printf("MIDAS error: %s\n", MIDASgetErrorMessage(MIDASgetLastError()));

    MIDASclose();

#ifdef DEBUG
    errPrintList();
#endif

    exit(EXIT_FAILURE);
}



void PrintUsedBlocks(void)
{
#if defined(__WC32__) || defined(__VC32__)
    static struct _heapinfo h_info;
    int         heap_status;
    ulong       used = 0;
    U8          *d;
    unsigned    findCount;

    h_info._pentry = NULL;

    while ( 1 )
    {
        heap_status = _heapwalk(&h_info);
        if ( heap_status != _HEAPOK )
            break;
        if ( h_info._useflag == _USEDENTRY )
        {            
            used += h_info._size;
            printf("USED block at %FP, size %u: ", h_info._pentry,
                   h_info._size);
            d = ((U8*) h_info._pentry) + h_info._size;

            findCount = 32;
            while ( (*((U32*) d) != 0x31337F00) && (findCount > 0) )
            {
                d--;
                findCount--;
            }
            
            if ( *((U32*) d) == 0x31337F00 )
            {
                printf("%s, %u\n", (d - sizeof(U32) - 16),
                       *((U32*) (d - sizeof(U32))));
            }
            else
            {
                printf("?\n");
            }
        }
    }

    printf("Total: %u bytes\n", used);

    if ( (heap_status != _HEAPEND) && (heap_status != _HEAPEMPTY) )
        FatalError("HEAP CORRUPTED!");
#endif
}






int main(int argc, char *argv[])
{
    unsigned num;
    unsigned mem1, mem2, firstUsed, firstLeft;
    MIDASmodule module;

    MIDASstartup();    

    puts("MIDAS module loading test\n");

    if ( argc != 3 )
    {
        puts("Usage: loadtest filename repeats");
        return EXIT_FAILURE;
    }

    if ( !MIDASinit() )
        MIDASerror();

    mem1 = MemUsed();
    if ( (module = MIDASloadModule(argv[1])) == NULL )
        MIDASerror();
    firstUsed = MemUsed() - mem1;
    printf("First loaded: memory used %u bytes\n", firstUsed);
    if ( !MIDASfreeModule(module) )
        MIDASerror();
    firstLeft = MemUsed() - mem1;
    printf("First left after free: %u bytes\n", firstLeft);
    

    num = atoi(argv[2]);
    while ( num )
    {
        printf("%i\n", num);
        mem1 = MemUsed();
        if ( (module = MIDASloadModule(argv[1])) == NULL )
            MIDASerror();
        mem2 = MemUsed();
        if ( (mem2 - mem1) > firstUsed )
        {
            printf("Error: More memory used than first time: %u bytes\n",
                   mem2-mem1);
        }
        
        if ( !MIDASfreeModule(module) )
            MIDASerror();
        mem2 = MemUsed();
        if ( mem2 != mem1 )
        {
            printf("Error: Memory left used after deallocation: %u bytes\n",
                   mem2-mem1);
            PrintUsedBlocks();
        }
        
        num--;
    }

    if ( !MIDASclose() )
        MIDASerror();

    return 0;
}


/*
 * $Log: loadtest.c,v $
 * Revision 1.1.2.1  1997/09/22 14:49:31  pekangas
 * Initial revision
 *
*/
