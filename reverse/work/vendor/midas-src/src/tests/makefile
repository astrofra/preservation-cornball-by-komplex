#*      makefile
#*
#* Make script for building MIDAS Digital Audio System tests
#*
#* $Id: makefile,v 1.11.2.1 1997/09/22 14:49:44 pekangas Exp $
#*
#* Copyright 1996,1997 Housemarque Inc.
#*
#* This file is part of MIDAS Digital Audio System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Digital Audio System license, "license.txt". By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*

# Define NODSOUND to disable DirectSound support from Win32 targets
#NODSOUND = 1

# Get the MIDAS library to use: (the tools need to be statically linked as
# they use MIDAS below the API level)
LIBPATH = ../../lib/$(_TARGET)/$(_COMP)$(_BUILD)
LIBS = midas
LINUXLIBS = pthread

# Get Allegro for DJGPP version:
DOSGCCLIBPATH = e:/djgpp/allegro
DOSGCCLIBS = alleg

# Get includes from the MIDAS source directory:
INCPATH = ../midas

WIN32LIBS = user32.lib winmm.lib advapi32.lib

# Include build options:
BLDOPTSDIR=../bldopts
MIDASDIR=../..
include $(BLDOPTSDIR)/bldopts.mak

# Install destination directory:
INSTALLDIR = $(MIDASDIR)/tools/$(_TARGET)

# Default target:
ifdef _WIN32_TARGET

all :		thread$(_EXE) musfade$(_EXE) wave$(_EXE) mix$(_EXE) \
		echotest$(_EXE) proftest$(_EXE) conftest$(_EXE) \
		effects$(_EXE) loadtest$(_EXE)

else

ifdef _DOS_TARGET
all :		wave$(_EXE) mix$(_EXE) echotest$(_EXE) proftest$(_EXE) \
		conftest$(_EXE) loadtest$(_EXE)
else

all :		wave.$(_EXE) mix$(_EXE) loadtest$(_EXE)

endif

endif

# Get build pattern rules:
include $(BLDOPTSDIR)/bldrules.mak


# Thread test:
thread$(_EXE) : thread.$(O)
	$(BUILDEXE)

thread.$(O) : thread.c



# Music channel fade test:
musfade$(_EXE) : musfade.$(O)
	$(BUILDEXE)

musfade.$(O) : musfade.c



# WAVE reader test:
wave$(_EXE) : wave.$(O)
	$(BUILDEXE)

wave.$(O) : wave.c



# Mixing test:
mix$(_EXE) : mix.$(O) testutil.$(O)
	$(BUILDEXE)

mix.$(O) : mix.c testutil.h


# Echo test:
echotest$(_EXE) : echotest.$(O)
	$(BUILDEXE)

echotest.$(O) : echotest.c


# Profiling test:
proftest$(_EXE) : proftest.$(O)
	$(BUILDEXE)

proftest.$(O) : proftest.c 


# Config test:
conftest$(_EXE) : conftest.$(O)
	$(BUILDEXE)

conftest.$(O) : conftest.c


# Sound effects test:
effects$(_EXE) : effects.$(O) testutil.$(O)
	$(BUILDEXE)

effects.$(O) : effects.c


# Module loading test:
loadtest$(_EXE) : loadtest.$(O) testutil.$(O)
	$(BUILDEXE)

loadtest.$(O) : loadtest.c


# Common utility functions:
testutil.$(O) : testutil.c testutil.h


# Cleanup

clean distclean cvsclean :
	$(CLEANALL)


.PHONY : all install clean distclean cvsclean


#* $Log: makefile,v $
#* Revision 1.11.2.1  1997/09/22 14:49:44  pekangas
#* Added module loading test
#*
#* Revision 1.11  1997/08/18 13:55:55  pekangas
#* Changed Allegro directory
#*
#* Revision 1.10  1997/07/31 10:56:58  pekangas
#* Renamed from MIDAS Sound System to MIDAS Digital Audio System
#*
#* Revision 1.9  1997/07/29 16:58:24  pekangas
#* Added sound effect test
#*
#* Revision 1.8  1997/07/28 13:43:20  pekangas
#* Fixed to work with Visual C
#*
#* Revision 1.7  1997/07/28 13:18:49  pekangas
#* Added config test, reorganized targets a bit to get only those test that
#* work in a given environment to compile
#*
#* Revision 1.6  1997/07/26 14:15:48  pekangas
#* Added profiling test
#*
#* Revision 1.5  1997/07/17 09:50:55  pekangas
#* Added testing for integer mono mixing
#* Removed an extra dsmClose() from main() (oops)
#*
#* Revision 1.4  1997/05/30 18:23:39  pekangas
#* Added mixing test
#*
#* Revision 1.3  1997/05/20 15:19:14  pekangas
#* Added WAVE reader test
#*
#* Revision 1.2  1997/04/08 19:17:24  pekangas
#* Initial revision
#*
