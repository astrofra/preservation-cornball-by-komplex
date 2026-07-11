#*      MBCLIB.MAK
#*
#* Make script for MIDAS Sound System Borland C library
#*
#* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
#*
#* This file is part of the MIDAS Sound System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Sound System license, LICENSE.TXT. By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*


# If CUTDOWN is defined (make -DCUTDOWN), the library file name will
# be "mcutbc.lib" and all Module Players will be compiled without loading
# routines. Also other routines, not necessary for small programs such
# as demos, will be removed.


# Compiler directories:

BCDIR = c:\bc
LIBDIR = $(BCDIR)\lib


# Compilers and options:

CC = bcc
CCOPTS = -c -3 -G- -ml -O1 -I$(BCDIR)\include -D__BC16__
ASM = tasm
ASMOPTS = -UT310 -ml -m9 -d__BC16__
LIB = tlib
LIBOPTS = /C


# MIDAS Sound System object files:

MIDASOBJS = midas.obj mglobals.obj errors.obj mmem.obj asmrfile.obj file.obj \
	    ems.obj mod.obj modload.obj s3m.obj s3mload.obj mtm.obj \
	    mtmload.obj timer.obj gus.obj pas.obj wss.obj sb.obj nsnd.obj \
	    dsm.obj dma.obj vu.obj mparser.obj mconfig.obj mutils.obj \
            vgatext.obj mm.obj

!ifdef CUTDOWN
ASMOPTS = $(ASMOPTS) -dNOLOADERS -dNOCMDNAMES -dNOEMS -dCUTDOWN
CCOPTS = $(CCOPTS) -DNOLOADERS -DNOCMDNAMES -DNOEMS -DCUTDOWN
LIBNAME = mcutbc.lib
!else
LIBNAME = midasbc.lib
!endif


all : ccopts asmopts $(LIBNAME)

ccopts :
	echo $(CCOPTS) > ccopts

asmopts :
	echo $(ASMOPTS) > asmopts



$(LIBNAME) : $(MIDASOBJS) midas.h midas.inc
        $(LIB) $(LIBNAME) $(LIBOPTS) @mlibobjs


vgatext.obj : vgatext.asm lang.inc vgatext.inc
        $(ASM) @asmopts vgatext.asm


mglobals.obj : mglobals.c
        $(CC) @ccopts mglobals.c

errors.obj : errors.c lang.h mtypes.h errors.h vgatext.h
        $(CC) @ccopts errors.c

midas.obj : midas.c midas.h
        $(CC) @ccopts midas.c

mparser.obj : mparser.c midas.h
        $(CC) @ccopts mparser.c

mconfig.obj : mconfig.c midas.h
        $(CC) @ccopts mconfig.c

mm.obj : mm.c midas.h
        $(CC) @ccopts mm.c

s3mload.obj : s3mload.c mtypes.h errors.h mglobals.h mmem.h sdevice.h \
	mplayer.h s3m.h ems.h vu.h lang.h file.h mutils.h
        $(CC) @ccopts s3mload.c

modload.obj : modload.c mtypes.h errors.h mglobals.h mmem.h sdevice.h \
	mplayer.h mod.h ems.h vu.h lang.h file.h mutils.h
        $(CC) @ccopts modload.c

mtmload.obj : mtmload.c mtypes.h errors.h mglobals.h mmem.h sdevice.h \
	mplayer.h mtm.h ems.h vu.h lang.h file.h mutils.h
        $(CC) @ccopts mtmload.c

mmem.obj : mmem.c lang.h mmem.h errors.h
        $(CC) @ccopts mmem.c

#rawfile.obj : rawfile.c lang.h mtypes.h errors.h mmem.h rawfile.h
#        $(CC) @ccopts rawfile.c

asmrfile.obj : asmrfile.asm lang.inc errors.inc rawfile.inc mmem.inc
        $(ASM) @asmopts asmrfile.asm

file.obj : file.c lang.h mtypes.h errors.h mmem.h file.h rawfile.h
        $(CC) @ccopts file.c

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
	touch midas.h

midas.inc : lang.inc errors.inc mglobals.inc mmem.inc sdevice.inc \
	    rawfile.inc file.inc mplayer.inc timer.inc mod.inc s3m.inc \
            mod.inc dma.inc dsm.inc ems.inc vu.inc
	touch midas.inc

file.h : rawfile.h
        touch file.h

