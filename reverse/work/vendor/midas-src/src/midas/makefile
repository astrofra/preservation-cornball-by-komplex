#*      makefile
#*
#* Make script for building MIDAS Digital Audio System libraries
#*
#* $Id: makefile,v 1.43.2.7 1998/02/26 19:12:27 pekangas Exp $
#*
#* Copyright 1996,1997 Housemarque Inc.
#*
#* This file is part of MIDAS Digital Audio System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Digital Audio System license, "license.txt". By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*

# Allegro directory for DJGPP:
ALLEGRODIR = c:/djgpp/allegro

# DirectX SDK include directory:
WIN32INCPATH = f:/dx5sdk/sdk/inc

# Libraries needed by Visual C:
WIN32LIBS = user32.lib advapi32.lib

# DJGPP version needs allegro:
DOSGCCINCPATH = $(ALLEGRODIR)/src
DOSGCCLIBPATH = $(ALLEGRODIR)
DOSGCCLIBS = alleg

# Additional #defines for tracker mode:
ifdef TRACKER
  DEFINES = M_TRACKER
endif

# Include build options:
BLDOPTSDIR=../bldopts
MIDASDIR=../..
include $(BLDOPTSDIR)/bldopts.mak

# Get MIDAS version information
include $(BLDOPTSDIR)/version.mak

# Object target directory
OTARGETDIR = $(MIDASDIR)/obj/$(_TARGET)/$(_COMP)$(BUILD)/midas/

# Library target directory
LIBTARGETDIR = $(MIDASDIR)/lib/$(_TARGET)/$(_COMP)$(BUILD)/

# DLL target directory
DLLTARGETDIR = $(MIDASDIR)/lib/$(_TARGET)/$(BUILD)/

# Include target directory:
INCLUDETARGETDIR = $(MIDASDIR)/include/

# Default target:
ifdef _DLL_TARGET
all :		dll includes
else
all :           lib includes
endif

# Get build pattern rules:
include $(BLDOPTSDIR)/bldrules.mak

# Get dependencies:
include mincs.mak
include mcdeps.mak
include madeps.mak


# MIDAS Base name:
ifdef TRACKER
  MIDASNAME = mtrk
else
  MIDASNAME = midas
endif

# Import library name:
IMPLIBNAME = $(LIBTARGETDIR)$(MIDASNAME)dll.$(_LIB)

# DLL name:
DLLNAME = $(DLLTARGETDIR)$(MIDASNAME)$(VERSTR).$(_DLL)

# Library name:
ifdef _DLL_TARGET
  LIBNAME = $(IMPLIBNAME)
else
  ifdef _GCC
    LIBNAME = $(LIBTARGETDIR)lib$(MIDASNAME).$(_LIB)
  else
    LIBNAME = $(LIBTARGETDIR)$(MIDASNAME).$(_LIB)
  endif
endif


# Common object modules:
COMMONOBJS = 	mglobals.$(O) mmem.$(O) errors.$(O) mutils.$(O) \
		file.$(O) \
		gmplayer.$(O) gmpcmds.$(O) \
		loadmod.$(O) loads3m.$(O) loadxm.$(O) loadit.$(O) \
		dsm.$(O) dsmmix.$(O) dsmfilt.$(O) \
		mix_mf.$(O) mix_sf.$(O) mix_mfi.$(O) mix_sfi.$(O) \
		mix_mi.$(O) mix_si.$(O) \
		mulaw.$(O) madpcm.$(O) waveread.$(O) vu.$(O) mecho.$(O) \
		adpcmtbl.$(O) \
		midasfx.$(O) midasstr.$(O) \
		midas.$(O) midasdll.$(O)
ifdef TRACKER
  COMMONOBJS +=	midastrk.$(O)
endif

# X86-specific object modules:
X86OBJS =	adsmmix.$(O) adsmfilt.$(O) \
		adpcmasm.$(O) cputype.$(O) \
		amix_mf.$(O) amix_sf.$(O) amix_mfi.$(O) amix_sfi.$(O) \
		amix_mi.$(O) amix_si.$(O)

# Win32-specific object modules:
WIN32OBJS =	rawf_nt.$(O) mpoll.$(O) \
		win32cnf.$(O) win32dlg.$(O) \
		dsmnsnd.$(O) winwave.$(O) dsound.$(O)

# DOS-specific object modules:
DOSOBJS =	rawfile.$(O) dostimer.$(O) dpmi.$(O) \
		doscnf.$(O) \
		dsmnsnd.$(O) dma.$(O) mixsd.$(O) sb.$(O) sbirq.$(O) \
		wss.$(O) pas.$(O) gusmix.$(O)

# DOS Watcom C -specific object modules:
DOSWCOBJS =	vgatext.$(O)

# DOS GCC -specific object modules:
DOSGCCOBJS =	djtext.$(O)

# Linux-specific object modules:
LINUXOBJS =	rawfile.$(O) mpoll.$(O) \
		midasrc.$(O) \
		oss.$(O) dsmnsnd.$(O)

# Add profiling objects for profile builds:
ifdef _PROF
  COMMONOBJS += mprof.$(O)
  X86OBJS += mprofasm.$(O)
endif

ifdef _WIN32_TARGET
  LIBOBJS = $(COMMONOBJS) $(X86OBJS) $(WIN32OBJS)
  DLLOBJS := $(LIBOBJS)
endif
ifdef _DOS_TARGET
  ifdef _DJGPP_TARGET
    LIBOBJS = $(COMMONOBJS) $(X86OBJS) $(DOSOBJS) $(DOSGCCOBJS)
  else
    LIBOBJS = $(COMMONOBJS) $(X86OBJS) $(DOSOBJS) $(DOSWCOBJS)
  endif
endif
ifdef _LINUX_TARGET
  LIBOBJS = $(COMMONOBJS) $(X86OBJS) $(LINUXOBJS)
endif

# Add directories to object file names:
LIBOBJS := $(patsubst %,$(OTARGETDIR)%, $(LIBOBJS))
ifdef DLLOBJS
  DLLOBJS := $(patsubst %,$(OTARGETDIR)%, $(DLLOBJS))
endif

# The library target:
ifndef _DLL_TARGET
lib :		$(LIBNAME)

$(LIBNAME) :	$(LIBOBJS)
	$(BUILDLIBRARY)
endif


# The dynamically linked library:
ifdef _DLL_TARGET
dll :		$(DLLNAME)
else
dll :
	echo Target type not win32dll!
endif

$(DLLNAME) : 	$(DLLOBJS)
	$(BUILDDLLIMPLIB)


# Includes in their destination directory:

ifdef TRACKER
includes :	$(INCLUDETARGETDIR)midasdll.h \
                $(INCLUDETARGETDIR)midasdll.pas \
		$(INCLUDETARGETDIR)midastrk.h \
		$(INCLUDETARGETDIR)midastrk.pas
else
includes :	$(INCLUDETARGETDIR)midasdll.h \
		$(INCLUDETARGETDIR)midasdll.pas
endif

$(INCLUDETARGETDIR)midasdll.h : midasdll.h
	$(COPYFILE)

$(INCLUDETARGETDIR)midasdll.pas : midasdll.pas
	$(COPYFILE)

$(INCLUDETARGETDIR)midastrk.h : midastrk.h
	$(COPYFILE)

$(INCLUDETARGETDIR)midastrk.pas : midastrk.pas
	$(COPYFILE)



# Cleanup:

clean cvsclean distclean :
	$(CLEANALL)

cleantarget :
ifdef _SHELL_BKSLASH
	-$(RM) $(subst /,\\,$(OTARGETDIR)*.$(O))	
	-$(RM) $(subst /,\\,$(LIBNAME))
  ifdef _DLL_TARGET
	-$(RM) $(subst /,\\,$(DLLNAME))
  endif	
else
	-$(RM) $(OTARGETDIR)*.$(O)
	-$(RM) $(LIBNAME)
  ifdef _DLL_TARGET
	-$(RM) $(DLLNAME)
  endif
endif

.PHONY : all lib dll includes clean distclean cvsclean cleantarget


#* $Log: makefile,v $
#* Revision 1.43.2.7  1998/02/26 19:12:27  pekangas
#* Changed allegro directory
#*
#* Revision 1.43.2.6  1997/10/05 13:16:04  pekangas
#* Fixed to work with Cygnus Win32 make
#*
#* Revision 1.43.2.5  1997/09/06 17:17:31  pekangas
#* Added cputype.nas
#*
#* Revision 1.43.2.4  1997/08/21 22:32:49  pekangas
#* The Mixing No Sound Sound Device is now used in the Linux version too
#*
#* Revision 1.43.2.3  1997/08/21 20:27:06  pekangas
#* Added assembler optimized ADPCM decoding routines
#*
#* Revision 1.43.2.2  1997/08/21 18:02:18  pekangas
#* Added the ADPCM decoding table
#*
#* Revision 1.43.2.1  1997/08/20 12:48:04  pekangas
#* Manually merged fixes from the main trunk (oops)
#*
#* Revision 1.44  1997/08/19 17:22:01  pekangas
#* Added assembler versions of the default filtering routines
#*
#* Revision 1.43  1997/08/18 13:55:30  pekangas
#* Changed Allegro directory
#*
#* Revision 1.42  1997/08/13 17:34:48  pekangas
#* Fixed DirectX include path
#*
#* Revision 1.41  1997/07/31 17:29:17  pekangas
#* Fixed import library target directory
#*
#* Revision 1.40  1997/07/31 15:54:07  pekangas
#* Changed DOS to use mixing No Sound Sound Device
#*
#* Revision 1.39  1997/07/31 10:56:45  pekangas
#* Renamed from MIDAS Sound System to MIDAS Digital Audio System
#*
#* Revision 1.38  1997/07/29 13:17:26  pekangas
#* Fixed include installation
#* Fixed DLL object target directories
#*
#* Revision 1.37  1997/07/28 13:44:31  pekangas
#* Added "cleantarget" rule
#*
#* Revision 1.36  1997/07/28 13:21:10  pekangas
#* Removed gus.obj (not used), removed old DOS configuration modules,
#* added new dos configuration module
#*
#* Revision 1.35  1997/07/24 14:55:04  pekangas
#* Added stereo integer mixing routines
#*
#* Revision 1.34  1997/07/23 17:27:38  pekangas
#* Added profiling build
#*
#* Revision 1.33  1997/07/22 13:46:18  pekangas
#* Added support for keeping object modules in a different directory
#* from the source
#*
#* Revision 1.32  1997/07/17 09:55:46  pekangas
#* Added support for integer mono mixing
#*
#* Revision 1.31  1997/07/16 12:39:18  pekangas
#* Major cleanup
#*
#* Revision 1.30  1997/07/15 18:24:07  jpaana
#* Added midasrc.*
#*
#* Revision 1.29  1997/07/15 12:18:54  pekangas
#* Fixed to compile and link with Visual C.
#* All API functions are again exported properly
#*
#* Revision 1.28  1997/07/10 18:40:23  pekangas
#* Added echo effect support
#*
#* Revision 1.27  1997/07/08 19:16:43  pekangas
#* Added Win32 setup functions, save/restore setup from registry, and
#* fixed WinWave to ignore buffer blocks -setting to be compatible with the
#* new setup.
#*
#* Revision 1.26  1997/06/26 19:14:01  pekangas
#* Added amix_sfi
#*
#* Revision 1.25  1997/06/26 14:33:31  pekangas
#* Added amix_mfi.nas
#*
#* Revision 1.24  1997/06/11 15:37:10  pekangas
#* Added interpolating stereo mixing routines
#*
#* Revision 1.23  1997/06/11 14:45:54  jpaana
#* Added dsmfilt to dos and linux
#*
#* Revision 1.22  1997/06/11 14:38:06  pekangas
#* Added filtering files
#*
#* Revision 1.21  1997/06/10 19:51:23  jpaana
#* Added IT-loader
#*
#* Revision 1.20  1997/06/05 20:18:47  pekangas
#* Added preliminary support for interpolating mixing (mono only at the
#* moment)
#*
#* Revision 1.19  1997/06/04 15:25:00  pekangas
#* Added assembler mixing and conversion routines
#*
#* Revision 1.18  1997/05/30 18:25:46  pekangas
#* Added new mixing routines for Win32
#*
#* Revision 1.17  1997/05/26 16:40:29  pekangas
#* Added midastrk.pas to install
#*
#* Revision 1.16  1997/05/23 15:05:12  pekangas
#* Fixed linux library names
#*
#* Revision 1.15  1997/05/23 14:47:05  pekangas
#* Added Delphi interface unit to install
#*
#* Revision 1.14  1997/05/21 18:13:09  pekangas
#* Fixed DLL file names
#*
#* Revision 1.13  1997/05/20 20:36:28  pekangas
#* Added RIFF WAVE reader
#*
#* Revision 1.12  1997/05/20 10:16:28  pekangas
#* Added preliminary tracker mode support
#*
#* Revision 1.11  1997/05/03 17:58:51  pekangas
#* Removed DirectSound import library
#*
#* Revision 1.10  1997/05/03 15:10:49  pekangas
#* Added stream support for DOS, removed GUS Sound Device from non-Lite
#* build. M_HAVE_THREADS now defined in threaded environment.
#*
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
