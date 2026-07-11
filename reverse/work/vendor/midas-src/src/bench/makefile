#*      makefile
#*
#* Make script for building MIDAS Digital Audio System benchmarks
#*
#* $Id: makefile,v 1.5.2.1 1997/08/25 19:39:05 pekangas Exp $
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
MIDASLIB = ../../lib/$(_TARGET)/$(_COMP)$(_BUILD)/midas.lib
LIBS = $(MIDASLIB)

# Get includes from the MIDAS source directory:
INCPATH = ../midas

WIN32LIBS = user32 winmm advapi32

# Include build options:
BLDOPTSDIR=../bldopts
include $(BLDOPTSDIR)/bldopts.mak

# Install destination directory:
INSTALLDIR = $(MIDASDIR)/tools/$(_TARGET)

# Default target:
ifdef _PROF
all :		empty$(_EXE) adpcm$(_EXE) mix$(_EXE) playprof$(_EXE)
else
all :		empty$(_EXE) adpcm$(_EXE) mix$(_EXE)
endif

# Get build pattern rules:
include $(BLDOPTSDIR)/bldrules.mak


# Empty polling:
empty$(_EXE) : empty.$(O) bench.$(O)
	$(BUILDEXE)

empty.$(O) : empty.c bench.h

bench.$(O) : bench.c bench.h


# ADPCM benchmark:
adpcm$(_EXE) : adpcm.$(O) bench.$(O)
	$(BUILDEXE)

adpcm.$(O) : adpcm.c bench.h


# Mixing benchmark:
mix$(_EXE) : mix.$(O) bench.$(O)
	$(BUILDEXE)

mix.$(O) : mix.c bench.h


# Module playback profiling:
playprof$(_EXE) : playprof.$(O)
	$(BUILDEXE)

playprof.$(O) : playprof.c


# Cleanup

clean distclean cvsclean :
	$(CLEANALL)


#* $Log: makefile,v $
#* Revision 1.5.2.1  1997/08/25 19:39:05  pekangas
#* Fixed for Visual C
#*
#* Revision 1.5  1997/07/31 10:56:32  pekangas
#* Renamed from MIDAS Sound System to MIDAS Digital Audio System
#*
#* Revision 1.4  1997/07/26 14:18:12  pekangas
#* Added playprof
#*
#* Revision 1.3  1997/05/30 18:21:56  pekangas
#* Added mixing routine benchmark
#*
#* Revision 1.2  1997/02/12 20:48:45  pekangas
#* Fixed Visual C support
#*
#* Revision 1.1  1997/02/12 19:07:35  pekangas
#* Initial revision
#*









