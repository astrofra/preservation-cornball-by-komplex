#*      WC_EXP.MAK
#*
#* Make script for MIDAS Sound System Watcom C examples
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


# Compilers and options:

CL = wcl
CLOPTS = -ml -w3 -2 -5 -olras -s -ze -zdp -zu -zq -d__WC16__ \
        -i$(MIDASDIR)\src



MIDASLIB = $(MIDASDIR)\midaswc.lib
MCUTLIB = $(MIDASDIR)\mcutwc.lib

all : mplay.exe msetup.exe effects.exe scrdemo.exe playmm.exe


mplay.exe : mplay.c $(MIDASLIB)
        $(CL) mplay.c $(MIDASLIB) $(CLOPTS)

msetup.exe : msetup.c $(MIDASLIB)
        $(CL) msetup.c $(MIDASLIB) $(CLOPTS)

effects.exe : effects.c $(MIDASLIB)
        $(CL) effects.c $(MIDASLIB) $(CLOPTS)

scrdemo.exe : scrdemo.c $(MIDASLIB)
        $(CL) scrdemo.c $(MIDASLIB) $(CLOPTS)

playmm.exe : playmm.c $(MCUTLIB)
        $(CL) playmm.c $(MCUTLIB) $(CLOPTS)
