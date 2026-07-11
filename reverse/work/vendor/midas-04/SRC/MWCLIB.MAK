#*      MWCLIB.MAK
#*
#* Make script for MIDAS Sound System Watcom C16 library
#*
#* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
#*
#* This file is part of the MIDAS Sound System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Sound System license, LICENSE.TXT. By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*


# If CUTDOWN is defined (wmake -u CUTDOWN=1), the library file name will
# be "mcutwc.lib" and all Module Players will be compiled without loading
# routines. Also other routines, not necessary for small programs such
# as demos, will be removed.


# Compilers and options:

CC = wcc
CCOPTS = -ml -w3 -2 -5 -olras -s -ze -zdp -zl -zu -zq -d__WC16__
ASM = tasm
ASMOPTS = -UT310 -ml -m9 -d__WC16__
LIB = wlib
LIBOPTS = -c -n

!ifdef CUTDOWN
ASMOPTS += -dNOLOADERS -dNOCMDNAMES -dNOEMS -dCUTDOWN
CCOPTS += -dNOLOADERS -dNOCMDNAMES -dNOEMS -dCUTDOWN
LIBNAME = mcutwc.lib
!else
LIBNAME = midaswc.lib
!endif


# MIDAS Sound System object files:

MIDASOBJS = midas.obj mglobals.obj errors.obj mmem.obj asmrfile.obj file.obj \
	    ems.obj mod.obj modload.obj s3m.obj s3mload.obj mtm.obj \
	    mtmload.obj timer.obj gus.obj pas.obj wss.obj sb.obj nsnd.obj \
	    dsm.obj dma.obj vu.obj mparser.obj mconfig.obj mutils.obj \
            vgatext.obj mm.obj

all : asmopts $(LIBNAME)

asmopts : $(__MAKEFILES__)
	echo $(ASMOPTS) > asmopts


$(LIBNAME) : $(MIDASOBJS) midas.h midas.inc
        $(LIB) $(LIBOPTS) $(LIBNAME) @mlibobjs.


vgatext.obj : vgatext.asm lang.inc vgatext.inc
        $(ASM) @asmopts vgatext.asm


mglobals.obj : mglobals.c
        $(CC) mglobals.c $(CCOPTS)

errors.obj : errors.c lang.h mtypes.h errors.h vgatext.h
        $(CC) errors.c $(CCOPTS)

midas.obj : midas.c midas.h
        $(CC) midas.c $(CCOPTS)

mparser.obj : mparser.c midas.h
        $(CC) mparser.c $(CCOPTS)

mconfig.obj : mconfig.c midas.h
        $(CC) mconfig.c $(CCOPTS)

mm.obj : mm.c midas.h
        $(CC) mm.c $(CCOPTS)

s3mload.obj : s3mload.c mtypes.h errors.h mglobals.h mmem.h sdevice.h \
	mplayer.h s3m.h ems.h vu.h lang.h file.h mutils.h
        $(CC) s3mload.c $(CCOPTS)

modload.obj : modload.c mtypes.h errors.h mglobals.h mmem.h sdevice.h \
	mplayer.h mod.h ems.h vu.h lang.h file.h mutils.h
        $(CC) modload.c $(CCOPTS)

mtmload.obj : mtmload.c mtypes.h errors.h mglobals.h mmem.h sdevice.h \
	mplayer.h mtm.h ems.h vu.h lang.h file.h mutils.h
        $(CC) mtmload.c $(CCOPTS)

mmem.obj : mmem.c lang.h mmem.h errors.h
        $(CC) mmem.c $(CCOPTS)

#rawfile.obj : rawfile.c lang.h mtypes.h errors.h mmem.h rawfile.h
#        $(CC) rawfile.c $(CCOPTS)

asmrfile.obj : asmrfile.asm lang.inc errors.inc rawfile.inc mmem.inc
        $(ASM) @asmopts asmrfile.asm

file.obj : file.c lang.h mtypes.h errors.h mmem.h file.h rawfile.h
        $(CC) file.c $(CCOPTS)

mutils.obj : mutils.asm lang.inc mutils.inc
        $(ASM) @asmopts mutils.asm


s3m.obj : s3m.asm lang.inc errors.inc mglobals.inc s3m.inc mplayer.inc \
        sdevice.inc ems.inc
        $(ASM) @asmopts s3m.asm

mod.obj : mod.asm lang.inc errors.inc mglobals.inc mod.inc mplayer.inc \
        sdevice.inc ems.inc
        $(ASM) @asmopts mod.asm

mtm.obj : mtm.asm lang.inc errors.inc mglobals.inc mtm.inc mplayer.inc \
        sdevice.inc ems.inc
        $(ASM) @asmopts mtm.asm

gus.obj : gus.asm lang.inc mglobals.inc sdevice.inc mmem.inc mglobals.inc
        $(ASM) @asmopts gus.asm

sb.obj : sb.asm lang.inc errors.inc sdevice.inc dsm.inc dma.inc
        $(ASM) @asmopts sb.asm

wss.obj : wss.asm lang.inc errors.inc sdevice.inc dsm.inc dma.inc
        $(ASM) @asmopts wss.asm

pas.obj : pas.asm pas.inc lang.inc errors.inc sdevice.inc dsm.inc dma.inc
        $(ASM) @asmopts pas.asm

nsnd.obj : nsnd.asm lang.inc sdevice.inc
        $(ASM) @asmopts nsnd.asm

dma.obj : dma.asm lang.inc errors.inc dma.inc mmem.inc mglobals.inc
        $(ASM) @asmopts dma.asm

dsm.obj : dsm.asm lang.inc errors.inc mglobals.inc dsm.inc mmem.inc ems.inc \
        sdevice.inc
        $(ASM) @asmopts dsm.asm

timer.obj : timer.asm errors.inc mglobals.inc lang.inc timer.inc ems.inc \
        sdevice.inc
        $(ASM) @asmopts timer.asm

ems.obj : ems.asm lang.inc errors.inc ems.inc mmem.inc
        $(ASM) @asmopts ems.asm

vu.obj : vu.asm lang.inc errors.inc vu.inc mmem.inc sdevice.inc
        $(ASM) @asmopts vu.asm


midas.h : lang.h mtypes.h errors.h mglobals.h mmem.h file.h sdevice.h \
	  mplayer.h s3m.h mod.h mtm.h timer.h ems.h dma.h dsm.h vu.h \
          mparser.h mconfig.h vgatext.h mutils.h mm.h
        wtouch midas.h

midas.inc : lang.inc errors.inc mglobals.inc mmem.inc sdevice.inc \
	    rawfile.inc file.inc mplayer.inc timer.inc mod.inc s3m.inc \
            mod.inc dma.inc dsm.inc ems.inc vu.inc
        wtouch midas.inc

file.h : rawfile.h
        wtouch file.h

