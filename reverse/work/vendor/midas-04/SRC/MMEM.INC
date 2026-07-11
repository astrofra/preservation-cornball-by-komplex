;*	MEM.INC
;*
;* MIDAS Sound System memory handling routines
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*


;/***************************************************************************\
;*
;* Function:    int memAlloc(unsigned short len, void **blk);
;*
;* Description: Allocates a block of conventional memory
;*
;* Input:       unsigned short len      Memory block length in bytes
;*              void **blk              Pointer to memory block pointer
;*
;* Returns:     MIDAS error code.
;*              Pointer to allocated block stored in *blk, NULL if error.
;*
;\***************************************************************************/
GLOBAL  LANG memAlloc : far




;/***************************************************************************\
;*
;* Function:     int memFree(void *blk);
;*
;* Description:  Deallocates a memory block allocated with memAlloc()
;*
;* Input:        void *blk               Memory block pointer
;*
;* Returns:      MIDAS error code.
;*
;\***************************************************************************/

GLOBAL	LANG memFree : far




;/***************************************************************************\
;*       enum memFunctIDs
;*       ----------------
;* Description:  ID numbers for memory management functions
;\***************************************************************************/

ENUM    memFunctIDs \
        ID_memAlloc = ID_mem, \
        ID_memFree
