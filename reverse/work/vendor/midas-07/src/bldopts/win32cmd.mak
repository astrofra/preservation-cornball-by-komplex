#*      win32cmd.mak
#*
#* Win32-specific command definitions
#*
#* $Id: win32cmd.mak,v 1.2 1997/02/05 22:53:55 pekangas Exp $
#*
#* Copyright 1997 Housemarque Inc.
#*
#* This file is part of the MIDAS Sound System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Sound System license, LICENSE.TXT. By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*

RM = del
CP = copy
REM = rem
MKDIR = md
RMDIR = rd

COPYFILE = $(CP) $(subst /,\\,$^)  $(subst /,\\,$@)


#* $Log: win32cmd.mak,v $
#* Revision 1.2  1997/02/05 22:53:55  pekangas
#* Several small changes to get the new make script system work better
#*
#* Revision 1.1  1997/02/05 17:33:43  pekangas
#* Initial revision
#*