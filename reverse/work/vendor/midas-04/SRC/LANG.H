/*      LANG.H
 *
 * Destination language dependent macros and conditional compilation
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#ifndef __LANG_H
#define __LANG_H


#ifdef __BPREAL__
    #define CALLING pascal
    #define GLOBALVAR pascal
    #define __PASCAL__
    #define __REALMODE__
    #define __16__
#else
#ifdef __BPPROT__
    #define CALLING pascal
    #define GLOBALVAR pascal
    #define __PASCAL__
    #define __PROTMODE__
    #define __16__
    #define NOEMS
#else
#ifdef __ASM16__
    #define CALLING cdecl
    #define GLOBALVAR
    #define __ASM__
    #define __REALMODE__
    #define __16__
#else
    #define CALLING cdecl
    #define GLOBALVAR
    #define __C__
    #define __REALMODE__
    #define __16__
#endif
#endif
#endif


#endif
