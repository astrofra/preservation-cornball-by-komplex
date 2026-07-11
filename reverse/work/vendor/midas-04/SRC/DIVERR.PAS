{*      DIVERR.PAS
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
*}


unit diverr;


interface




{****************************************************************************\
*
* Function:     deInit;
*
* Description:  Initializes divide error handler. MUST be called before
*               MIDAS Sound System is initialized.
*
\****************************************************************************}

procedure deInit;




{****************************************************************************\
*
* Function:     deClose;
*
* Description:  Removes divide error handler. MUST be called after MIDAS
*               Sound System is uninitialized.
*
\****************************************************************************}

procedure deClose;



implementation


uses vgaText, timer;


procedure deInit; external;
procedure deClose; external;
{$L DIVERR.OBJ}


END.
