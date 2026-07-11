#*      tasmopts.mak
#*
#* Turbo Assembler -specific macros
#*
#* $Id: tasmopts.mak,v 1.2 1997/02/27 16:22:44 pekangas Exp $
#*
#* Copyright 1997 Housemarque Inc.
#*
#* This file is part of the MIDAS Sound System, and may only be
#* used, modified and distributed under the terms of the MIDAS
#* Sound System license, LICENSE.TXT. By continuing to use,
#* modify or distribute this file you indicate that you have
#* read the license and understand and accept it fully.
#*


# Make a final sanity check - TASM only runs in DOS and Win32:
ifndef _DOS
ifndef _WIN32
error :
	echo Invalid system for TASM!
endif
endif


# Define the programs:
ASM = tasm

# Generic compiler options:
ASMOPTS = -UT310 -p -m9 -ml -t
        # -UT310        emulate TASM 3.10 syntax
        # -p            check for code segment overrides
        # -m9           multiple passes (max 9)
        # -ml           case sensitivity in all symbols
        # -t            no messages for successful assembly


# Add debug information and debug code for debug build:
ifdef _DEBUG
  ASMOPTS += -zi -dDEBUG
        # -zi           full debug information
endif


# Add compiler-dependent macro definitions:
ifdef _WC
  ASMOPTS += -d__WC32__
endif
ifdef _VC
  ASMOPTS += -d__VC32__
endif
ifdef _GCC
  ASMOPTS += -d__WC32__
endif



#* $Log: tasmopts.mak,v $
#* Revision 1.2  1997/02/27 16:22:44  pekangas
#* Added DJGPP support. Changed INCDIR to INCPATH. Other minor fixes.
#*
#* Revision 1.1  1997/02/05 17:33:42  pekangas
#* Initial revision
#*


