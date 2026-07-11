#*      makefile
#*
#* Main make script for MIDAS source code
#*
#* $Id: makefile,v 1.8 1997/07/31 10:56:31 pekangas Exp $
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
BLDOPTSDIR=bldopts
MIDASDIR=.
include $(BLDOPTSDIR)/bldopts.mak


# Default target:
all :		lib


# Show information about the current settings:
showinfo :	$(_IGNORE)
ifdef _DOS
	$(REM) Running in DOS
endif	
ifdef _WIN32
	$(REM) Running in Win32
endif		
ifdef _LINUX
	$(REM) Running in Linux
endif
ifdef _DLL_TARGET
	$(REM) Building a DLL
endif
	$(REM) COMPILER = $(COMPILER)
	$(REM) BUILD = $(BUILD)
	$(REM) TARGET = $(TARGET)
	$(REM) $(CC) $(CCOPTS)
	$(REM) $(ASM) $(ASMOPTS)


# Libraries:
lib dll :
	$(MAKE) -C midas $@

# Install:
install installdll :
	$(MAKE) -C midas $@

# Build/install tools:
tools installtools :
	$(MAKE) -C tools $@

# Build tests:
tests :
	$(MAKE) -C tests

# Cleanup:
clean distclean cvsclean :
	$(CLEANALL)
	$(MAKE) -C midas $@
	$(MAKE) -C bldopts $@
	$(MAKE) -C doc $@
	$(MAKE) -C tools $@
	$(MAKE) -C tests $@
	$(MAKE) -C win32dlg $@

.PHONY : all showinfo lib dll install installdll tools installtools \
         clean distclean cvsclean tests

