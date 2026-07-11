#*      makefile
#*
#* Make script for building MIDAS Sound System libraries
#*
#* $Id: makefile,v 1.9 1997/03/05 16:52:21 pekangas Exp $
#*
#* Copyright 1996,1997 Housemarque Inc.
#*
#* This file is part of the MIDAS Sound System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Sound System license, LICENSE.TXT. By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*

# Define NODSOUND to disable DirectSound support from Win32 targets
#NODSOUND = 1

# Allegro directory for DJGPP:
ALLEGRODIR = c:/djgpp/allegro

# DirectSound SDK directory:
DXSDKDIR = f:/dxsdk/sdk

ifdef NODSOUND
  WIN32DEFINES = NODSOUND
else
  WIN32INCPATH = $(DXSDKDIR)/inc
  WIN32LIBS = $(DXSDKDIR)/lib/dsound.lib user32.lib
endif

# DJGPP version needs allegro:
DOSGCCINCPATH = $(ALLEGRODIR)/src
DOSGCCLIBPATH = $(ALLEGRODIR)
DOSGCCLIBS = alleg

# Include build options:
BLDOPTSDIR=../bldopts
MIDASDIR=../..
include $(BLDOPTSDIR)/bldopts.mak

# Get MIDAS version information
include $(BLDOPTSDIR)/version.mak

# Install destination directory:
INSTALLDIR = $(MIDASDIR)/lib/$(_TARGET)/$(_COMP)$(BUILD)
DLLINSTALLDIR = $(MIDASDIR)/lib/$(_TARGET)/$(BUILD)

# Include install destination directory:
INCINSTDIR = $(MIDASDIR)/include

# Default target:
ifdef _DLL_TARGET
all :		dll
else
all :           lib
endif

# Get build pattern rules:
include $(BLDOPTSDIR)/bldrules.mak

# Get dependencies:
include mincs.mak
include mcdeps.mak
include madeps.mak


# Import library name:
IMPLIBNAME = midasdll.$(_LIB)

# DLL name:
DLLNAME = midas$(VERSTR).$(_DLL)

# Library name:
ifdef _DLL_TARGET
  LIBNAME = $(IMPLIBNAME)
else
  ifdef _GCC
    LIBNAME = libmidas.$(_LIB)
  else
    LIBNAME = midas.$(_LIB)
  endif
endif



# Build the object list for the library:
ifdef _WIN32_TARGET
  # Win32 library objects:
  LIBOBJS = midas.obj dsm.obj dsmmix.obj mmem.obj errors.obj  mutils.obj \
            winwave.obj postproc.obj mglobals.obj gmplayer.obj gmpcmds.obj \
            file.obj rawf_nt.obj loadmod.obj loadxm.obj loads3m.obj vu.obj \
            dsmnsnd.obj mpoll.obj midasfx.obj midasstr.obj midasdll.obj \
            madpcm.obj mulaw.obj
  ifndef NODSOUND
    LIBOBJS += dsound.obj
  endif

  # DLL objects:
  DLLOBJS = $(LIBOBJS)
endif

ifdef _DOS_TARGET
  # DOS library objects:
  LIBOBJS = midas.$(O) dsm.$(O) dsmmix.$(O) mmem.$(O) errors.$(O) \
            mutils.$(O) dma.$(O) mixsd.$(O) postproc.$(O) pas.$(O) \
            mglobals.$(O) sb.$(O) sbirq.$(O) gmplayer.$(O) gmpcmds.$(O) \
            file.$(O) rawfile.$(O) loadmod.$(O) wss.$(O) dostimer.$(O) \
            gus.$(O) loadxm.$(O) loads3m.$(O) vu.$(O) nosound.$(O) \
            mconfig.$(O) gusmix.$(O) dpmi.$(O) midasdll.$(O) midasfx.$(O) \
            mulaw.$(O) mapiconf.$(O)

  ifdef _DJGPP_TARGET
    LIBOBJS += djtext.$(O)
  else
    LIBOBJS += vgatext.$(O)
  endif
endif

ifdef _LINUX_TARGET
  # Linux library objects:
  LIBOBJS = midas.o midasdll.o midasfx.o midasstr.o dsm.o dsmmix.o mmem.o \
            errors.o mutils.o postproc.o mglobals.o gmplayer.o gmpcmds.o \
            file.o rawfile.o loadmod.o loadxm.o loads3m.o mpoll.o oss.o \
            madpcm.o mulaw.o
endif



####
module.exe : $(LIBNAME) module.c
	$(CC) -g -DDEBUG -Wall -Werror -m486 -fpack-struct -D_REENTRANT -I. -L. -Lc:/djgpp/allegro module.c -o module -lmidas -lalleg



# The statically linked library:
lib :		$(LIBNAME)

$(LIBNAME) :	$(LIBOBJS)
	$(BUILDLIBRARY)


# The dynamically linked library:
dll :		$(DLLNAME)

$(DLLNAME) : 	$(DLLOBJS)
ifdef _DLL_TARGET
	$(BUILDDLLIMPLIB)
else
	echo Target type not Win32 DLL!
endif


# Install it to the correct directory:
install :	$(INSTALLDIR)/$(LIBNAME) $(INCINSTDIR)/midasdll.h

$(INSTALLDIR)/$(LIBNAME) : $(LIBNAME)
	$(COPYFILE)

$(INCINSTDIR)/midasdll.h : midasdll.h
	$(COPYFILE)


installdll :	$(DLLINSTALLDIR)/$(DLLNAME)

$(DLLINSTALLDIR)/$(DLLNAME) : $(DLLNAME)
	$(COPYFILE)



# Cleanup:

clean distclean cvsclean :
	$(CLEANALL)


.PHONY : all lib dll install installdll clean distclean cvsclean


#* $Log: makefile,v $
#* Revision 1.9  1997/03/05 16:52:21  pekangas
#* Added module mapiconf
#*
#* Revision 1.8  1997/02/27 16:02:26  pekangas
#* Added DJGPP support and GUS software mixing SD
#*
#* Revision 1.7  1997/02/22 18:43:08  jpaana
#* Added mulaw.c to Linux and DOS (plus adpcm.c)
#*
#* Revision 1.6  1997/02/20 19:50:13  pekangas
#* Added mulaw.obj
#*
#* Revision 1.5  1997/02/11 18:13:52  pekangas
#* Added ADPCM
#*
#* Revision 1.4  1997/02/07 15:15:22  pekangas
#* Added preliminary Linux support - static library seems to compile OK
#*
#* Revision 1.3  1997/02/06 20:58:19  pekangas
#* Added DirectSound support - new files, errors, and global flags
#*
#* Revision 1.2  1997/02/05 22:55:04  pekangas
#* Small fixes to get new build system work better. Changed MIDAS version
#* to 0.6.1
#*
#* Revision 1.1  1997/02/05 17:40:42  pekangas
#* Changed to new makefile structure. Removed old makefiles, library
#* command files and related junk. Fixed some double linefeeds caused
#* by RCS-CVS transition. lang.h now defined NOTIMER for Win32.
#*
