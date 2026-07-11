#*      makefile
#*
#* Make script, used just for cleanup
#*
#* $Id: makefile,v 1.1 1997/02/07 15:37:20 pekangas Exp $
#*
#* Copyright 1996,1997 Housemarque Inc.
#*
#* This file is part of the MIDAS Sound System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Sound System license, LICENSE.TXT. By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*

# Include build options:
BLDOPTSDIR=../../src/bldopts
include $(BLDOPTSDIR)/bldopts.mak


# Default target:
all :
	$(REM) Targets available here: clean, distclean, cvsclean

# Cleanup:
clean cvsclean :
	$(CLEANALL)
	-$(RM) *.ps

distclean :
	$(CLEANALL)