#*      makefile
#*
#* Make script for documentation
#*
#* $Id: makefile,v 1.5 1997/07/31 10:56:34 pekangas Exp $
#*
#* Copyright 1996,1997 Housemarque Inc.
#*
#* This file is part of MIDAS Digital Audio System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Digital Audio System license, "license.txt". By continuing to use,
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


# Build WinHelp:
winhelp : midas.hlp

midas.hlp midas.cnt : prgguide/prgguide.m4 apiref/apiref.m4
	python ../tools/docconv.py w midas prgguide/prgguide.m4 apiref/apiref.m4
	hcw /c midas.hpj

winhelp-install : ../../doc/winhelp/midas.hlp ../../doc/winhelp/midas.cnt

../../doc/winhelp/midas.hlp : midas.hlp
	$(COPYFILE)

../../doc/winhelp/midas.cnt : midas.cnt
	$(COPYFILE)


# Cleanup:
clean distclean cvsclean :
	$(CLEANALL)
ifdef _WIN32
	@attrib -h midas.gid
	@del midas.*
endif
	make -C common $@
	make -C apiref $@
	make -C prgguide $@

# Installation:
install :
	make -C apiref $@
	make -C prgguide $@














