;*	MGLOBALS.INC
;*
;* MIDAS Digital Audio System global variables
;*
;* $Id: mglobals.inc,v 1.5 1997/07/31 10:56:47 pekangas Exp $
;*
;* Copyright 1996,1997 Housemarque Inc.
;*
;* This file is part of MIDAS Digital Audio System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Digital Audio System license, "license.txt". By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*


GLOBAL  LANG mUseEMS : _int             ; 1 if EMS should be used
GLOBAL  LANG mEnableSurround : _int     ; 1 if surround sound is enabled
GLOBAL  LANG mBufferLength : _int       ; buffer length in milliseconds
GLOBAL  LANG mBufferBlocks : _int       ; number of buffer blocks
                                        ; (not applicable to all SDs)
GLOBAL  LANG mDefaultFramerate : _int   ; default framerate (in 100*Hz), used
                                        ; when screen sync is not available
GLOBAL  LANG mSyncScreen : _int         ; 1 if timer can be synchronized to
                                        ; screen



;* $Log: mglobals.inc,v $
;* Revision 1.5  1997/07/31 10:56:47  pekangas
;* Renamed from MIDAS Sound System to MIDAS Digital Audio System
;*
;* Revision 1.4  1997/01/16 18:41:59  pekangas
;* Changed copyright messages to Housemarque
;*
;* Revision 1.3  1996/05/31 21:40:50  pekangas
;* Added mSyncScreen and mDefaultFramerate
;*
;* Revision 1.2  1996/05/30 22:38:46  pekangas
;* no changes?
;*
;* Revision 1.1  1996/05/22 20:49:33  pekangas
;* Initial revision
;*