#*      makefile
#*
#* Make script for building MIDAS Digital Audio System tools
#*
#* $Id: makefile,v 1.2 1997/07/31 10:57:00 pekangas Exp $
#*
#* Copyright 1996,1997 Housemarque Inc.
#*
#* This file is part of MIDAS Digital Audio System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Digital Audio System license, "license.txt". By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*

WIN32LIBS = user32.lib

# Include build options:
BLDOPTSDIR=../bldopts
MIDASDIR=../..
include $(BLDOPTSDIR)/bldopts.mak

ifndef _WIN32
error :
	echo Win32 only!
endif

# Install destination directory:
INSTALLDIR = ../midas

# Default target:
all :		win32dlg.c dlgtest.exe

# Get build pattern rules:
include $(BLDOPTSDIR)/bldrules.mak


dlgtest.exe :	dlgtest.obj win32dlg.obj
	$(BUILDEXE)

dlgtest.obj :	dlgtest.c

win32dlg.obj :	win32dlg.c

win32dlg.c :	dlg2c.exe
	dlg2c.exe > win32dlg.c


ifdef _VC
dlg2c.exe :	dlg2c.obj dialog.res
	$(BUILDEXE)
else
dlg2c.exe :	dlg2c.obj dialog.res
	$(LINK) $(LINKOPTS) name dlg2c.exe file dlg2c.obj
	$(RC) $(RCOPTS) dialog.res dlg2c.exe
endif

dialog.res :	temp.rc
	$(BUILDRES)

temp.rc :	dialog.rc
	-del temp.rc dialog.res
	-copy incwin.rc+dialog.rc temp.rc


dlg2c.obj :	dlg2c.c resource.h



install :	$(INSTALLDIR)/win32dlg.c $(INSTALLDIR)/win32dlg.h

$(INSTALLDIR)/win32dlg.c : win32dlg.c
	$(COPYFILE)

$(INSTALLDIR)/win32dlg.h : resource.h
	$(COPYFILE)



# Cleanup

clean distclean cvsclean :
	$(CLEANALL)
	-$(RM) win32dlg.c
	-$(RM) dialog.aps
	-$(RM) temp.rc


.PHONY : all install clean distclean cvsclean


#* $Log: makefile,v $
#* Revision 1.2  1997/07/31 10:57:00  pekangas
#* Renamed from MIDAS Sound System to MIDAS Digital Audio System
#*
#* Revision 1.1  1997/07/08 19:23:46  pekangas
#* Initial revision
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

