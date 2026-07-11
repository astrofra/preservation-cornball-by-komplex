#*      doscmd.mak
#*
#* Linux-specific command definitions
#*
#* $Id: linuxcmd.mak,v 1.1 1997/02/07 15:16:17 pekangas Exp $
#*
#* Copyright 1997 Housemarque Inc.
#*
#* This file is part of the MIDAS Sound System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Sound System license, LICENSE.TXT. By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*

RM = rm
CP = cp
REM = true
MKDIR = mkdir
RMDIR = rmdir

COPYFILE = $(CP) $^ $@


#* $Log: linuxcmd.mak,v $
#* Revision 1.1  1997/02/07 15:16:17  pekangas
#* Initial revision
#*

