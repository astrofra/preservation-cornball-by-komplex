#*      makefile
#*
#* MIDAS main make script, used just for cleanup
#*
#* $Id: makefile,v 1.3 1997/02/27 11:57:28 pekangas Exp $
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
BLDOPTSDIR=src/bldopts
include $(BLDOPTSDIR)/bldopts.mak


# Default target:
all :
	$(REM) Targets available here: clean, distclean, cvsclean

# Cleanup:
clean distclean cvsclean :
	$(CLEANALL)
	$(MAKE) -C doc $@
	$(MAKE) -C include $@
	$(MAKE) -C lib $@
	$(MAKE) -C samples $@
	$(MAKE) -C src $@
	$(MAKE) -C tools $@

.PHONY : clean distclean cvsclean

