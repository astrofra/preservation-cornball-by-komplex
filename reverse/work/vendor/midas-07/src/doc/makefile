#*      makefile
#*
#* Make script for documentation
#*
#* $Id: makefile,v 1.1 1997/02/07 15:19:34 pekangas Exp $
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
BLDOPTSDIR=../bldopts
include $(BLDOPTSDIR)/bldopts.mak


# Default target
all :
	make -C apiref $@
	make -C prgguide $@

# Cleanup:
clean distclean cvsclean :
	$(CLEANALL)
	make -C common $@
	make -C apiref $@
	make -C prgguide $@

# Installation:
install :
	make -C apiref $@
	make -C prgguide $@














