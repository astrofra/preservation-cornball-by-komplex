#*      makefile
#*
#* Make script for building MIDAS Digital Audio System tools
#*
#* $Id: makefile,v 1.7.2.1 1998/02/27 22:09:14 pekangas Exp $
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
DOSGCCLIBPATH = c:/djgpp/allegro
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
all :		adpcm$(_EXE) playstr$(_EXE) ulaw$(_EXE)

# Get build pattern rules:
include $(BLDOPTSDIR)/bldrules.mak


# ADPCM converter:
adpcm$(_EXE) : adpcm.$(O)
	$(BUILDEXE)

adpcm.$(O) : adpcm.c


# Stream player:
playstr$(_EXE) : playstr.$(O)
	$(BUILDEXE)

playstr.$(O) : playstr.c


# u-law converter:
ulaw$(_EXE) : ulaw.$(O)
	$(BUILDEXE)

ulaw.$(O) : ulaw.c


# Installation:
install :	$(INSTALLDIR)/playstr$(_EXE) $(INSTALLDIR)/adpcm$(_EXE) $(INSTALLDIR)/ulaw$(_EXE)

$(INSTALLDIR)/playstr$(_EXE) : playstr$(_EXE)
	$(COPYFILE)

$(INSTALLDIR)/adpcm$(_EXE) : adpcm$(_EXE)
	$(COPYFILE)

$(INSTALLDIR)/ulaw$(_EXE) : ulaw$(_EXE)
	$(COPYFILE)

# Cleanup

clean distclean cvsclean :
	$(CLEANALL)


.PHONY : all install clean distclean cvsclean


#* $Log: makefile,v $
#* Revision 1.7.2.1  1998/02/27 22:09:14  pekangas
#* Added ulaw to install
#*
#* Revision 1.7  1997/07/31 15:54:36  pekangas
#* Fixed to work with djgpp
#*
#* Revision 1.6  1997/07/31 10:57:00  pekangas
#* Renamed from MIDAS Sound System to MIDAS Digital Audio System
#*
#* Revision 1.5  1997/02/27 16:23:26  pekangas
#* Changed INCDIR to INCPATH
#*
#* Revision 1.4  1997/02/20 19:47:38  pekangas
#* Added u-law decoder and updated for u-law
#*
#* Revision 1.3  1997/02/12 20:46:40  pekangas
#* Fixed Visual C support, added install target
#*
#* Revision 1.2  1997/02/12 16:26:50  pekangas
#* Added stream player
#*
#* Revision 1.1  1997/02/11 17:18:33  pekangas
#* Initial revision
#*

