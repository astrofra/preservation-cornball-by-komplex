/*      DIVERR.H
 *
 * MIDAS Sound System DEBUG version divide error handler
 *
 * Copyright 1995 Petteri Kangaslampi and Jarno Paananen
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

#ifndef __DIVERR_H
#define __DIVERR_H



#ifdef __cplusplus
extern "C" {
#endif



/****************************************************************************\
*
* Function:     void deInit(void);
*
* Description:  Initializes divide error handler. MUST be called before
*               MIDAS Sound System is initialized.
*
\****************************************************************************/

void CALLING deInit(void);



/****************************************************************************\
*
* Function:     void deClose(void);
*
* Description:  Removes divide error handler. MUST be called after MIDAS
*               Sound System is uninitialized.
*
\****************************************************************************/

void CALLING deClose(void);



#ifdef __cplusplus
}
#endif



#endif
