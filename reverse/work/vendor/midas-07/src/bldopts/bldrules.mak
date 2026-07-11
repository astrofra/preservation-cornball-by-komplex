#*      bldrules.mak
#*
#* Build pattern rules common for all make scripts
#*
#* $Id: bldrules.mak,v 1.5 1997/03/05 16:54:15 pekangas Exp $
#*
#* Copyright 1997 Housemarque Inc.
#*
#* This file is part of the MIDAS Sound System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Sound System license, LICENSE.TXT. By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*


# Hack for GCC:
ifdef _GCC

%.$(O) :        %.c
	$(CC) $(CCOPTS) $< -o $@

%.$(O) :        %.cc
	$(CPP) $(CPPOPTS) $< -o $@

%.$(O) :        %.cpp
	$(CPP) $(CPPOPTS) $< -o $@

ifdef _DOS_TARGET
%.$(O) :	%.asm
	$(ASM) $(ASMOPTS) $<
	o2c $(patsubst %.$(O),%.obj,$@)

%.$(O) :	%.S

%.$(O) :	%.s

else

%.$(O) :        %.S
	$(CC) $(CCOPTS) $< -o $@

endif

else

# C source files:
%.$(O) :	%.c
	$(CC) $(CCOPTS) $<


# C++ source files:
%.$(O) :	%.cc
	$(CPP) $(CPPOPTS) $<

%.$(O) :	%.cpp
	$(CPP) $(CPPOPTS) $<


# Assembler source files:
%.$(O) :	%.asm
	$(ASM) $(ASMOPTS) $<


# Resource files:
%.res :		%.rc
	$(BUILDRES)

endif


#* $Log: bldrules.mak,v $
#* Revision 1.5  1997/03/05 16:54:15  pekangas
#* Fixed compiling assembler files for DJGPP when not running in DOS
#*
#* Revision 1.4  1997/02/27 16:22:44  pekangas
#* Added DJGPP support. Changed INCDIR to INCPATH. Other minor fixes.
#*
#* Revision 1.3  1997/02/07 15:16:32  pekangas
#* Added GCC/Linux support
#*
#* Revision 1.2  1997/02/05 22:53:55  pekangas
#* Several small changes to get the new make script system work better
#*
#* Revision 1.1  1997/02/05 17:25:54  pekangas
#* Initial revision
#*







