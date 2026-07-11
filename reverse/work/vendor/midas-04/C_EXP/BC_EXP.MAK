#*      BC_EXP.MAK
#*
#* Make script for MIDAS Sound System Borland C examples
#*
#* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
#*
#* This file is part of the MIDAS Sound System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Sound System license, LICENSE.TXT. By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*


# MIDAS Sound System directory:

MIDASDIR = c:\midas


# Compiler directories:

BCDIR = c:\bc
LIBDIR = $(BCDIR)\lib


# Compilers and options:

CL = bcc
CLOPTS = -3 -G -ml -O1 -I$(BCDIR)\include;$(MIDASDIR)\src -D__BC16__



MIDASLIB = $(MIDASDIR)\midasbc.lib
MCUTLIB = $(MIDASDIR)\mcutbc.lib

all : clopts mplay.exe msetup.exe effects.exe scrdemo.exe playmm.exe


clopts :
        echo $(CLOPTS) > clopts


mplay.exe : mplay.c $(MIDASLIB)
        $(CL) @clopts mplay.c $(MIDASLIB)

msetup.exe : msetup.c $(MIDASLIB)
        $(CL) @clopts msetup.c $(MIDASLIB)

effects.exe : effects.c $(MIDASLIB)
        $(CL) @clopts effects.c $(MIDASLIB)

scrdemo.exe : scrdemo.c $(MIDASLIB)
        $(CL) @clopts scrdemo.c $(MIDASLIB)

playmm.exe : playmm.c $(MCUTLIB)
        $(CL) @clopts playmm.c $(MCUTLIB)
