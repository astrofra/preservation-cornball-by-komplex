/*      DPMI.H
 *
 * DPMI functions for protected mode MIDAS
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/


#ifndef __DPMI_H
#define __DPMI_H


/* new errors:
        errDPMIFailure                  DPMI failure
        errInvalidDescriptor            invalid segment descriptor
 */



/****************************************************************************\
*       struct dpmiRealCallRegs
*       -----------------------
* Description:  Real mode calling register value structure
\****************************************************************************/

typedef struct
{
    ulong       rEDI;
    ulong       rESI;
    ulong       rEBP;
    ulong       reserved;
    ulong       rEBX;
    ulong       rEDX;
    ulong       rECX;
    ulong       rEAX;
    ushort      flags;
    ushort      rES;
    ushort      rDS;
    ushort      rFS;
    ushort      rGS;
    ushort      rIP;
    ushort      rCS;
    ushort      rSP;                    /* if SS and SP are zero, DPMI will */
    ushort      rSS;                    /* provide a small (30 words) stack */
} dpmiRealCallRegs;



#ifdef __cplusplus
extern "C" {
#endif



/****************************************************************************\
*
* Function:     int dpmiAllocDescriptor(ushort *descriptor);
*
* Description:  Allocate LDT descriptor. Use dpmiFreeDescriptor to deallocate.
*
* Input:        ushort *descriptor      pointer to descriptor number
*
* Returns:      MIDAS error code. Descriptor number is written to *descriptor.
*
\****************************************************************************/

int CALLING dpmiAllocDescriptor(ushort *descriptor);




/****************************************************************************\
*
* Function:     int dpmiFreeDescriptor(ushort descriptor);
*
* Description:  Deallocates an LDT descriptor.
*
* Input:        ushort descriptor       descriptor to deallocate
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dpmiFreeDescriptor(ushort descriptor);




/****************************************************************************\
*
* Function:     int dpmiSegmentDescriptor(ushort segment, ushort *descriptor);
*
* Description:  Creates a descriptor to a real mode segment. Subsequent
*               calls will return the same descriptor. Note that the
*               DESCRIPTOR CANNOT BE DEALLOCATED.
*
* Input:        ushort segment          real mode segment
*               ushort *descriptor      pointer to descriptor number
*
* Returns:      MIDAS error code. Descriptor number is written to *descriptor.
*
\****************************************************************************/

int CALLING dpmiSegmentDescriptor(ushort segment, ushort *descriptor);




/****************************************************************************\
*
* Function:     int dpmiSetSegmentBase(ushort selector, ulong baseAddr);
*
* Description:  Changes the 32-bit linear base address of a selector.
*
* Input:        ushort selector         selector number
*               ulong baseAddr          32-bit linear base address for
*                                       selector
*
* Returns:      MIDAS error code.
*
\****************************************************************************/

int CALLING dpmiSetSegmentBase(ushort selector, ulong baseAddr);




/****************************************************************************\
*
* Function:     int dpmiGetSegmentBase(ushort selector, ulong *baseAddr);
*
* Description:  Reads the 32-bit linear base address of a selector.
*
* Input:        ushort selector         selector number
*               ulong *baseAddr         pointer to the 32-bit linear base
*                                       address of the selector
*
* Returns:      MIDAS error code. Selector base address is written to
*               *baseAddr.
*
\****************************************************************************/

int CALLING dpmiGetSegmentBase(ushort selector, ulong *baseAddr);




/****************************************************************************\
*
* Function:     int dpmiSetSegmentLimit(ushort selector, ulong limit);
*
* Description:  Changes the limit of a segment selector.
*
* Input:        ushort selector         selector number
*               ulong limit             32-bit segment limit
*
* Returns:      MIDAS error code.
*
\****************************************************************************/

int CALLING dpmiSetSegmentLimit(ushort selector, ulong limit);




/****************************************************************************\
*
* Function:     int dpmiSetSegmentAccessRights(ushort selector,
*                   ushort accessRights);
*
* Description:  Changes the access rights of a selector
*
* Input:        ushort selector         selector
*               ushort accessRights     new access rights for the segment
*
* Returns:      MIDAS error code.
*
\****************************************************************************/

int CALLING dpmiSetSegmentAccessRights(ushort selector, ushort accessRights);




/****************************************************************************\
*
* Function:     int dpmiCreateCodeAlias(ushort codeSelector,
*                   ushort *selector);
*
* Description:  Creates a data descriptor that has the same base and limit
*               as a code segment descriptor. Use dpmiFreeDescriptor() to
*               deallocate data descriptor.
*
* Input:        ushort codeSelector     code segment selector
*               ushort *selector        pointer to data segment selector
*
* Returns:      MIDAS error code. New data selector is written to *selector.
*
\****************************************************************************/

int CALLING dpmiCreateCodeAlias(ushort codeSelector, ushort *selector);




/****************************************************************************\
*
* Function:     int dpmiAllocDOSMem(ushort numParagraphs, ushort *segment,
*                   ushort *selector);
*
* Description:  Allocates memory from DOS free memory pool, below 1MB. Use
*               dpmiFreeDOSMem() to deallocate.
*
* Input:        ushort numParagraphs    number of paragraphs to allocate
*               ushort *segment         pointer to real mode segment
*               ushort *selector        pointer to selector
*
* Returns:      MIDAS error code. Real mode segment of allocated block is
*               written to *segment. Protected mode selector for block is
*               written to *selector.
*
\****************************************************************************/

int CALLING dpmiAllocDOSMem(ushort numParagraphs, ushort *segment,
    ushort *selector);




/****************************************************************************\
*
* Function:     dpmiFreeDOSMem(ushort selector);
*
* Description:  Deallocates memory allocated with dpmiAllocDOSMem().
*
* Input:        ushort selector     selector for allocated block
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dpmiFreeDOSMem(ushort selector);




/****************************************************************************\
*
* Function:     int dpmiGetRealIntVect(uchar intNum, ushort *segment,
*                   ushort *offset);
*
* Description:  Gets a real mode interrupt vector
*
* Input:        uchar intNum            interrupt number
*               ushort *segment         pointer to interrupt vector segment
*               ushort *offset          pointer to interrupt vector offset
*
* Returns:      MIDAS error code. Interrupt vector is written to
*               *segment and *offset.
*
\****************************************************************************/

int CALLING dpmiGetRealIntVect(uchar intNum, ushort *segment, ushort *offset);



/****************************************************************************\
*
* Function:     int dpmiSetRealIntVect(uchar intNum, ushort segment,
*                   ushort offset);
*
* Description:  Sets a real mode interrupt vector
*
* Input:        uchar intNum            interrupt number
*               ushort segment          interrupt vector segment
*               ushort offset           interrupt vector offset
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dpmiSetRealIntVect(uchar intNum, ushort segment, ushort offset);





/****************************************************************************\
*
* Function:     int dpmiGetIntVect(uchar intNum, void **intVector);
*
* Description:  Gets a protected mode interrupt vector.
*
* Input:        uchar intNum            interrupt number
*               void **intVector        pointer to interrupt vector pointer
*
* Returns:      MIDAS error code. Interrupt vector is written to *intVector.
*
\****************************************************************************/

int CALLING dpmiGetIntVect(uchar intNum, void **intVector);




/****************************************************************************\
*
* Function:     int dpmiSetIntVect(uchar intNum, void *intVector);
*
* Description:  Sets a protected mode interrupt vector.
*
* Input:        uchar intNum            interrupt number
*               void *intVector         interrupt vector
*
* Returns:      MIDAS error code
*
\****************************************************************************/

int CALLING dpmiSetIntVect(uchar intNum, void *intVector);




/****************************************************************************\
*
* Function:     int dpmiRealModeInt(uchar intNum,
*                   dpmiRealCallRegs *registers);
*
* Description:  Simulates a real mode interrupt using DPMI service 0x0300.
*               *register MUST contain appropriate register values for
*               interrupt (CS:IP is ignored).
*
* Input:        uchar intNum                    interrupt number
*               dpmiRealCallRegs *registers     DPMI real mode calling struct
*
* Returns:      MIDAS error code. Register values returned by the interrupt
*               are written to *registers.
*
\****************************************************************************/

int CALLING dpmiRealModeInt(uchar intNum, dpmiRealCallRegs *registers);



#ifdef __cplusplus
}
#endif



#endif
